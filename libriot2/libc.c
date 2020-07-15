#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>

#include "ops.h"
#include "log.h"
#include "filedb.h"

#ifndef STATIC
	#define FORWARD_DECLARE(ret,name,args) \
		ret (*__real_ ## name)args = NULL;
	#define MAP(func) \
		if (!(__real_ ## func)) { \
			__real_ ## func = dlsym(RTLD_NEXT, #func); \
			if (!(__real_ ## func)) fprintf(stderr, "Failed to link symbol: %s\n", #func); \
		}
	#define FUNCTION_DECLARE(func) func
#else
	#define FORWARD_DECLARE(ret, name, args) \
		extern ret __real_ ## name args;
	#define MAP(func)
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


FORWARD_DECLARE(int, open, (const char *path, int flags, ...));
FORWARD_DECLARE(int, close, (int fd));
FORWARD_DECLARE(ssize_t, write, (int fd, const void *buf, size_t count));
FORWARD_DECLARE(ssize_t, read, (int fd, void *buf, size_t count));
FORWARD_DECLARE(ssize_t, pread, (int fd, void *buf, size_t count, off_t offset));
FORWARD_DECLARE(ssize_t, pwrite, (int fd, const void *buf, size_t count, off_t offset));
FORWARD_DECLARE(int, fcntl, (int fd, int cmd, ...));

#if _FILE_OFFSET_BITS != 64
FORWARD_DECLARE(int, open64, (const char *path, int flags, ...));
FORWARD_DECLARE(ssize_t, pread64, (int fd, void *buf, size_t count, off64_t offset));
FORWARD_DECLARE(ssize_t, pwrite64, (int fd, const void *buf, size_t count, off64_t offset));
#endif

#pragma GCC visibility push(default)
int FUNCTION_DECLARE(open)(const char *path, int flags, ...) {
	DEBUG_ENTER;

	MAP(open);
	
	int ret;
	
	int fileid = addFile(path);

	if ((flags & O_CREAT) == O_CREAT) {
		va_list argf;
		va_start(argf, flags);
		mode_t mode = va_arg(argf, mode_t);
		va_end(argf);

		addRecord(BEGIN_OPEN, fileid, 0);
		
		ret = __real_open(path, flags, mode);

		addRecord(END_OPEN, fileid, 0);
	} else {
		addRecord(BEGIN_OPEN, fileid, 0);
		
		ret = __real_open(path, flags);

		addRecord(END_OPEN, fileid, 0);
	}
	
	openPOSIXFile(ret, path);
	
	DEBUG_EXIT;
	return ret;
}

int FUNCTION_DECLARE(close)(int fd) {
	DEBUG_ENTER;
	
	MAP(close);
	
	int ret;
	
	int fileid = getPOSIXFile(fd);
	
	addRecord(BEGIN_CLOSE, fileid, 0);
	
	ret = __real_close(fd);
	
	addRecord(END_CLOSE, fileid, 0);
	
	closePOSIXFile(fd);
	
	DEBUG_EXIT;
	return ret;
}

ssize_t FUNCTION_DECLARE(write)(int fd, const void *buf, size_t count) {
	DEBUG_ENTER;

	MAP(write);

	ssize_t ret;

	int fileid = getPOSIXFile(fd);

	addRecord(BEGIN_WRITE, fileid, lseek(fd, 0, SEEK_CUR));
	
	ret = __real_write(fd, buf, count);
	
	addRecord(END_WRITE, fileid, lseek(fd, 0, SEEK_CUR));

	DEBUG_EXIT;
	return ret;
}

ssize_t FUNCTION_DECLARE(read)(int fd, void *buf, size_t count) {
	DEBUG_ENTER;

	MAP(read);

	ssize_t ret;

	int fileid = getPOSIXFile(fd);

	addRecord(BEGIN_READ, fileid, lseek(fd, 0, SEEK_CUR));

	ret = __real_read(fd, buf, count);

	addRecord(END_READ, fileid, lseek(fd, 0, SEEK_CUR));

	DEBUG_EXIT;
	return ret;
}

ssize_t FUNCTION_DECLARE(pread)(int fd, void *buf, size_t count, off_t offset) {
	DEBUG_ENTER;

	MAP(pread);

	ssize_t ret;

	int fileid = getPOSIXFile(fd);

	addRecord(BEGIN_READ, fileid, offset);

	ret = __real_pread(fd, buf, count, offset);

	addRecord(END_READ, fileid, offset + ret);

	DEBUG_EXIT;
	return ret;
}

ssize_t FUNCTION_DECLARE(pwrite)(int fd, const void *buf, size_t count, off_t offset) {
	DEBUG_ENTER;

	MAP(pwrite);

	ssize_t ret;

	int fileid = getPOSIXFile(fd);

	addRecord(BEGIN_WRITE, fileid, offset);

	ret = __real_pwrite(fd, buf, count, offset);

	addRecord(END_WRITE, fileid, offset + ret);

	DEBUG_EXIT;
	return ret;
}

int FUNCTION_DECLARE(fcntl)(int fd, int cmd, ...) {
	DEBUG_ENTER;

	MAP(fcntl);
	
	int status = -1;
	
	int fileid = getPOSIXFile(fd);

	if ((cmd == F_GETLK) | (cmd == F_SETLK) | (cmd == F_SETLKW)) {
		va_list argf;
		va_start(argf, cmd);
		struct flock *lockst = va_arg(argf, struct flock *);
		va_end(argf);

		if ((cmd == F_SETLK) || (cmd == F_SETLKW)) {
			if (lockst->l_type == F_UNLCK) {
				addRecord(BEGIN_UNLOCK, fileid, lockst->l_start);
				status = __real_fcntl(fd, cmd, lockst);
				addRecord(END_UNLOCK, fileid, lockst->l_start + lockst->l_len);
			} else {
				addRecord(BEGIN_LOCK, fileid, lockst->l_start);
				status = __real_fcntl(fd, cmd, lockst);
				addRecord(END_LOCK, fileid, lockst->l_start + lockst->l_len);
			}
		}
	} else if ((cmd == F_DUPFD) | (cmd == F_SETFD) | (cmd == F_SETFL) | (cmd == F_SETOWN)) {
		va_list argf;
		va_start(argf, cmd);
		int arg = va_arg(argf, int);
		va_end(argf);
		
		status = __real_fcntl(fd, cmd, arg);
	} else {
		status = __real_fcntl(fd, cmd);
	}
	DEBUG_EXIT;
	return status;
}

#if _FILE_OFFSET_BITS != 64
int FUNCTION_DECLARE(open64)(const char *path, int flags, ...) {
	DEBUG_ENTER;

	MAP(open64);
	
	int ret;
	
	int fileid = addFile(path);
	
	if ((flags & O_CREAT) == O_CREAT) {
		va_list argf;
		va_start(argf, flags);
		mode_t mode = va_arg(argf, mode_t);
		va_end(argf);

		addRecord(BEGIN_OPEN, fileid, 0);
		
		ret = __real_open64(path, flags, mode);

		addRecord(END_OPEN, fileid, 0);
	} else {
		addRecord(BEGIN_OPEN, fileid, 0);
		
		ret = __real_open64(path, flags);

		addRecord(END_OPEN, fileid, 0);
	}
	
	openPOSIXFile(ret, path);
	
	DEBUG_EXIT;
	return ret;
}

ssize_t FUNCTION_DECLARE(pread64)(int fd, void *buf, size_t count, off64_t offset) {
	DEBUG_ENTER;

	MAP(pread64);

	ssize_t ret;

	int fileid = getPOSIXFile(fd);

	addRecord(BEGIN_READ, fileid, offset);

	ret = __real_pread64(fd, buf, count, offset);

	addRecord(END_READ, fileid, offset + ret);

	DEBUG_EXIT;
	return ret;
}

ssize_t FUNCTION_DECLARE(pwrite64)(int fd, const void *buf, size_t count, off64_t offset) {
	DEBUG_ENTER;

	MAP(pwrite64);

	ssize_t ret;

	int fileid = getPOSIXFile(fd);

	addRecord(BEGIN_WRITE, fileid, offset);

	ret = __real_pwrite64(fd, buf, count, offset);

	addRecord(END_WRITE, fileid, offset + ret);

	DEBUG_EXIT;
	return ret;
}
#endif

#pragma GCC visibility pop


