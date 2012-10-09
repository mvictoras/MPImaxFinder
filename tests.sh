#!/bin/bash
##./submit.sh -n 1000 -p 6 -k 1 -t ring -a ring_shift
#./submit.sh -n 1000000 -p 6 -k 1 -t ring -a ring_shift 
#./submit.sh -n 100000000 -p 6 -k 1 -t ring -a ring_shift

./submit.sh -n 1000 -p 6 -k 4 -t ring -a ring_shift
./submit.sh -n 1000000 -p 6 -k 3 -t ring -a ring_shift 
#./submit.sh -n 100000000 -p 6 -k 2 -t ring -a ring_shift

##./submit.sh -n 1000 -p 4 -k 4 -t ring -a ring_shift
./submit.sh -n 1000000 -p 4 -k 3 -t ring -a ring_shift 
##./submit.sh -n 100000000 -p 4 -k 2 -t ring -a ring_shift

##./submit.sh -n 1000 -p 2 -k 4 -t ring -a ring_shift
##./submit.sh -n 1000000 -p 2 -k 3 -t ring -a ring_shift 
##./submit.sh -n 100000000 -p 2 -k 2 -t ring -a ring_shift

##./submit.sh -n 1000 -p 1 -k 4 -t ring -a ring_shift
##./submit.sh -n 1000000 -p 1 -k 3 -t ring -a ring_shift 
##./submit.sh -n 100000000 -p 1 -k 2 -t ring -a ring_shift

# reduction
#./submit.sh -n 1000 -p 6 -k 1 -t ring -a reduction
#./submit.sh -n 1000000 -p 6 -k 1 -t ring -a reduction 
#./submit.sh -n 100000000 -p 6 -k 1 -t ring -a reduction

#./submit.sh -n 1000 -p 6 -k 4 -t ring -a reduction
#./submit.sh -n 1000000 -p 6 -k 3 -t ring -a reduction 
#./submit.sh -n 100000000 -p 6 -k 2 -t ring -a reduction

##./submit.sh -n 1000 -p 4 -k 4 -t ring -a reduction
#./submit.sh -n 1000000 -p 4 -k 3 -t ring -a reduction 
##./submit.sh -n 100000000 -p 4 -k 2 -t ring -a reduction

##./submit.sh -n 1000 -p 2 -k 4 -t ring -a reduction
##./submit.sh -n 1000000 -p 2 -k 3 -t ring -a reduction 
##./submit.sh -n 100000000 -p 2 -k 2 -t ring -a reduction

##./submit.sh -n 1000 -p 1 -k 4 -t ring -a reduction
##./submit.sh -n 1000000 -p 1 -k 3 -t ring -a reduction 
##./submit.sh -n 100000000 -p 1 -k 2 -t ring -a reduction

# gather
#./submit.sh -n 1000 -p 6 -k 1 -t ring -a gather
#./submit.sh -n 1000000 -p 6 -k 1 -t ring -a gather 
#./submit.sh -n 100000000 -p 6 -k 1 -t ring -a gather

#./submit.sh -n 1000 -p 6 -k 4 -t ring -a gather
#./submit.sh -n 1000000 -p 6 -k 3 -t ring -a gather 
#./submit.sh -n 100000000 -p 6 -k 2 -t ring -a gather

##./submit.sh -n 1000 -p 4 -k 4 -t ring -a gather
##./submit.sh -n 1000000 -p 4 -k 3 -t ring -a gather 
##./submit.sh -n 100000000 -p 4 -k 2 -t ring -a gather

##./submit.sh -n 1000 -p 2 -k 4 -t ring -a gather
##./submit.sh -n 1000000 -p 2 -k 3 -t ring -a gather 
##./submit.sh -n 100000000 -p 2 -k 2 -t ring -a gather

##./submit.sh -n 1000 -p 1 -k 4 -t ring -a gather
##./submit.sh -n 1000000 -p 1 -k 3 -t ring -a gather 
##./submit.sh -n 100000000 -p 1 -k 2 -t ring -a gather

#serial
##./submit.sh -n 1000 -p 1 -k 1 -t ring -a ring_shift
##./submit.sh -n 1000000 -p 1 -k 1 -t ring -a ring_shift 
##./submit.sh -n 100000000 -p 1 -k 1 -t ring -a ring_shift

##./submit.sh -n 1000 -p 1 -k 1 -t ring -a reduction
##./submit.sh -n 1000000 -p 1 -k 1 -t ring -a reduction 
##./submit.sh -n 100000000 -p 1 -k 1 -t ring -a reduction

##./submit.sh -n 1000 -p 1 -k 1 -t ring -a gather
##./submit.sh -n 1000000 -p 1 -k 1 -t ring -a gather 
##./submit.sh -n 100000000 -p 1 -k 1 -t ring -a gather
