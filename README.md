MPImaxFinder
============


Program that finds the MAX number in an array of n numbers using MPI
For this project I implemented the find_max algorithm for use on clusters. The program creates a random array of n numbers, distributes the array across all the nodes of the cluster and then finds the max. The program also calculates the processing time, communication time and overall time to find the max. It outputs the results in either a human readable form, or in a tab separated output.

Source Code
============
git pull https://github.com/mvictoras/MPImaxFinder

Compiling
============
make

Running
============
Running

The project was run on ARGO, that is a University of Illinois at Chicago cluster available for students and staff. If you are on ARGO (or any cluster that processes are submitted using TORQUE), you need to run:

./submit.sh 
-n <# of numbers> 
-p <# of physical processors> 
-k <# of logical processors per node> 
-t <topology: ring, 2dmesh, hypercube, tree> 
-a <find_max algorithm: ring_shift, gather, reduction>
If your cluster allows you to run directly programs (without process queues, then you run:
mpirun -np <num_of_nodes> unified 
-n <# of numbers> 
-p <# of physical processors> 
-k <# of logical processors> 
-t <topology: ring, 2dmesh, hypercube, tree> 
-a <find_max algorithm: ring_shift, gather, reduction> 
[-c]

All the arguments are mandatory. The -c option is optional. By default the program prints the results into a human readable format, -c outputs in a tab separated format, so that the data can be easily read and processed by other processes.
