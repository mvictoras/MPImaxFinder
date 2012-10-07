#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>


int n, k, p;

int parse_arguments(int argc, char **argv) {
	int c;

	while( (c = getopt (argc, argv, "n:k:p:")) != -1 ) {
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
	int i, myrank, numprocs, name_len, *array, dims[1], periods[1];
	int *to_array, *local_array, * recv_array, nodes;
	int local_rank, local_coords[1];
	//clock_t start, end;
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	MPI_Comm comm_ring;
	MPI_Status status;
  double start, end, dt, g_start;	
	// Parse the arguments
  if( parse_arguments(argc, argv) ) return 1;
	
  // Initialize MPI
  MPI_Init(&argc, &argv); 
	MPI_Get_processor_name(processor_name, &name_len);

	// Cartesian topology
  nodes = p * k;
	dims[0] = nodes;
	// Wrparound connections
	periods[0] = 1;

  // Create the topology
	MPI_Cart_create(MPI_COMM_WORLD, 1, dims, periods, 0, &comm_ring);
	MPI_Comm_rank(comm_ring, &myrank);
	MPI_Comm_size(comm_ring, &numprocs);
	MPI_Cart_coords(comm_ring, myrank, 1, local_coords);

  int elem_per_proc;
  elem_per_proc = n / numprocs;

  local_array = (int *)malloc(sizeof(int) * elem_per_proc);
  recv_array = (int *)malloc(sizeof(int) * elem_per_proc);
 
  if( local_coords[0] == 0) {
    g_start = MPI_Wtime();
		unsigned int iseed = (unsigned int)time(NULL);
    printf("(%s(%d/%d): Generating %d random numbers using %d physical processors and %d logical processors\n", processor_name, local_coords[0], numprocs, n, p, k);
		srand (iseed);
		array = (int *)malloc(sizeof(int) * n);
    start = MPI_Wtime();
		for(i = 0; i < n; i++) {
			//array[i] = rand();
      array[i] = 1;
		}
    array[6] = 1880118973;
    end = MPI_Wtime();
    dt = end - start;
    printf("(%s(%d/%d): %d random numbers generated in %1.8fs\n", processor_name, local_coords[0], numprocs, n, dt);
  }   
  
  start = MPI_Wtime();
  MPI_Scatter(array, elem_per_proc, MPI_INT, local_array, elem_per_proc, MPI_INT, 0, comm_ring);
	end = MPI_Wtime();
  dt = end - start;
  printf("(%s(%d/%d): It took %1.8fs to receive the sub-array\n", processor_name, local_coords[0], numprocs, dt);

  start = MPI_Wtime();
  int local_max = find_max(local_array, 0, elem_per_proc);
  int found_max;
  MPI_Reduce(&local_max, &found_max, 1, MPI_INT, MPI_MAX, 0, comm_ring);
  //MPI_Gather(local_array, elem_per_proc, MPI_INT, recv_array, elem_per_proc, MPI_INT, 0, comm_ring);
  end = MPI_Wtime();
  dt = end - start;

  if( local_coords[0] == 0 ) 
    printf("(%s(%d/%d): Max is %d (%1.8fs)\n",  processor_name, local_coords[0], numprocs, 
                                                found_max, dt);

	MPI_Comm_free(&comm_ring);
	MPI_Finalize(); // Exit MPI

  free(local_array);
  free(recv_array);
	return 0;
}

int find_max(int *array, int start, int end) {
	int i, max = 0;
	for(i = start; i <end; i++) {
		if( array[i] > max ) max = array[i];
	}

	return max;
}
