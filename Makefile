CC=mpicc

all: manual_scatter ring_scatter ring_shift ring_reduction unified

unified: unified.c
			$(CC) -o unified unified.c

manual_scatter: manual_scatter.c
			$(CC) -o manual_scatter manual_scatter.c

scatter: ring_scatter.c
			$(CC) -o ring_scatter ring_scatter.c

ring_shift: ring_shift.c
			$(CC) -o ring_shift ring_shift.c

ring_reduction: ring_reduction.c
			$(CC) -o ring_reduction ring_reduction.c

clean:
	    rm -rf manual_scatter scatter
