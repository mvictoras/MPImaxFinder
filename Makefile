CC=mpicc

all: unified

unified: unified.c
			$(CC) -lm -o unified unified.c

clean:
	    rm -rf unified
