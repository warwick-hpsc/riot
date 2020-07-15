#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <string.h>
#include <mpi.h>

#include "filedb.h"
#include "ops.h"
#include "log.h"


#ifndef STATIC
	#ifdef PMPI
		#define FORWARD_DECLARE(name,args)
		#define MAP(func)
		#define FUNCTION(name) \
			P ## name
	#else
		#define FORWARD_DECLARE(name,args) \
			int (*__real_ ## name)args = NULL;
		#define MAP(func) \
			if (!(__real_ ## func)) { \
				__real_ ## func = dlsym(RTLD_NEXT, #func); \
				if (!(__real_ ## func)) fprintf(stderr, "Failed to link symbol: %s\n", #func); \
			}
		#define FUNCTION(name) \
			__real_ ## name
	#endif
		
	#define FUNCTION_DECLARE(func) func
#else
	#ifdef PMPI
		#define FORWARD_DECLARE(name, args)
		#define MAP(func)
		#define FUNCTION(name) \
			P ## name
	#else
		#define FORWARD_DECLARE(name, args) \
			extern int __real_ ## name args;
		#define MAP(func)
		#define FUNCTION(name) \
			__real_ ## name
	#endif

	#define FUNCTION_DECLARE(func) __wrap_ ## func
#endif

#ifdef DEBUG
	#define DEBUG_ENTER \
		printf("Enter %s\n", __FUNCTION__ );
	#define DEBUG_EXIT \
		printf("Exit %s\n", __FUNCTION__ );
#else
	#define DEBUG_ENTER
	#define DEBUG_EXIT
#endif


#pragma GCC visibility push(default)
FORWARD_DECLARE(MPI_File_open, (MPI_Comm comm, char *filename, int amode, MPI_Info info, MPI_File *fh));
FORWARD_DECLARE(MPI_File_close, (MPI_File *fh));

FORWARD_DECLARE(MPI_File_write, (MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status));
FORWARD_DECLARE(MPI_File_write_all, (MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status));
FORWARD_DECLARE(MPI_File_write_at, (MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status));
FORWARD_DECLARE(MPI_File_write_at_all, (MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status));
FORWARD_DECLARE(MPI_File_write_ordered, (MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status));
FORWARD_DECLARE(MPI_File_write_shared, (MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status));

FORWARD_DECLARE(MPI_File_read, (MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status));
FORWARD_DECLARE(MPI_File_read_all, (MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status));
FORWARD_DECLARE(MPI_File_read_at, (MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status));
FORWARD_DECLARE(MPI_File_read_at_all, (MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status));
FORWARD_DECLARE(MPI_File_read_ordered, (MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status));
FORWARD_DECLARE(MPI_File_read_shared, (MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status));

int FUNCTION_DECLARE(MPI_File_open)(MPI_Comm comm, char *filename, int amode, MPI_Info info, MPI_File *fh) {
	DEBUG_ENTER;
	
	MAP(MPI_File_open);
	
	int fileid = addFile(filename);
	
	addRecord(BEGIN_MPI_OPEN, fileid, 0);
	
	int ret = FUNCTION(MPI_File_open)(comm, filename, amode, info, fh);

	addRecord(END_MPI_OPEN, fileid, 0);

	openMPIFile(*fh, filename);
	
	DEBUG_EXIT;
	return ret;
}

int FUNCTION_DECLARE(MPI_File_close)(MPI_File *fh) {
	DEBUG_ENTER;
	
	MAP(MPI_File_close);

	int fileid = getMPIFile(*fh);

	addRecord(BEGIN_MPI_CLOSE, fileid, 0);
	
	int ret = FUNCTION(MPI_File_close)(fh);
	
	addRecord(END_MPI_CLOSE, fileid, 0);
	
	DEBUG_EXIT;
	return ret;
}

int FUNCTION_DECLARE(MPI_File_write)(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	DEBUG_ENTER;
	
	MAP(MPI_File_write);
	
	int fileid = getMPIFile(fh);

	MPI_Offset offset;
	MPI_Offset pos;

	MPI_File_get_position(fh, &offset);
	MPI_File_get_byte_offset(fh, offset, &pos);

	addRecord(BEGIN_MPI_WRITE, fileid, (long) pos);
	
	int ret = FUNCTION(MPI_File_write)(fh, buf, count, datatype, status);
	
	MPI_File_get_position(fh, &offset);
	MPI_File_get_byte_offset(fh, offset, &pos);
	addRecord(END_MPI_WRITE, fileid, (long) pos);
	
	DEBUG_EXIT;
	return ret;
}

int FUNCTION_DECLARE(MPI_File_write_all)(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	DEBUG_ENTER;
	
	MAP(MPI_File_write_all);
	
	int fileid = getMPIFile(fh);

	MPI_Offset disp;
	MPI_Datatype etype;
	MPI_Datatype ftype;
	char *datarep = malloc(sizeof(char) * 10);
	MPI_File_get_view(fh, &disp, &etype, &ftype, datarep);

	free(datarep);
	
	MPI_Aint lb;
	MPI_Aint extent;

	// Do something here to find out where offset starts in the ftype.

	MPI_Type_get_extent(ftype, &lb, &extent);

	addRecord(BEGIN_MPI_WRITE_ALL, fileid, (long) disp);

	int ret = FUNCTION(MPI_File_write_all)(fh, buf, count, datatype, status);
	
	// Perhaps find out where my ftype ends... or how many bytes have actually been written...

	addRecord(END_MPI_WRITE_ALL, fileid, (long) (disp + extent));

	DEBUG_EXIT;
	return ret;
}

int FUNCTION_DECLARE(MPI_File_write_at)(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	DEBUG_ENTER;
	
	MAP(MPI_File_write_at);

	int fileid = getMPIFile(fh);
	
	MPI_Offset disp;
	MPI_File_get_byte_offset(fh, offset, &disp);

	addRecord(BEGIN_MPI_WRITE, fileid, (long) disp);

	int ret = FUNCTION(MPI_File_write_at)(fh, offset, buf, count, datatype, status);

	int dsize;
	MPI_Type_size(datatype, &dsize);
	
	int realcount;
	MPI_Get_count(status, datatype, &realcount);
	
	addRecord(END_MPI_WRITE, fileid, (long) disp + ((long) realcount * (long) dsize));
	
	DEBUG_EXIT;
	return ret;
}

int FUNCTION_DECLARE(MPI_File_write_at_all)(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	DEBUG_ENTER;
	
	MAP(MPI_File_write_at_all);

	int fileid = getMPIFile(fh);
	
	MPI_Offset disp;
	MPI_File_get_byte_offset(fh, offset, &disp);
	
	addRecord(BEGIN_MPI_WRITE_ALL, fileid, (long) disp);
	
	int ret = FUNCTION(MPI_File_write_at_all)(fh, offset, buf, count, datatype, status);

	int dsize;
	MPI_Type_size(datatype, &dsize);

	int realcount;
	MPI_Get_count(status, datatype, &realcount);

	addRecord(END_MPI_WRITE_ALL, fileid, (long) disp + ((long) realcount * (long) dsize));
	
	DEBUG_EXIT;
	return ret;
}

int FUNCTION_DECLARE(MPI_File_write_ordered)(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	DEBUG_ENTER;
	
	MAP(MPI_File_write_ordered);

	int fileid = getMPIFile(fh);

	MPI_Offset pos;
	
	MPI_File_get_position_shared(fh, &pos);
	addRecord(BEGIN_MPI_WRITE_SHARED, fileid, (long) pos);

	int ret = FUNCTION(MPI_File_write_ordered)(fh, buf, count, datatype, status);

	MPI_File_get_position_shared(fh, &pos);
	addRecord(END_MPI_WRITE_SHARED, fileid, (long) pos);
	
	DEBUG_EXIT;
	return ret;
}

int FUNCTION_DECLARE(MPI_File_write_shared)(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	DEBUG_ENTER;
	
	MAP(MPI_File_write_shared);

	int fileid = getMPIFile(fh);

	MPI_Offset pos;
	
	MPI_File_get_position_shared(fh, &pos);
	addRecord(BEGIN_MPI_WRITE_SHARED, fileid, (long) pos);
	
	int ret = FUNCTION(MPI_File_write_shared)(fh, buf, count, datatype, status);
	
	MPI_File_get_position_shared(fh, &pos);
	addRecord(END_MPI_WRITE_SHARED, fileid, (long) pos);

	DEBUG_EXIT;
	return ret;
}

int FUNCTION_DECLARE(MPI_File_read)(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	DEBUG_ENTER;
	
	MAP(MPI_File_read);
	
	int fileid = getMPIFile(fh);

	MPI_Offset offset;
	MPI_Offset pos;
	
	MPI_File_get_position(fh, &offset);
	MPI_File_get_byte_offset(fh, offset, &pos);
	
	addRecord(BEGIN_MPI_READ, fileid, (long) pos);
	
	int ret = FUNCTION(MPI_File_read)(fh, buf, count, datatype, status);
	
	MPI_File_get_position(fh, &offset);
	MPI_File_get_byte_offset(fh, offset, &pos);
	addRecord(END_MPI_READ, fileid, (long) pos);

	DEBUG_EXIT;
	return ret;
}

int FUNCTION_DECLARE(MPI_File_read_all)(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	DEBUG_ENTER;
	
	MAP(MPI_File_read_all);

	int fileid = getMPIFile(fh);

	MPI_Offset disp;
	MPI_Datatype etype;
	MPI_Datatype ftype;
	char *datarep = malloc(sizeof(char) * 10);
	MPI_File_get_view(fh, &disp, &etype, &ftype, datarep);

	free(datarep);
	
	MPI_Aint lb;
	MPI_Aint extent;

	MPI_Type_get_extent(ftype, &lb, &extent);

	addRecord(BEGIN_MPI_READ_ALL, fileid, (long) disp);
	
	int ret = FUNCTION(MPI_File_read_all)(fh, buf, count, datatype, status);

	addRecord(END_MPI_READ_ALL, fileid, (long) disp + (long) extent);
	
	DEBUG_EXIT;
	return ret;
}

int FUNCTION_DECLARE(MPI_File_read_at)(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	DEBUG_ENTER;
	
	MAP(MPI_File_read_at);

	int fileid = getMPIFile(fh);
	
	MPI_Offset disp;
	MPI_File_get_byte_offset(fh, offset, &disp);
	addRecord(BEGIN_MPI_READ, fileid, (long) disp);

	int ret = FUNCTION(MPI_File_read_at)(fh, offset, buf, count, datatype, status);

	int dsize;
	MPI_Type_size(datatype, &dsize);
	
	int realcount;
	MPI_Get_count(status, datatype, &realcount);
	
	addRecord(END_MPI_READ, fileid, (long) disp + ((long) realcount * (long) dsize));
	

	DEBUG_EXIT;
	return ret;
}

int FUNCTION_DECLARE(MPI_File_read_at_all)(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	DEBUG_ENTER;
	
	MAP(MPI_File_read_at_all);
	
	int fileid = getMPIFile(fh);
	
	MPI_Offset disp;
	MPI_File_get_byte_offset(fh, offset, &disp);
	
	addRecord(BEGIN_MPI_READ_ALL, fileid, (long) disp);
	
	int ret = FUNCTION(MPI_File_read_at_all)(fh, offset, buf, count, datatype, status);
	
	int dsize;
	MPI_Type_size(datatype, &dsize);
	
	int realcount;
	MPI_Get_count(status, datatype, &realcount);
	
	addRecord(END_MPI_READ_ALL, fileid, (long) disp + ((long) realcount * (long) dsize));
	
	DEBUG_EXIT;
	return ret;
}

int FUNCTION_DECLARE(MPI_File_read_ordered)(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	DEBUG_ENTER;
	
	MAP(MPI_File_read_ordered);

	int fileid = getMPIFile(fh);

	MPI_Offset pos;
	
	MPI_File_get_position_shared(fh, &pos);
	addRecord(BEGIN_MPI_READ_SHARED, fileid, (long) pos);
	
	int ret = FUNCTION(MPI_File_read_ordered)(fh, buf, count, datatype, status);

	MPI_File_get_position_shared(fh, &pos);
	addRecord(END_MPI_READ_SHARED, fileid, (long) pos);
	
	DEBUG_EXIT;
	return ret;
}

int FUNCTION_DECLARE(MPI_File_read_shared)(MPI_File fh, void *buf, int count, MPI_Datatype datatype, MPI_Status *status) {
	DEBUG_ENTER;
	
	MAP(MPI_File_read_shared);

	int fileid = getMPIFile(fh);

	MPI_Offset pos;
	
	MPI_File_get_position_shared(fh, &pos);
	addRecord(BEGIN_MPI_READ_SHARED, fileid, (long) pos);
	
	int ret = FUNCTION(MPI_File_read_shared)(fh, buf, count, datatype, status);
	
	MPI_File_get_position_shared(fh, &pos);
	addRecord(END_MPI_READ_SHARED, fileid, (long) pos);

	DEBUG_EXIT;
	return ret;
}
#pragma GCC visibility pop

