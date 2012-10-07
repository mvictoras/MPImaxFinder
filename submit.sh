#!/bin/bash

if ( ! getopts "k:n:p:" opt); then
  echo "Usage: `basename $0` -n <number of numbers> -p <number of physical processors> -k <number of logical processors>"
  exit 1
fi

while getopts ":k:n:p:" opt; do
    if ! [[ $OPTARG =~ ^[0-9]+$ ]] ; then
      echo "Option -$opt must be an integer"
      exit 1
    fi
      
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

echo "Running $7 Creating $n random numbers and using $p physical processors and $k logical processors"
num_nodes=$[$p*$k]
qsub -V -l nodes=$p:ppn=$k -v n=$n,p=$p,k=$k,proc=$7,id=$8 ${HOME}/proj1/process.sh 
