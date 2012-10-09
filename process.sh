#!/bin/bash

#PBS -m be
#PBS -e ${proc}.${n}n.${p}.${k}.error
#PBS -o ${proc}.${n}n.${p}.${k}.output
#PBS -N ${proc}.${n}n.${p}.${k}

num_nodes=$[$p*$k]
mpiexec -np $num_nodes ${HOME}/proj1/unified -n $n -p $p -k $k -t $t -a $a -c
