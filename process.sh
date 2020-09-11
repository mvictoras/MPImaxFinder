#!/bin/bash

NODES=`cat $COBALT_NODEFILE | wc -l`
PROCS=$((NODES * 12))

while getopts ":s:t:a:" opt; do
    case $opt in
    s)
      s=$OPTARG
      ;;
    t)
      t=$OPTARG
      ;;
    a)
      a=$OPTARG
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
    :)
      echo "Option -$OPTARG requires an argument." >&2
      exit 1
      ;;
  esac
done


mpiexec -f $COBALT_NODEFILE -n $PROCS ./unified -s $s -t $t -a $a
