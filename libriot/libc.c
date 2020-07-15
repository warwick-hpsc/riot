#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <dlfcn.h>

#include "libriot.h"
#include "utils.h"

#ifdef PLFS
#include "plfs.h"
#define FILENAME getPath(getFilename(filedes))
#else
#define FILENAME getFilename(filedes)
#endif

char buffer[2048];
double btime;
double etime;

extern double zerotime;
extern int initialised;

extern int mpitrace;

extern int (*_open)(const char *fn, int oflags, ...);
extern int (*_open64)(const char *fn, int oflags, ...);
extern int (*_close)(int filedes);

extern ssize_t (*_read)(int filedes, void *buf, size_t nbyte);
extern ssize_t (*_pread)(int filedes, void *buf, size_t nbyte, off_t offset);
extern ssize_t (*_pread64)(int filedes, void *buf, size_t nbyte, off64_t offset);

extern ssize_t (*_write)(int filedes, const void *buf, size_t nbyte);
extern ssize_t (*_pwrite)(int filedes, const void *buf, size_t nbyte, off_t offset);
extern ssize_t (*_pwrite64)(int filedes, const void *buf, size_t nbyte, off64_t offset);

extern int (*_lockf)(int filedes, int function, off_t size);
extern int (*_fsync)(int filedes);
extern int (*_fcntl)(int filedes, int cmd, ...);
extern off_t (*_lseek)(int filedes, off_t offset, int whence);

//extern void *(*_mmap)(void *addr, size_t len, int prot, int flags, int fildes, off_t off);

#pragma GCC visibility push(default)
int open(const char *fn, int oflags, ...) {
	if (initialised == 0) my_init();

	int filedes = -1;

	if ((oflags & O_CREAT) == O_CREAT) {
		va_list argf;
		va_start(argf, oflags);
		int mode = va_arg(argf, mode_t);
		va_end(argf);

		btime = getTime();
		filedes = _open(fn, oflags, mode);
		etime = getTime();

		if (filedes == -1) return filedes;

		if (mpitrace == 1) {
			sprintf(buffer, "open, %s, %d, %9.7f, %9.7f", FILENAME, filedes, btime-zerotime, etime-zerotime);
			add(buffer);
		}
	} else {
		btime = getTime();
		filedes = _open(fn, oflags);
		etime = getTime();

		if (filedes == -1) return filedes;

		if (mpitrace == 1) {
			sprintf(buffer, "open, %s, %d, %9.7f, %9.7f", FILENAME, filedes, btime-zerotime, etime-zerotime);
			add(buffer);
		}
	}

	return filedes;
}

int open64(const char *fn, int oflags, ...) {
	if (initialised == 0) my_init();
	
	int filedes = -1;

	if ((oflags & O_CREAT) == O_CREAT) {
		va_list argf;
		va_start(argf, oflags);
		int mode = va_arg(argf, mode_t);
		va_end(argf);

		btime = getTime();
		filedes = _open64(fn, oflags, mode);
		etime = getTime();
		
		if (filedes == -1) return filedes;

		if (mpitrace == 1) {
			sprintf(buffer, "open, %s, %d, %9.7f, %9.7f", FILENAME, filedes, btime-zerotime, etime-zerotime);
			add(buffer);
		}
	} else {
		btime = getTime();
		filedes = _open64(fn, oflags);
		etime = getTime();

		if (filedes == -1) return filedes;

		if (mpitrace == 1) {
			sprintf(buffer, "open, %s, %d, %9.7f, %9.7f", FILENAME, filedes, btime-zerotime, etime-zerotime);
			add(buffer);
		}
	}

	return filedes;
}

int close(int filedes) {
	if (initialised == 0) my_init();
	
	btime = getTime();
	int status = _close(filedes);
	etime = getTime();

	if (mpitrace == 1) {
		sprintf(buffer, "close, %s, %d, %9.7f, %9.7f", FILENAME, status, btime-zerotime, etime-zerotime);
		add(buffer);
	}

	return status;
}

ssize_t read(int filedes, void *buf, size_t nbyte) {
	if (initialised == 0) my_init();
	
	btime = getTime();
	ssize_t status = _read(filedes, buf, nbyte);
	etime = getTime();

	if (mpitrace == 1) {
		sprintf(buffer, "read, %s, %d, %9.7f, %9.7f", FILENAME, status, btime-zerotime, etime-zerotime);
		add(buffer);
	}

	return status;
}

ssize_t pread(int filedes, void *buf, size_t nbyte, off_t offset) {
	if (initialised == 0) my_init();

	btime = getTime();
	ssize_t status = _pread(filedes, buf, nbyte, offset);
	etime = getTime();

	if (mpitrace == 1) {
		sprintf(buffer, "read, %s, %d, %9.7f, %9.7f", FILENAME, status, btime-zerotime, etime-zerotime);
		add(buffer);
	}

	return status;
}


ssize_t pread64(int filedes, void *buf, size_t nbyte, off64_t offset) {
	if (initialised == 0) my_init();

	btime = getTime();
	ssize_t status = _pread64(filedes, buf, nbyte, offset);
	etime = getTime();

	if (mpitrace == 1) {
		sprintf(buffer, "read, %s, %d, %9.7f, %9.7f", FILENAME, status, btime-zerotime, etime-zerotime);
		add(buffer);
	}

	return status;
}

