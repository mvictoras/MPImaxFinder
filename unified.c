#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int n, k, p, dimension, *local_coords;
char s_coords[255];

typedef enum {
  manual,
  scatter
} SCATTER_METHOD;

typedef enum { 
  ring_shift, ring_reduction, 
  hypercube_reduction, hybercube_2,
  mesh_reduction,
  tree_reduction
} TYPE;
SCATTER_METHOD scatter_method;
TYPE type;

int find_max(int *array, int start, int end);
int parse_arguments(int argc, char **argv);
int *generate_array(int num_procs, int *local_coords, char *proc_name, int local_rank);
void create_ring_topology(MPI_Comm *comm_ring, int *local_rank, int *num_procs, int *local_coords);
void create_hypercube_topology(MPI_Comm *comm_ring, int *local_rank, int *num_procs, int *local_coords);
void create_2dmesh_topology(MPI_Comm *comm_ring, int *local_rank, int *num_procs, int *local_coords);
void create_tree_topology(MPI_Comm *comm_ring, int *local_rank, int *num_procs, int *local_coords);

int *scatter_manual(MPI_Comm *comm_ring, int local_rank, int num_procs, int *local_coords, char *proc_name, int *elem_per_node);
int *scatter_mpi(MPI_Comm *comm_ring, int local_rank, int num_procs, int *local_coords, char *proc_name, int *elem_per_node);
void fmax_ring_shift(MPI_Comm *comm_ring, int local_rank, int num_procs, int *local_coords, char *proc_name, int *local_array, int elem_per_node);
void fmax_ring_reduction(MPI_Comm *comm_ring, int local_rank, int num_procs, int *local_coords, char *proc_name, int *local_array, int elem_per_node);


int main(int argc, char **argv) {
	int *ptr_gen_array, *local_array, elem_per_node;
  int i, num_procs, local_rank, *local_coords, name_len;
	
	char proc_name[MPI_MAX_PROCESSOR_NAME];
	MPI_Comm comm_ring;
  
  // Parse the arguments
  if( parse_arguments(argc, argv) ) return 1;
	
  // Initialize MPI
  MPI_Init(&argc, &argv); 
	MPI_Get_processor_name(proc_name, &name_len);

  if( type == ring_shift || type == ring_reduction ) {
      create_ring_topology(&comm_ring, &local_rank, &num_procs, local_coords);
  }
  else if( type == hypercube_reduction ) {
      create_hypercube_topology(&comm_ring, &local_rank, &num_procs, local_coords);
  }
  else if( type == mesh_reduction ) {
    create_2dmesh_topology(&comm_ring, &local_rank, &num_procs, local_coords);
  }
  else if( type == tree_reduction ) {
    create_tree_topology(&comm_ring, &local_rank, &num_procs, local_coords);
  }

  if( scatter_method == manual )
    local_array = scatter_manual(&comm_ring, local_rank, num_procs, local_coords, proc_name, &elem_per_node);
  else if( scatter_method == scatter )
    local_array = scatter_mpi(&comm_ring, local_rank, num_procs, local_coords, proc_name, &elem_per_node);

  
  if( type == ring_shift ) {
    fmax_ring_shift(&comm_ring, local_rank, num_procs, local_coords, proc_name, local_array, elem_per_node);
  }
  else if( type == ring_reduction ) {
    fmax_ring_reduction(&comm_ring, local_rank, num_procs, local_coords, proc_name, local_array, elem_per_node);
  }
 
	MPI_Comm_free(&comm_ring);
	MPI_Finalize(); // Exit MPI
	//fclose(f);
	return 0;
}

