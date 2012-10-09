#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int n, k, p, dimension, *local_coords;
char s_local_coords[255];
int computerStats = 0;
// Timing
double gen_time, process_time, comm_time, total_time;

typedef enum { 
  ring, 
  hypercube,
  mesh,
  tree
} TYPE;

typedef enum {
  ring_shift,
  reduction,
  gather
} ALGO;

TYPE type;
ALGO algo;

int find_max(int *array, int start, int end);
int parse_arguments(int argc, char **argv);
int *generate_array(int num_procs, char *proc_name, int local_rank);
void create_ring_topology(MPI_Comm *comm_new, int *local_rank, int *num_procs);
void create_hypercube_topology(MPI_Comm *comm_new, int *local_rank, int *num_procs);
void create_2dmesh_topology(MPI_Comm *comm_new, int *local_rank, int *num_procs);
void create_tree_topology(MPI_Comm *comm_new, int *local_rank, int *num_procs);

int *scatter_mpi(MPI_Comm *comm_new, int local_rank, int num_procs, char *proc_name, int *elem_per_node);

void fmax_ring_shift(MPI_Comm *comm_new, int local_rank, int num_procs, char *proc_name, int *local_array, int elem_per_node);
void fmax_reduction(MPI_Comm *comm_new, int local_rank, int num_procs, char *proc_name, int *local_array, int elem_per_node);
void fmax_gather(MPI_Comm *comm_new, int local_rank, int num_procs, char *proc_name, int *local_array, int elem_per_node);


int main(int argc, char **argv) {
	double t_start, t_end;
  int *ptr_gen_array, *local_array, elem_per_node;
  int i, num_procs, local_rank, name_len;
  	
	char proc_name[MPI_MAX_PROCESSOR_NAME];
	MPI_Comm comm_new;
  gen_time = 0.0; process_time = 0.0; comm_time = 0.0; total_time = 0.0;
 
  // Parse the arguments
  if( parse_arguments(argc, argv) ) return 1;
	
  // Initialize MPI
  MPI_Init(&argc, &argv); 
	MPI_Get_processor_name(proc_name, &name_len);
  
  if( type == ring ) {
      create_ring_topology(&comm_new, &local_rank, &num_procs);
  }
  else if( type == hypercube ) {
      create_hypercube_topology(&comm_new, &local_rank, &num_procs);
  }
  else if( type == mesh ) {
    create_2dmesh_topology(&comm_new, &local_rank, &num_procs);
  }
  else if( type == tree ) {
    create_tree_topology(&comm_new, &local_rank, &num_procs);
  }

  t_start = MPI_Wtime();
  local_array = scatter_mpi(&comm_new, local_rank, num_procs, proc_name, &elem_per_node);

  
  if( algo == ring_shift ) {
    fmax_ring_shift(&comm_new, local_rank, num_procs, proc_name, local_array, elem_per_node);
  }
  else if( algo == reduction ) {
    fmax_reduction(&comm_new, local_rank, num_procs, proc_name, local_array, elem_per_node);
  }
  else if( algo == gather ) {
    fmax_gather(&comm_new, local_rank, num_procs, proc_name, local_array, elem_per_node);
  }
  
  t_end = MPI_Wtime();
  total_time = t_end - t_start;

  if( computerStats ) {
    printf("g\t%s\t%d\t%d\t%d\t%f\n", s_local_coords, num_procs, k, p, gen_time);
    printf("p\t%s\t%d\t%d\t%d\t%f\n", s_local_coords, num_procs, k, p, process_time);
    printf("c\t%s\t%d\t%d\t%d\t%f\n", s_local_coords, num_procs, k, p, comm_time);
    printf("t\t%s\t%d\t%d\t%d\t%f\n", s_local_coords, num_procs, k, p, total_time);
  }
	MPI_Comm_free(&comm_new);
	MPI_Finalize(); // Exit MPI
	//fclose(f);
	return 0;
}

