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
	int *to_array, *local_array, nodes;
	int local_rank, local_coords[1];
	//clock_t start, end;
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	MPI_Comm comm_ring;
	MPI_Status status;
  double start, end, dt;	
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
	
  //printf("This is processor %s (%d-%d) out of %d\n", processor_name, myrank, local_coords[0], numprocs);

  int elem_per_proc;
  if( local_coords[0] == numprocs - 1 )
    elem_per_proc = n % numprocs;
  else
    elem_per_proc = n / numprocs;

	local_array = (int *)malloc(sizeof(int) * elem_per_proc);// if we are in node 0
  if( local_coords[0] == 0) {
		unsigned int iseed = (unsigned int)time(NULL);
    printf("(%s(%d/%d): Generating %d random numbers using %d physical processors and %d logical processors\n", processor_name, local_coords[0], numprocs, n, p, k);
		srand (iseed);
		array = (int *)malloc(sizeof(int) * n);
    start = MPI_Wtime();
		for(i = 0; i < n; i++) {
			array[i] = rand();
		}
    end = MPI_Wtime();
    dt = end - start;
    printf("(%s(%d/%d): %d random numbers generated in %1.8fs\n", processor_name, local_coords[0], numprocs, n, dt);

    // Divide the array into the nymber of processors
    memcpy(local_array, array, elem_per_proc);
		to_array = array;
		to_array += elem_per_proc;
		
    // Distribute the results now
    start = MPI_Wtime();
		for( i = 1; i < nodes; i++) {
      if( i == nodes - 1 ) elem_per_proc = n % numprocs;
			MPI_Send(to_array, elem_per_proc, MPI_INT, i, i, comm_ring);
			to_array += elem_per_proc;
		}
    end = MPI_Wtime();
    dt = end - start;
    printf("(%s(%d/%d): It took %1.8fs to distribute the numbers\n", processor_name, local_coords[0], numprocs, dt);
  } else {
    start = MPI_Wtime();
		MPI_Recv(local_array, elem_per_proc, MPI_INT, 0, myrank, comm_ring, &status);
	  end = MPI_Wtime();
    dt = end - start;
    printf("(%s(%d/%d): It took %1.8fs to receive the sub-array\n", processor_name, local_coords[0], numprocs, dt);
}
  
  /*
	int elem_per_proc = n / numprocs;
	local_array = (int *)malloc(sizeof(int) * elem_per_proc);
	// Read the file
	if ( local_coords[0] == 0 ) {
		printf("Inside if\n");
		start = clock();
		unsigned int iseed = (unsigned int)time(NULL);
		srand (iseed);
		array = (int *)malloc(sizeof(int) * n);
		for(i = 0; i < n; i++) {
			array[i] = rand();
		}

		memcpy(local_array, array, elem_per_proc);
		to_array = array;
		to_array += elem_per_proc;
		// Distribute the results now
		printf("Before for\n");
		for( i = 1; i < 4; i++) {
			MPI_Send(to_array, elem_per_proc, MPI_INT, i, i, comm_ring);
			printf("Sending to %d\n", i);
			to_array += elem_per_proc;
		} 		
		//f = fopen("/data/evl/victoras/array.txt", "r");
		end = clock();
		//free(array);
		//free(to_array);
	} else {
			printf("Receving for %d\n", myrank);
			MPI_Recv(local_array, elem_per_proc, MPI_INT, 0, myrank, comm_ring, &status);
			printf("Received at %d\n", myrank);
	}

  start = clock();
	int local_max = find_max(local_array, 0, elem_per_proc);
	printf("(%d): Local Max is %d\n", myrank, local_max);
	end = clock();
	printf("(Proc %d) Time it took to find the max:%.3f seconds\n", myrank, ((double)end - (double)start) * 10e-6);

	int leftrank, rightrank;
	MPI_Cart_shift(comm_ring, 0, -1, &rightrank, &leftrank);
	
	int sent_max[1];
	sent_max[0] = local_max;
	for (i = 0; i < numprocs; i++) {
		printf("Sending at %d from %d to %d:%d\n", myrank, leftrank, rightrank, sent_max[0]);
		MPI_Sendrecv_replace(sent_max, 1, MPI_INT, rightrank, 3, leftrank, 3, comm_ring, &status);
		printf("Received at %d:%d\n", myrank, sent_max[0]);
		if( sent_max[0] > local_max) {
			local_max = sent_max[0];
		}
	}
	if( myrank == 0) {
		printf("Max is:%d\n", local_max);
		end = clock();
		printf("(Proc %d) Time it took to find the max:%.3f seconds\n", myrank, ((double)end - (double)start) * 10e-6);

	}
  */
	MPI_Comm_free(&comm_ring);
	MPI_Finalize(); // Exit MPI
	//fclose(f);
	return 0;
}

int find_max(int *array, int start, int end) {
	int i, max = 0;
	for(i = start; i <end; i++) {
		if( array[i] > max ) max = array[i];
	}

	return max;
}
