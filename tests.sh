#!/bin/bash
./submit.sh -n 100 -k 2 -p 4 manual_scatter 100n
./submit.sh -n 100000 -k 2 -p 4 manual_scatter 100000n
./submit.sh -n 100000000 -k 2 -p 4 manual_scatter 100000000n
