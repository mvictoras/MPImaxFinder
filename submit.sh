#!/bin/bash

if ( ! getopts "s:n:p:t:a:" opt); then
  echo "Usage: `basename $0` -s <size of number set> -n <node count> -t <ring, hypercube, 2dmesh, tree> -a <ring_shift, reduction, gather>"
  exit 1
fi
while getopts ":k:s:n:t:a:" opt; do
    case $opt in
    s)
      s=$OPTARG
      ;;
    n)
      n=$OPTARG
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

echo "Creating $n random numbers using $n nodes"
qsub -n $n -t 10 ./process.sh -s $s -t $t -a $a
