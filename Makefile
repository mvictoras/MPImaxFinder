all:: manual_scatter

manual_scatter: manual_scatter.c
	    mpicc -o manual_scatter manual_scatter.c

clean:
	    rm -rf manual_scatter
