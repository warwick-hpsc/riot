#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <string.h>
#include <mpi.h>

#include "mpi.h"
#include "libriot.h"
#include "utils.h"

int rank = -1;

#pragma GCC visibility push(default)
double starttime;
double stoptime;

extern int mpitrace;

char buff[2048];

// Things for non blocking... MPI_Wait() and MPIO_Wait()... catch stuff between the iread/iwrite and them... see what happens!

int (*_MPI_Init)(int *argc, char ***argv);
int (*_MPI_Finalize)();

int (*_MPI_File_open)(MPI_Comm comm, char *filename, int amode, MPI_Info info, MPI_File *fh);

int (*_MPI_File_write)(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
int (*_MPI_File_write_all)(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
//_MPI_File_write_all_begin
//_MPI_File_write_all_end
int (*_MPI_File_write_at)(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
int (*_MPI_File_write_at_all)(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
//_MPI_File_write_at_all_begin
//_MPI_File_write_at_all_end
int (*_MPI_File_write_ordered)(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
//_MPI_File_write_ordered_begin
//_MPI_File_write_ordered_end
int (*_MPI_File_write_shared)(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);

int (*_MPI_File_read)(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
int (*_MPI_File_read_all)(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
//_MPI_File_read_all_begin
//_MPI_File_read_all_end
int (*_MPI_File_read_at)(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
int (*_MPI_File_read_at_all)(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
//_MPI_File_read_at_all_begin
//_MPI_File_read_at_all_end
int (*_MPI_File_read_ordered)(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);
//_MPI_File_read_ordered_begin
//_MPI_File_read_ordered_end
int (*_MPI_File_read_shared)(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status);

void _init_mpi() {
	if (!_MPI_Init) _MPI_Init = dlsym(RTLD_NEXT, "MPI_Init");
	if (!_MPI_Finalize) _MPI_Finalize = dlsym(RTLD_NEXT, "MPI_Finalize");

	if (!_MPI_File_open) _MPI_File_open = dlsym(RTLD_NEXT, "MPI_File_open");

	if (!_MPI_File_write) _MPI_File_write = dlsym(RTLD_NEXT, "MPI_File_write");
	if (!_MPI_File_write_all) _MPI_File_write_all = dlsym(RTLD_NEXT, "MPI_File_write_all");
	if (!_MPI_File_write_at) _MPI_File_write_at = dlsym(RTLD_NEXT, "MPI_File_write_at");
	if (!_MPI_File_write_at_all) _MPI_File_write_at_all = dlsym(RTLD_NEXT, "MPI_File_write_at_all");
	if (!_MPI_File_write_ordered) _MPI_File_write_ordered = dlsym(RTLD_NEXT, "MPI_File_write_ordered");
	if (!_MPI_File_write_shared) _MPI_File_write_shared = dlsym(RTLD_NEXT, "MPI_File_write_shared");
	
	if (!_MPI_File_read) _MPI_File_read = dlsym(RTLD_NEXT, "MPI_File_read");
	if (!_MPI_File_read_all) _MPI_File_read_all = dlsym(RTLD_NEXT, "MPI_File_read_all");
	if (!_MPI_File_read_at) _MPI_File_read_at = dlsym(RTLD_NEXT, "MPI_File_read_at");
	if (!_MPI_File_read_at_all) _MPI_File_read_at_all = dlsym(RTLD_NEXT, "MPI_File_read_at_all");
	if (!_MPI_File_read_ordered) _MPI_File_read_ordered = dlsym(RTLD_NEXT, "MPI_File_read_ordered");
	if (!_MPI_File_read_shared) _MPI_File_read_shared = dlsym(RTLD_NEXT, "MPI_File_read_shared");

}

int MPI_Init(int *argc, char ***argv) {
	if (initialised == 0) my_init();

	int ret = _MPI_Init(argc, argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// if we're the root processor.. On init create the jobid folder for all output
#ifdef JOBID
	if (rank == 0) {
		if (getJobID() != NULL) mkdir(getJobID(), S_IRWXU);
	}
#endif

	MPI_Barrier(MPI_COMM_WORLD);
	setZero(getTime());
	return ret;
}

int MPI_Finalize() {
	if (initialised == 0) my_init();

	int ret = _MPI_Finalize();

	writeOut();

	return ret;
}


int MPI_File_open(MPI_Comm comm, char *filename, int amode, MPI_Info info, MPI_File *fh) {
	if (initialised == 0) my_init();

	sprintf(buff, "## _BEGIN_ MPI_File_open(%s), %9.7f", filename, getTime() - zerotime);
	add(buff);

	mpitrace = 1;
	int ret =_MPI_File_open(comm, filename, amode, info, fh);
	mpitrace = 0;

	sprintf(buff, "## _END_ MPI_File_open(%s), %9.7f", filename, getTime() - zerotime);
	add(buff);

	setMPIH(filename, *fh);

	return ret;
}

int MPI_File_write(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	if (initialised == 0) my_init();

	char *fn = getMPIFilename(fh);

	sprintf(buff, "## _BEGIN_ MPI_File_write(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	mpitrace = 1;
	int ret = _MPI_File_write(fh, buf, count, datatype, status);
	mpitrace = 0;

	sprintf(buff, "## _END_ MPI_File_write(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	return ret;
}

int MPI_File_write_all(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	if (initialised == 0) my_init();

	char *fn = getMPIFilename(fh);
	
	sprintf(buff, "## _BEGIN_ MPI_File_write_all(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	mpitrace = 1;
	int ret = _MPI_File_write_all(fh, buf, count, datatype, status);
	mpitrace = 0;
	
	sprintf(buff, "## _END_ MPI_File_write_all(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	return ret;
}

int MPI_File_write_at(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	if (initialised == 0) my_init();

	char *fn = getMPIFilename(fh);
	
	sprintf(buff, "## _BEGIN_ MPI_File_write_at(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	mpitrace = 1;
	int ret = _MPI_File_write_at(fh, offset, buf, count, datatype, status);
	mpitrace = 0;
	
	sprintf(buff, "## _END_ MPI_File_write_at(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	return ret;
}

int MPI_File_write_at_all(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	if (initialised == 0) my_init();

	char *fn = getMPIFilename(fh);
	
	sprintf(buff, "## _BEGIN_ MPI_File_write_at_all(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	mpitrace = 1;
	int ret = _MPI_File_write_at_all(fh, offset, buf, count, datatype, status);
	mpitrace = 0;

	sprintf(buff, "## _END_ MPI_File_write_at_all(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	return ret;
}

int MPI_File_write_ordered(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	if (initialised == 0) my_init();

	char *fn = getMPIFilename(fh);
	
	sprintf(buff, "## _BEGIN_ MPI_File_write_ordered(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	mpitrace = 1;
	int ret = _MPI_File_write_ordered(fh, buf, count, datatype, status);
	mpitrace = 0;
	sprintf(buff, "## _END_ MPI_File_write_ordered(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	return ret;
}

int MPI_File_write_shared(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	if (initialised == 0) my_init();

	char *fn = getMPIFilename(fh);
	
	sprintf(buff, "## _BEGIN_ MPI_File_write_shared(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	mpitrace = 1;
	int ret = _MPI_File_write_shared(fh, buf, count, datatype, status);
	mpitrace = 0;
	
	sprintf(buff, "## _END_ MPI_File_write_shared(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	return ret;
}

int MPI_File_read(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	if (initialised == 0) my_init();

	char *fn = getMPIFilename(fh);
	
	sprintf(buff, "## _BEGIN_ MPI_File_read(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	mpitrace = 1;
	int ret = _MPI_File_read(fh, buf, count, datatype, status);
	mpitrace = 0;
	
	sprintf(buff, "## _END_ MPI_File_read(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	return ret;
}

int MPI_File_read_all(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	if (initialised == 0) my_init();

	char *fn = getMPIFilename(fh);
	
	sprintf(buff, "## _BEGIN_ MPI_File_read_all(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	mpitrace = 1;
	int ret = _MPI_File_read_all(fh, buf, count, datatype, status);
	mpitrace = 0;
	
	sprintf(buff, "## _END_ MPI_File_read_all(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	return ret;
}

int MPI_File_read_at(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	if (initialised == 0) my_init();

	char *fn = getMPIFilename(fh);
	
	sprintf(buff, "## _BEGIN_ MPI_File_read_at(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	mpitrace = 1;
	int ret = _MPI_File_read_at(fh, offset, buf, count, datatype, status);
	mpitrace = 0;

	sprintf(buff, "## _END_ MPI_File_read_at(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	return ret;
}

int MPI_File_read_at_all(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	if (initialised == 0) my_init();

	char *fn = getMPIFilename(fh);
		
	sprintf(buff, "## _BEGIN_ MPI_File_read_at_all(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	mpitrace = 1;
	int ret = _MPI_File_read_at_all(fh, offset, buf, count, datatype, status);
	mpitrace = 0;

	sprintf(buff, "## _END_ MPI_File_read_at_all(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	return ret;
}

int MPI_File_read_ordered(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	if (initialised == 0) my_init();

	char *fn = getMPIFilename(fh);
					
	sprintf(buff, "## _BEGIN_ MPI_File_read_ordered(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	mpitrace = 1;
	int ret = _MPI_File_read_ordered(fh, buf, count, datatype, status);
	mpitrace = 0;
										
	sprintf(buff, "## _END_ MPI_File_read_ordered(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	return ret;
}

int MPI_File_read_shared(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	if (initialised == 0) my_init();

	char *fn = getMPIFilename(fh);
	
	sprintf(buff, "## _BEGIN_ MPI_File_read_shared(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	mpitrace = 1;
	int ret = _MPI_File_read_shared(fh, buf, count, datatype, status);
	mpitrace = 0;

	sprintf(buff, "## _END_ MPI_File_read_shared(%s), %9.7f", fn, getTime() - zerotime);
	add(buff);

	return ret;
}
#pragma GCC visibility pop

int getRank() {
	return rank;
}

#ifdef JOBID
char *getJobID() {
	return getenv(JOBID);
}
#endif

