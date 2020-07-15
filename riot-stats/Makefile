CC=gcc

OPTS=-O3 -g 

all: riot-stats 

riot-stats:
	$(CC) $(OPTS) -c parse.c
	$(CC) $(OPTS) -c stats.c
	$(CC) $(OPTS) -c utils.c
	$(CC) $(OPTS) -c riot-stats.c
	$(CC) $(OPTS) -c plfs.c
	$(CC) $(OPTS) -o riot-stats parse.o stats.o utils.o riot-stats.o plfs.o
	
clean:
	rm -f riot-stats *.o