int parse_arguments(int argc, char **argv) {
	int c;

	while( (c = getopt (argc, argv, "n:k:p:s:t:")) != -1 ) {
		switch(c) {
			case 'n':
				n = atoi(optarg);
				break;
			case 'k':
				k = atoi(optarg);
				break;
			case 'p':
				p = atoi(optarg);
				break;
      case 's':
        if( strcmp(optarg, "manual" ) == 0 ) scatter_method = manual;
        else if( strcmp(optarg, "scatter" ) == 0 ) scatter_method = scatter;
        else {
          fprintf( stderr, "Option -%c %s in incorrect. Allowed values are: manual, scatter\n", optopt, optarg);
          return 1;
        }
        break;
      case 't':
        if( strcmp(optarg, "ring_shift" ) == 0 ) type = ring_shift;
        else if( strcmp(optarg, "ring_reduction" ) == 0 ) type = ring_reduction;
        else if( strcmp(optarg, "hypercube_reduction" ) == 0 ) type = hypercube_reduction;
        else if( strcmp(optarg, "2dmesh_reduction" ) == 0 ) type = mesh_reduction;
        else if( strcmp(optarg, "tree_reduction" ) == 0 ) type = tree_reduction;
        else {
          fprintf( stderr, "Option -%c %s in incorrect. Allowed values are: ring_shift, ring_reduction, hypercube_reduction\n", optopt, optarg);
          return 1;
        }
break;
			case '?':
				if( optopt == 'n' || optopt == 'k' || optopt == 'p' )
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
				return 1;
			default:
				fprintf(stderr, "Usage: %s -n <number of numbers> -k <number of logical processors> -p <number of physical processors>\n", argv[0]);
				fprintf(stderr, "\tExample: %s -n 1000 -k 10 -p 4\n", argv[0]);
				return 1;
		}
	}

		return 0;
}

void create_ring_topology(MPI_Comm *comm_ring, int *local_rank, int *num_procs, int *local_coords) {
	int dims[1], periods[1];
 
  dimension = 1;
  dims[0] = p * k;
  periods[0] = 1;
  local_coords = (int *) malloc(sizeof(int) * dimension);
  // Create the topology
	MPI_Cart_create(MPI_COMM_WORLD, dimension, dims, periods, 0, comm_ring);
	MPI_Comm_rank(*comm_ring, local_rank);
	MPI_Comm_size(*comm_ring, num_procs);
	MPI_Cart_coords(*comm_ring, *local_rank, dimension, local_coords);
}

void create_hypercube_topology(MPI_Comm *comm_ring, int *local_rank, int *num_procs, int *local_coords) {
	int *dims, i, *periods;
  
  MPI_Comm_size(MPI_COMM_WORLD, num_procs);
	MPI_Comm_rank(MPI_COMM_WORLD, local_rank);
  
  dimension = (int) (log(*num_procs) / log(2));
  local_coords = (int *) malloc(sizeof(int) * dimension);
  dims = (int *) malloc(sizeof(int) * dimension);
  periods = (int *) malloc(sizeof(int) * dimension);
  for( i = 0; i < dimension; i++ ) {
    dims[i] = 2;
    periods[i] = 1;
  }

  MPI_Cart_create(MPI_COMM_WORLD, dimension, dims, periods, 0, comm_ring);
	MPI_Comm_size(*comm_ring, num_procs);
  MPI_Cart_coords(*comm_ring, *local_rank, dimension, local_coords);
}

void create_2dmesh_topology(MPI_Comm *comm_ring, int *local_rank, int *num_procs, int *local_coords) {
  int *dims, i, *periods, nodes_per_dim;
  
  MPI_Comm_size(MPI_COMM_WORLD, num_procs);
	MPI_Comm_rank(MPI_COMM_WORLD, local_rank);
 
  dimension = 2;
  nodes_per_dim = (int) sqrt( (double) *num_procs );
  local_coords = (int *) malloc(sizeof(int) * dimension);
  dims = (int *) malloc(sizeof(int) * dimension);
  periods = (int *) malloc(sizeof(int) * dimension);
  for( i = 0; i < dimension; i++ ) {
    dims[i] = nodes_per_dim;
    periods[i] = 1;
  }

  MPI_Cart_create(MPI_COMM_WORLD, dimension, dims, periods, 0, comm_ring);
	MPI_Comm_size(*comm_ring, num_procs);
  MPI_Cart_coords(*comm_ring, *local_rank, dimension, local_coords);
}

