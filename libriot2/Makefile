CC=mpicc

JOBID="PBS_JOBID"

OPTS = -g -shared -fPIC -fvisibility=hidden -D_LARGEFILE64_SOURCE #-D_FILE_OFFSET_BITS=64 #-DPMPI #-DDEBUG
LINKOPTS = -g -shared -fvisibility=hidden -Wl,-soname,libriot.so.2 -o libriot.so.2.0.1 -ldl

ifdef JOBID
OPTS += -DJOBID='$(JOBID)'
endif

all: libriot 

libriot:
	rm -Rf *.so* # clean up any symlinks
	$(CC) $(OPTS) -c libc.c
	$(CC) $(OPTS) -c bfile.c
	$(CC) $(OPTS) -c log.c
	$(CC) $(OPTS) -c mpi.c
	$(CC) $(OPTS) -c filedb.c
	$(CC) $(LINKOPTS) libc.o bfile.o filedb.o log.o mpi.o
	ln -s libriot.so.2.0.1 libriot.so.2
	ln -s libriot.so.2 libriot.so
	
clean:
	rm -f *.o *.so *.so.*
