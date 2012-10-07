CC=mpicc
SOURCES=manual_scatter.c scatter.c

all: manual_scatter scatter

manual_scatter: manual_scatter.c
			$(CC) -o manual_scatter manual_scatter.c

scatter: scatter.c
			$(CC) -o scatter scatter.c

clean:
	    rm -rf manual_scatter scatter