void create_tree_topology(MPI_Comm *comm_ring, int *local_rank, int *num_procs, int *local_coords) {
  int *dims, i, *periods;
  
  MPI_Comm_size(MPI_COMM_WORLD, num_procs);
	MPI_Comm_rank(MPI_COMM_WORLD, local_rank);
  
  dimension = (int) (log(*num_procs) / log(2));
  local_coords = (int *) malloc(sizeof(int) * dimension);
  dims = (int *) malloc(sizeof(int) * dimension);
  periods = (int *) malloc(sizeof(int) * dimension);
  for( i = 0; i < dimension; i++ ) {
    dims[i] = 2;
    periods[i] = 0;
  }

  MPI_Cart_create(MPI_COMM_WORLD, dimension, dims, periods, 0, comm_ring);
	MPI_Comm_size(*comm_ring, num_procs);
  MPI_Cart_coords(*comm_ring, *local_rank, dimension, local_coords);
}

int find_max(int *array, int start, int end) {
	int i, max = 0;
	for(i = start; i <end; i++) {
		if( array[i] > max ) max = array[i];
	}

	return max;
}

int *generate_array(int num_procs, int *local_coords, char *proc_name, int local_rank) {
	unsigned int iseed = (unsigned int)time(NULL);
  int *gen_array, i;
  double start, end, dt;
  printf("(%s(%d/%d): Generating %d random numbers using %d physical processors and %d logical processors\n", proc_name, local_coords[local_rank], num_procs, n, p, k);
	srand (iseed);
	gen_array = (int *)malloc(sizeof(int) * n);
  start = MPI_Wtime();
	for(i = 0; i < n; i++) {
		gen_array[i] = rand();
	}
   end = MPI_Wtime();
   dt = end - start;
   printf("(%s(%d/%d): %d random numbers generated in %1.8fs\n", proc_name, local_coords[local_rank], num_procs, n, dt);
  return gen_array;
}

int *scatter_manual(MPI_Comm *comm_ring, int local_rank, int num_procs, 
                    int *local_coords, char *proc_name, int *elem_per_node) {
  int *gen_array, *local_array, *ptr_gen_array, i;
  double start, end, dt;	
	MPI_Status status;
  
  // Find the elements per node
  // Last node has the remaining elements of the array.
  if( local_coords[local_rank] == num_procs - 1 )
    *elem_per_node = n % num_procs;
  else
    *elem_per_node = n / num_procs;

	local_array = (int *)malloc(sizeof(int) * *elem_per_node);// if we are in node 0
  
  // If this is node 0, create and distribute the array
  if( local_coords[local_rank] ==  0) {
    gen_array = generate_array(num_procs, local_coords, proc_name, local_rank);
    // Divide the array into the nymber of processors
    memcpy(local_array, gen_array, *elem_per_node);
		ptr_gen_array = gen_array;
		ptr_gen_array += *elem_per_node;
		
    // Distribute the results now
    start = MPI_Wtime();
		for( i = 1; i < num_procs; i++) {
      if( i == num_procs - 1 ) *elem_per_node = n % num_procs;
			MPI_Send(ptr_gen_array, *elem_per_node, MPI_INT, i, i, *comm_ring);
			ptr_gen_array += *elem_per_node;
		}
    end = MPI_Wtime();
    dt = end - start;
    printf("(%s(%d/%d): It took %1.8fs to distribute the numbers\n", proc_name, local_coords[local_rank], num_procs, dt);
    free(gen_array);
  } else {
    start = MPI_Wtime();
		MPI_Recv(local_array, *elem_per_node, MPI_INT, 0, local_rank, *comm_ring, &status);
	  end = MPI_Wtime();
    dt = end - start;
    printf("(%s(%d/%d): It took %1.8fs to receive the sub-array\n", proc_name, local_coords[local_rank], num_procs, dt);
  }

  return local_array;
}

