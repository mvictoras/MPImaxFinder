# MPImaxFinder
============


Program that finds the MAX number in an array of n numbers using MPI
For this project I implemented the find_max algorithm for use on clusters. The program creates a random set of numbers on n nodes, distributes the array across the n nodes of the cluster and then finds the max. The program also calculates the processing time, communication time and overall time to find the max. It outputs the results in either a human readable form, or in a tab separated output.

When running the code you can also setup the topology (ring, 2dmesh, hypercube, tree) as well as the find_max algorithm that will be used: ring_shift, gather or reduction.

# Source Code
============
```git pull https://github.com/mvictoras/MPImaxFinder```

# Compiling
============
I have supplied a Makefile to make compilation easier.
```make```

You will need to have mpicc, the Open MPI C convenience wrapper installed on your system. If not, please modify the Makefile or compile manually. 

# Running
============
There are two scripts to help you submit your code to the cluster, and you might need to modify them based on the configuration of the machines you will be using. In this case, qsub is used to submit the jobs to the scheduler.

```submit.sh``` submits the job to the scheduler, while ```process.sh``` is the script that will be executed on each node. 
Note: we will be using 12 processes per node (see ```process.sh```) with a walltime of 10 minutes.

```
./submit.sh 
-s <size of sets of numbers> 
-n <# of nodes> 
-t <topology: ring, 2dmesh, hypercube, tree> 
-a <find_max algorithm: ring_shift, gather, reduction>
[-c] optional. Show stats computer-friendly stats.
```

All the arguments are mandatory. The -c option is optional. By default the program prints the results into a human readable format, -c outputs in a tab separated format, so that the data can be easily read and processed by other processes.

Example:
Using 2 nodes with a ring topology and the gather find_max algorithm, generate a set of 1000 numbers and find the max.
```./submit.sh 1000 -n 2 -t ring -a gather```

You can also submit the job in interactive mode:
```qsub -I -n 2 -t 10```

Now submit the job:
```mpiexec -n 24 ./unified -s 1000 -t ring -a gather```
Note: We are using 12 processes per node, sp 2 * 12 = 24