ssize_t write(int filedes, const void *buf, size_t nbyte) {
	if (initialised == 0) my_init();
	
	btime = getTime();
	ssize_t status = _write(filedes, buf, nbyte);
	etime = getTime();

	if (mpitrace == 1) {
		sprintf(buffer, "write, %s, %d, %9.7f, %9.7f", FILENAME, status, btime-zerotime, etime-zerotime);
		add(buffer);
	}

	return status;
}

ssize_t pwrite64(int filedes, const void *buf, size_t nbyte, off64_t offset) {
	if (initialised == 0) my_init();

	btime = getTime();
	ssize_t status = _pwrite64(filedes, buf, nbyte, offset);
	etime = getTime();

	if (mpitrace == 1) {
		sprintf(buffer, "write, %s, %d, %9.7f, %9.7f", FILENAME, status, btime-zerotime, etime-zerotime);
		add(buffer);
	}

	return status;
}

ssize_t pwrite(int filedes, const void *buf, size_t nbyte, off_t offset) {
	if (initialised == 0) my_init();

	btime = getTime();
	ssize_t status = _pwrite(filedes, buf, nbyte, offset);
	etime = getTime();

	if (mpitrace == 1) {
		sprintf(buffer, "write, %s, %d, %9.7f, %9.7f", FILENAME, status, btime-zerotime, etime-zerotime);
		add(buffer);
	}

	return status;
}

int lockf(int filedes, int function, off_t size) {
	if (initialised == 0) my_init();
	
	btime = getTime();
	int status = _lockf(filedes, function, size);
	etime = getTime();

	if (mpitrace == 1) {
		if ((function == F_LOCK) || (function == F_TLOCK))
			sprintf(buffer, "lock, %s, %d, %9.7f, %9.7f", FILENAME, status, btime-zerotime, etime-zerotime);
		else if (function = F_ULOCK)
			sprintf(buffer, "unlock, %s, %d, %9.7f, %9.7f", FILENAME, status, btime-zerotime, etime-zerotime);
		add(buffer);
	}

	return status;
}

int fsync(int filedes) {
	if (initialised == 0) my_init();
	
	btime = getTime();
	int status = _fsync(filedes);
	etime = getTime();

	if (mpitrace == 1) {
		sprintf(buffer, "sync, %s, %d, %9.7f, %9.7f", FILENAME, status, btime-zerotime, etime-zerotime);
		add(buffer);
	}

	return status;
}

int fcntl(int filedes, int cmd, ...) {
	if (initialised == 0) my_init();
	
	int status = -1;

	if ((cmd == F_DUPFD) | (cmd == F_SETFD) | (cmd == F_SETFL) | (cmd == F_SETOWN)) {
		va_list argf;
		va_start(argf, cmd);
		int arg = va_arg(argf, int);
		va_end(argf);

		btime = getTime();
		status = _fcntl(filedes, cmd, arg);
		etime = getTime();

		if (mpitrace == 1) {
			sprintf(buffer, "fcntl, %s, %d, %9.7f, %9.7f", FILENAME, status, btime-zerotime, etime-zerotime);
			add(buffer);
		}
	} else if ((cmd == F_GETLK) | (cmd == F_SETLK) | (cmd == F_SETLKW)) {
		va_list argf;
		va_start(argf, cmd);
		struct flock *lockst = va_arg(argf, struct flock *);
		va_end(argf);

		btime = getTime();
		status = _fcntl(filedes, cmd, lockst);
		etime = getTime();

		if (mpitrace == 1) {
			if ((cmd == F_SETLK) || (cmd == F_SETLKW)) {
				if (lockst->l_type == F_UNLCK)
					sprintf(buffer, "unlock, %s, %d, %9.7f, %9.7f", FILENAME, status, btime-zerotime, etime-zerotime);
				else
					sprintf(buffer, "lock, %s, %d, %9.7f, %9.7f", FILENAME, status, btime-zerotime, etime-zerotime);
			} else {
				sprintf(buffer, "fcntl, %s, %d, %9.7f, %9.7f", FILENAME, status, btime-zerotime, etime-zerotime);
			}
			add(buffer);
		}
	} else {
		btime = getTime();
		status = _fcntl(filedes, cmd);
		etime = getTime();
		
		if (mpitrace == 1) {
			sprintf(buffer, "fcntl, %s, %d, %9.7f, %9.7f", FILENAME, status, btime-zerotime, etime-zerotime);
			add(buffer);
		}
	}

	return status;
}

off_t lseek(int filedes, off_t offset, int whence) {
	if (initialised == 0) my_init();
	
	btime = getTime();
	off_t status = _lseek(filedes, offset, whence);
	etime = getTime();

	sprintf(buffer, "seek, %s, %d, %9.7f, %9.7f", FILENAME, status, btime-zerotime, etime-zerotime);

	return status;
}

/*void *mmap(void *addr, size_t len, int prot, int flags, int filedes, off_t off) {
	if (initialised == 0) my_init();

	btime = getTime();
	void *ret = _mmap(addr, len, prot, flags, filedes, off);
	etime = getTime();


	if (mpitrace == 1) {
		sprintf(buffer, "mmap, %s, %d, %9.7f, %9.7f", FILENAME, len, btime-zerotime, etime-zerotime);
		printf("%s\n", buffer);
		add(buffer);
	}

	return ret;
}*/

#pragma GCC visibility pop

