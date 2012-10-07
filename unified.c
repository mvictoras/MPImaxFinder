#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>


int n, k, p;

typedef enum {
  manual,
  scatter
} SCATTER_METHOD;

typedef enum { 
  ring_shift, ring_reduction, 
  hypercube_1, hybercube_2 
} TYPE;
SCATTER_METHOD scatter_method;
TYPE type;

int *generate_array(int num_procs, int *local_coords, char *proc_name);
void create_ring_topology(MPI_Comm *comm_ring, int *local_rank, int *num_procs, int *local_coords);
void scatter_manual(MPI_Comm *comm_ring, int local_rank, int num_procs, int *local_coords, char *proc_name);
void scatter_mpi(MPI_Comm *comm_ring, int local_rank, int num_procs, int *local_coords, char *proc_name);

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
        else {
          fprintf( stderr, "Option -%c %s in incorrect. Allowed values are: ring_shift, ring_reduction\n", optopt, optarg);
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

int main(int argc, char **argv) {
	int name_len, *array, dims[1], periods[1];
	int *ptr_gen_array, *local_array;
	
  int i, num_procs, local_rank, local_coords[1];
	//clock_t start, end;
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

  if( scatter_method == manual )
    scatter_manual(&comm_ring, local_rank, num_procs, local_coords, proc_name);
  else if( scatter_method == scatter )
    scatter_mpi(&comm_ring, local_rank, num_procs, local_coords, proc_name);

	MPI_Comm_free(&comm_ring);
	MPI_Finalize(); // Exit MPI
	//fclose(f);
	return 0;
}

void create_ring_topology(MPI_Comm *comm_ring, int *local_rank, int *num_procs, int *local_coords) {
	int dims[1];
	int periods[1];
  
  dims[0] = p * k;
  periods[0] = 1;

  // Create the topology
	MPI_Cart_create(MPI_COMM_WORLD, 1, dims, periods, 0, comm_ring);
	MPI_Comm_rank(*comm_ring, local_rank);
	MPI_Comm_size(*comm_ring, num_procs);
	MPI_Cart_coords(*comm_ring, *local_rank, 1, local_coords);
}

int find_max(int *array, int start, int end) {
	int i, max = 0;
	for(i = start; i <end; i++) {
		if( array[i] > max ) max = array[i];
	}

	return max;
}

int *generate_array(int num_procs, int *local_coords, char *proc_name) {
	unsigned int iseed = (unsigned int)time(NULL);
  int *gen_array, i;
  double start, end, dt;
  printf("(%s(%d/%d): Generating %d random numbers using %d physical processors and %d logical processors\n", proc_name, local_coords[0], num_procs, n, p, k);
	srand (iseed);
	gen_array = (int *)malloc(sizeof(int) * n);
  start = MPI_Wtime();
	for(i = 0; i < n; i++) {
		gen_array[i] = rand();
	}
   end = MPI_Wtime();
   dt = end - start;
   printf("(%s(%d/%d): %d random numbers generated in %1.8fs\n", proc_name, local_coords[0], num_procs, n, dt);
  return gen_array;
}

void scatter_manual(MPI_Comm *comm_ring, int local_rank, int num_procs, int *local_coords, char *proc_name) {
  int *gen_array, *local_array, *ptr_gen_array, elem_per_node, i;
  double start, end, dt;	
	MPI_Status status;
  
  // Find the elements per node
  // Last node has the remaining elements of the array.
  if( local_coords[0] == num_procs - 1 )
    elem_per_node = n % num_procs;
  else
    elem_per_node = n / num_procs;

	local_array = (int *)malloc(sizeof(int) * elem_per_node);// if we are in node 0
  
  // If this is node 0, create and distribute the array
  if( local_coords[0] ==  0) {
    gen_array = generate_array(num_procs, local_coords, proc_name);
    // Divide the array into the nymber of processors
    memcpy(local_array, gen_array, elem_per_node);
		ptr_gen_array = gen_array;
		ptr_gen_array += elem_per_node;
		
    // Distribute the results now
    start = MPI_Wtime();
		for( i = 1; i < num_procs; i++) {
      if( i == num_procs - 1 ) elem_per_node = n % num_procs;
			MPI_Send(ptr_gen_array, elem_per_node, MPI_INT, i, i, *comm_ring);
			ptr_gen_array += elem_per_node;
		}
    end = MPI_Wtime();
    dt = end - start;
    printf("(%s(%d/%d): It took %1.8fs to distribute the numbers\n", proc_name, local_coords[0], num_procs, dt);
    //free(gen_array)
  } else {
    start = MPI_Wtime();
		MPI_Recv(local_array, elem_per_node, MPI_INT, 0, local_rank, *comm_ring, &status);
	  end = MPI_Wtime();
    dt = end - start;
    printf("(%s(%d/%d): It took %1.8fs to receive the sub-array\n", proc_name, local_coords[0], num_procs, dt);
  }
}

void scatter_mpi(MPI_Comm *comm_ring, int local_rank, int num_procs, int *local_coords, char *proc_name) {
  int *gen_array, *local_array, elem_per_node;
  double start, end, dt;
  if( local_coords[0] == 0 ) {
    gen_array = generate_array(num_procs, local_coords, proc_name);
  }
  elem_per_node = n / num_procs;
  local_array = (int *) malloc (sizeof(int) * elem_per_node);
  start = MPI_Wtime();
  MPI_Scatter(gen_array, elem_per_node, MPI_INT, local_array, elem_per_node, MPI_INT, 0, *comm_ring);
	end = MPI_Wtime();
  dt = end - start;
  printf("(%s(%d/%d): It took %1.8fs to receive the sub-array\n", proc_name, local_coords[0], num_procs, dt);
}
