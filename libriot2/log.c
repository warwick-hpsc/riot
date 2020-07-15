#include "bfile.h"
#include "filedb.h"
#include "ops.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
#include <sys/stat.h>

// Record is: Time, Rank, OP, FileID, Value

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 8388608
#endif

bfile *out;
char buffer[1024];

int rank = -1;

double zerotime = 0.0;

#ifdef JOBID
char *getJobID() {
	return getenv(JOBID);
}
#endif

double getTime() {
	struct timeval t;
	gettimeofday(&t, (struct timezone *)0);
	return (t.tv_sec + t.tv_usec*1.0e-6) - zerotime;
}

void addRecord(op operation, int fileid, off_t value) {
	if ((rank == -1) || (fileid == -1)) return;

	sprintf(buffer, "%9.7f\t%d\t%d\t%d\t%jd\n", getTime(), rank, operation, fileid, value);
	bwrite(out, buffer); 
}

char *generateFilename() {
	// Get running programs name for filename output
	char *progname = malloc(sizeof(char) * 100);
	progname[0] = '\0';

	FILE *fp = fopen("/proc/self/cmdline", "r");
	if (fp != NULL) {
		fread(progname, sizeof(char), 100, fp);
		fclose(fp);
	}

	char *tmp = strrchr(progname, '/');
	if (tmp != NULL) {
		strcpy(progname, tmp);
	}

	char *fn = malloc(sizeof(char) * 100);
#ifdef JOBID
	sprintf(fn, "%s/%s-%d.log", getJobID(), progname, rank);
#else
	sprintf(fn, "%s-%d.log", progname, rank);
#endif
	free(progname);
	return fn;
}


#pragma GCC visibility push(default)
int MPI_Init(int *argc, char ***argv) {
	int ret = PMPI_Init(argc, argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
#ifdef JOBID
	if (rank == 0) {
		if (getJobID() != NULL) mkdir(getJobID(), S_IRWXU);
	}
#endif

	MPI_Barrier(MPI_COMM_WORLD); // other ranks open the file too quickly!

	out = bopen(generateFilename(), BUFFER_SIZE);


	
	MPI_Barrier(MPI_COMM_WORLD);
	zerotime = getTime();
	// set rank

	return ret;
}

int MPI_Finalize() {
	int ret = PMPI_Finalize();

	bclose(out);
	char filedb[1024];
	sprintf(filedb, "%s.filedb", generateFilename());
	printFileDB(filedb);
	
	rank = -1;

	return ret;
}
#pragma GCC visibility pop




