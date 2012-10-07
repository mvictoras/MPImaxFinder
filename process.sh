#!/bin/bash

#PBS -m be
#PBS -e ${proc}.error
#PBS -o ${proc}.output
#PBS -N ${proc}

num_nodes=$[$p*$k]
mpiexec -np $num_nodes ${HOME}/proj1/$proc -n $n -p $num_nodes -k $num_nodes 
