#!/bin/bash

#PBS -m be
#PBS -e ${proc}.${id}.error
#PBS -o ${proc}.${id}.output
#PBS -N ${proc}.${id}

num_nodes=$[$p*$k]
mpiexec -np $num_nodes ${HOME}/proj1/$proc -n $n -p $p -k $k 
