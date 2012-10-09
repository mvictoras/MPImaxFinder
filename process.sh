#!/bin/bash

#PBS -m be
#PBS -e ${t}.${a}.${n}n.${p}.${k}.error
#PBS -o ${t}.${a}.${n}n.${p}.${k}.output
#PBS -N unified

num_nodes=$[$p*$k]
mpiexec -np $num_nodes ${HOME}/proj1/unified -n $n -p $p -k $k -t $t -a $a -c
