#!/bin/bash

if ( ! getopts "k:n:p:t:a:" opt); then
  echo "Usage: `basename $0` -n <number of numbers> -p <number of physical processors> -k <number of logical processors> -t <ring, hypercube, 2dmesh, tree> -a <ring_shift, reduction, gather>"
  exit 1
fi
while getopts ":k:n:p:t:a:" opt; do
    case $opt in
    k)
      k=$OPTARG
      ;;
    n)
      n=$OPTARG
      ;;
    p)
      p=$OPTARG
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

echo "Creating $n random numbers and using $p physical processors and $k logical processors"
num_nodes=$[$p*$k]
qsub -V -l nodes=$p:ppn=$k -v n=$n,p=$p,k=$k,t=$t,a=$a ${HOME}/proj1/process.sh 
