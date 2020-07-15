CC=mpicc

JOBID="PBS_JOBID"

ifdef JOBID
OPTS=-O0 -g -shared -fPIC -fvisibility=hidden -DJOBID='$(JOBID)' -DPLFS 
else
OPTS=-shared -fPIC -fvisibility=hidden
endif

all: libriot 

libriot: 
	$(CC) $(OPTS) -c libc.c
	$(CC) $(OPTS) -c mpi.c
	$(CC) $(OPTS) -c libriot.c
	$(CC) $(OPTS) -c ll.c
	$(CC) $(OPTS) -c utils.c
	$(CC) $(OPTS) -c plfs.c
	$(CC) $(OPTS) -ldl -o libriot.so libc.o mpi.o libriot.o ll.o utils.o plfs.o
	
clean:
	rm -f *.o *.so