int *scatter_mpi(MPI_Comm *comm_ring, int local_rank, int num_procs, 
                int *local_coords, char *proc_name, int *elem_per_node) {
  int *gen_array, *local_array;
  double start, end, dt;
  if( local_coords[local_rank] == 0 ) {
    gen_array = generate_array(num_procs, local_coords, proc_name, local_rank);
  } else gen_array = (int *) malloc(sizeof(int) * n);
  *elem_per_node = n / num_procs;
  local_array = (int *) malloc (sizeof(int) * *elem_per_node);
  start = MPI_Wtime();
  MPI_Scatter(gen_array, *elem_per_node, MPI_INT, local_array, *elem_per_node, MPI_INT, 0, *comm_ring);
	end = MPI_Wtime();
  dt = end - start;
  printf("(%s(%d/%d): It took %1.8fs to receive the sub-array\n", proc_name, local_coords[local_rank], num_procs, dt);
  free(gen_array);

  return local_array;
}

void fmax_ring_shift(MPI_Comm *comm_ring, int local_rank, int num_procs, 
                int *local_coords, char *proc_name, int *local_array,
                int elem_per_node) {
  
  double start, end, dt;
  int i, local_max;
  MPI_Status status;

  start = MPI_Wtime();
	local_max = find_max(local_array, 0, elem_per_node);
	end = MPI_Wtime();
	dt = end - start;
  printf("(%s(%d/%d): Local max is %d (%1.8fs)\n", proc_name, local_coords[local_rank], num_procs, local_max, dt);

	int leftrank, rightrank;
	MPI_Cart_shift(*comm_ring, 0, -1, &rightrank, &leftrank);
	
	int sent_max[1];
	sent_max[0] = local_max;
	
  for (i = 0; i < num_procs; i++) {
		printf("(%s(%d/%d): Sending at %d from %d to %d:%d\n",  proc_name, local_coords[local_rank], 
                                                            num_procs, local_rank, leftrank, 
                                                            rightrank, sent_max[0]);

		start = MPI_Wtime();
    MPI_Sendrecv_replace(sent_max, 1, MPI_INT, rightrank, 3, leftrank, 3, *comm_ring, &status);
		end = MPI_Wtime();
    dt = end - start;
		printf("(%s(%d/%d): Sending at %d from %d to %d:%d (%1.8fs)\n",  proc_name, local_coords[local_rank], 
                                                            num_procs, local_rank, leftrank, 
                                                            rightrank, sent_max[0], dt);

    if( sent_max[0] > local_max) {
			local_max = sent_max[0];
		}
	}
	if( local_rank == 0) {
		printf("Max is:%d\n", local_max);
		end = MPI_Wtime();
    dt = end - start;
    printf("(%s(%d/%d): Time it took to find the max:%1.8fs\n", proc_name, local_coords[local_rank], 
                                                            num_procs, dt);
	}
}

void fmax_ring_reduction(MPI_Comm *comm_ring, int local_rank, int num_procs, int *local_coords, char *proc_name, int *local_array, int elem_per_node) {
  double start, end, dt;
  int i, local_max;
  MPI_Status status;

  start = MPI_Wtime();
  local_max = find_max(local_array, 0, elem_per_node);
  end = MPI_Wtime();
  dt = end - start;
  printf("(%s(%d/%d): Local max is %d (%1.8fs)\n", proc_name, local_coords[local_rank], num_procs, local_max, dt);
int found_max;

  start = MPI_Wtime();
  MPI_Reduce(&local_max, &found_max, 1, MPI_INT, MPI_MAX, 0, *comm_ring);
  //MPI_Gather(local_array, elem_per_proc, MPI_INT, recv_array, elem_per_proc, MPI_INT, 0, comm_ring);
  end = MPI_Wtime();
  dt = end - start;

  if( local_coords[local_rank] == 0 ) 
    printf("(%s(%d/%d): Max is %d (%1.8fs)\n",  proc_name, local_coords[local_rank], num_procs, 
                                                found_max, dt);
}

/*
void print_message(char *message, int num_procs, int *local_coords, char *proc_name) {
  int i;
  char buf[255];
  sscanf(buf, "(%s-", proc_name);
  for( i = 0; i < dimension; i++ ) {
    sscanf(buf, "%s[%d]", buf, local_coords[i]);,
  }
  printf("%s/%d: %s\n", buf, num_procs, message);
}*/