int parse_arguments(int argc, char **argv) {
	int c;

	while( (c = getopt (argc, argv, "n:k:p:t:a:c")) != -1 ) {
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
      case 't':
        if( strcmp(optarg, "ring" ) == 0 ) type = ring;
        else if( strcmp(optarg, "hypercube" ) == 0 ) type = hypercube;
        else if( strcmp(optarg, "2dmesh" ) == 0 ) type = mesh;
        else if( strcmp(optarg, "tree" ) == 0 ) type = tree;
        else {
          fprintf( stderr, "Option -%c %s in incorrect. Allowed values are: ring, hypercube, 2dmesh, tree\n", optopt, optarg);
          return 1;
        }
        break;
      case 'a':
        if( strcmp(optarg, "ring_shift" ) == 0 ) algo = ring_shift;
        else if( strcmp(optarg, "reduction" ) == 0 ) algo = reduction;
        else if( strcmp(optarg, "gather" ) == 0 ) algo = gather;
        else {
          fprintf( stderr, "Option -%c %s in incorrect. Allowed values are: ring_shift, reduction\n", optopt, optarg);
          return 1;
        }
        break;
      case 'c':
        computerStats = 1;
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

void create_ring_topology(MPI_Comm *comm_new, int *local_rank, int *num_procs) {
	int dims[1], periods[1];
 
  MPI_Comm_size(MPI_COMM_WORLD, num_procs);

  dimension = 1;
  dims[0] = *num_procs;
  periods[0] = 1;
  local_coords = (int *) malloc(sizeof(int) * dimension);
  // Create the topology
	MPI_Cart_create(MPI_COMM_WORLD, dimension, dims, periods, 0, comm_new);
	MPI_Comm_rank(*comm_new, local_rank);
	MPI_Comm_size(*comm_new, num_procs);
	MPI_Cart_coords(*comm_new, *local_rank, dimension, local_coords);
  sprintf(s_local_coords, "[%d]", local_coords[0]);
}

void create_hypercube_topology(MPI_Comm *comm_new, int *local_rank, int *num_procs) {
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

  MPI_Cart_create(MPI_COMM_WORLD, dimension, dims, periods, 0, comm_new);
	MPI_Comm_size(*comm_new, num_procs);
  MPI_Cart_coords(*comm_new, *local_rank, dimension, local_coords);
  s_local_coords[0] = '\0';
  for( i = 0; i < dimension; i++ ) {
    char number[10];
    sprintf(number, "[%d]", local_coords[i]);
    strcat(s_local_coords, number);
  }
}

void create_2dmesh_topology(MPI_Comm *comm_new, int *local_rank, int *num_procs) {
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

  MPI_Cart_create(MPI_COMM_WORLD, dimension, dims, periods, 0, comm_new);
	MPI_Comm_size(*comm_new, num_procs);
  MPI_Cart_coords(*comm_new, *local_rank, dimension, local_coords);
  sprintf(s_local_coords, "[%d][%d]", local_coords[0], local_coords[1]);
}

void create_tree_topology(MPI_Comm *comm_new, int *local_rank, int *num_procs) {
  MPI_Comm_size(MPI_COMM_WORLD, num_procs);
	MPI_Comm_rank(MPI_COMM_WORLD, local_rank);
  
  local_coords = (int *) malloc(sizeof(int) * 4);
 
  int index[] = { 2, 5, 7, 8, 9, 10, 12, 14 };
  int edges[] = { 1, 2, 3, 4, 5, 6, 7, 1, 1, 1, 2, 2};
  MPI_Graph_create(MPI_COMM_WORLD, 8, index, edges, 1, comm_new);
	MPI_Comm_size(*comm_new, num_procs);
  //MPI_Cart_coords(*comm_new, *local_rank, dimension, local_coords);
}

int find_max(int *array, int start, int end) {
	int i, max = 0;
	for(i = start; i <end; i++) {
		if( array[i] > max ) max = array[i];
	}

	return max;
}

int *generate_array(int num_procs, char *proc_name, int local_rank) {
	unsigned int iseed = (unsigned int)time(NULL);
  int *gen_array, i;
  double start, end, dt;
  
  if( !computerStats )
    printf("(%s(%d/%d)%s: Generating %d random numbers using %d physical processors and %d logical processors\n", proc_name, local_rank, num_procs, s_local_coords, n, p, k);
	
  srand (iseed);
	gen_array = (int *)malloc(sizeof(int) * n);
  
  start = MPI_Wtime();
	for(i = 0; i < n; i++) {
		//gen_array[i] = rand();
	  gen_array[i] = 1;
  }
  gen_array[2345345] = 10;
  end = MPI_Wtime();
  dt = end - start;
  gen_time = dt;

  if( !computerStats )
    printf("(%s(%d/%d)%s: %d random numbers generated in %1.8fs\n", proc_name, local_rank, num_procs, s_local_coords, n, dt);
  
  return gen_array;
}

int *scatter_mpi(MPI_Comm *comm_new, int local_rank, int num_procs, 
                 char *proc_name, int *elem_per_node) {
  int *gen_array, *local_array;
  double start, end, dt;
  if( local_rank == 0 ) {
    gen_array = generate_array(num_procs, proc_name, local_rank);
  } else gen_array = (int *) malloc(sizeof(int) * n);
  *elem_per_node = n / num_procs;
  local_array = (int *) malloc (sizeof(int) * *elem_per_node);
  
  start = MPI_Wtime();
  MPI_Scatter(gen_array, *elem_per_node, MPI_INT, local_array, *elem_per_node, MPI_INT, 0, *comm_new);
	end = MPI_Wtime();
  dt = end - start;
  comm_time += dt;

  if( !computerStats )
    printf("(%s(%d/%d)%s: It took %1.8fs to receive the sub-array\n", proc_name, local_rank, num_procs, s_local_coords, dt);
  free(gen_array);

  return local_array;
}

void fmax_ring_shift(MPI_Comm *comm_new, int local_rank, int num_procs, 
                     char *proc_name, int *local_array, int elem_per_node) {
  double start, end, dt;
  int i, local_max;
  MPI_Status status;

  start = MPI_Wtime();
	local_max = find_max(local_array, 0, elem_per_node);
	end = MPI_Wtime();
	dt = end - start;
  process_time += dt;

  if( !computerStats )
    printf("(%s(%d/%d)%s: Local max is %d (%1.8fs)\n", proc_name, local_rank, num_procs, s_local_coords, local_max, dt);

	int leftrank, rightrank;
	MPI_Cart_shift(*comm_new, 0, -1, &rightrank, &leftrank);
	
	int sent_max[1];
	sent_max[0] = local_max;
	
  for (i = 0; i < num_procs; i++) {
		if( !computerStats )
      printf("(%s(%d/%d)%s: Sending at %d from %d to %d:%d\n",  proc_name, local_rank, num_procs,
                                                                s_local_coords, local_rank, leftrank, 
                                                                rightrank, sent_max[0]);

		start = MPI_Wtime();
    MPI_Sendrecv_replace(sent_max, 1, MPI_INT, rightrank, 3, leftrank, 3, *comm_new, &status);
		end = MPI_Wtime();
    dt = end - start;
    comm_time += dt;

		if( !computerStats )
      printf("(%s(%d/%d)%s: Sending at %d from %d to %d:%d (%1.8fs)\n", proc_name, local_rank, num_procs,
                                                                        s_local_coords, local_rank, leftrank, 
                                                                        rightrank, sent_max[0], dt);

    if( sent_max[0] > local_max) {
			local_max = sent_max[0];
		}
	}
	if( local_rank == 0) {
		if( !computerStats )
      printf("Max is:%d\n", local_max);
		end = MPI_Wtime();
    dt = end - start;
    if( !computerStats )
      printf("(%s(%d/%d)%s: Time it took to find the max:%1.8fs\n", proc_name, local_rank, num_procs,
                                                                    s_local_coords, dt);
	}
}

void fmax_reduction(MPI_Comm *comm_new, int local_rank, int num_procs, char *proc_name, int *local_array, int elem_per_node) {
  double start, end, dt;
  int i, local_max, found_max;
  MPI_Status status;

  start = MPI_Wtime();
  local_max = find_max(local_array, 0, elem_per_node);
  end = MPI_Wtime();
  dt = end - start;
  process_time += dt;

  if( !computerStats )
    printf("(%s(%d/%d)%s: Local max is %d (%1.8fs)\n", proc_name, local_rank, num_procs, s_local_coords, local_max, dt);

  start = MPI_Wtime();
  MPI_Reduce(&local_max, &found_max, 1, MPI_INT, MPI_MAX, 0, *comm_new);
  end = MPI_Wtime();
  dt = end - start;
  process_time += dt;

  if( local_rank == 0 && !computerStats ) 
    printf("(%s(%d/%d)%s: Max is %d (%1.8fs)\n",  proc_name, local_rank, num_procs, s_local_coords, found_max, dt);
}

void fmax_gather(MPI_Comm *comm_new, int local_rank, int num_procs, char *proc_name, int *local_array, int elem_per_node) {
  double start, end, dt;
  int i, local_max[1], found_max[1];
  MPI_Status status;
  int *recv_array = (int *) malloc(sizeof(int) * num_procs);;

  start = MPI_Wtime();
  local_max[0] = find_max(local_array, 0, elem_per_node);
  end = MPI_Wtime();
  dt = end - start;
  process_time += dt;

  if( !computerStats )
    printf("(%s(%d/%d)%s: Local max is %d (%1.8fs)\n", proc_name, local_rank, num_procs, s_local_coords, local_max[0], dt);

  start = MPI_Wtime();
  //MPI_Reduce(&local_max, &found_max, 1, MPI_INT, MPI_MAX, 0, *comm_new);
  MPI_Gather(local_max, 1, MPI_INT, recv_array, 1, MPI_INT, 0, *comm_new);
  end = MPI_Wtime();
  dt = end - start;
  comm_time += dt;

  if( local_rank == 0 ) {
    // Find the max
    start = MPI_Wtime();
    found_max[0] = find_max(recv_array, 0, num_procs);
    end = MPI_Wtime();
    dt = end - start;
    process_time += dt;

    if( !computerStats )
      printf("(%s(%d/%d)%s: Max is %d (%1.8fs)\n",  proc_name, local_rank, num_procs, s_local_coords, found_max[0], dt);
  }
  free(recv_array);
}

