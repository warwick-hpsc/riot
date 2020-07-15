#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <dlfcn.h>

#include "ll.h"
#include "utils.h"
#include "libriot.h"

#include "mpi.h"

ll_list *list;
int cnt;
int maxcnt = 0;

int mpitrace = 0;

char *filename = NULL;
char *jobid = NULL;

double zerotime;
int initialised = 0;

int critical = 0;

void add(char *data) {
	while (critical != 0);
	critical = 1;
	ll_addtotail(list, data);
	critical = 0;

	cnt++;
	if (maxcnt == 0) return;
	if (cnt >= maxcnt) {
		if (filename == NULL)
			filename = generateFilename();
		
		ll_dumptofile(list, filename);
		
		cnt = 0;
	}
}

void setZero(double t) {
	zerotime = t;
}

void __attribute__ ((constructor)) my_init() {
	if (initialised == 0) {
		list = ll_create();
		cnt = 0;
		zerotime = getTime();

		// set up functions
		if (!_open) _open = dlsym(RTLD_NEXT, "open");
		if (!_open64) _open64 = dlsym(RTLD_NEXT, "open64");
		if (!_close) _close = dlsym(RTLD_NEXT, "close");
		if (!_read) _read = dlsym(RTLD_NEXT, "read");
		if (!_pread) _pread = dlsym(RTLD_NEXT, "pread");
		if (!_pread64) _pread64 = dlsym(RTLD_NEXT, "pread64");
		if (!_write) _write = dlsym(RTLD_NEXT, "write");
		if (!_pwrite) _pwrite = dlsym(RTLD_NEXT, "pwrite");
		if (!_pwrite64) _pwrite64 = dlsym(RTLD_NEXT, "pwrite64");
		if (!_lockf) _lockf = dlsym(RTLD_NEXT, "lockf");
		if (!_fsync) _fsync = dlsym(RTLD_NEXT, "fsync");
		if (!_fcntl) _fcntl = dlsym(RTLD_NEXT, "fcntl");
		if (!_lseek) _lseek = dlsym(RTLD_NEXT, "lseek");

		//_mmap = dlsym(RTLD_NEXT, "mmap");

		// call init mpi function too.
		_init_mpi();

		initialised = 1;
	}
}

/*void __attribute__ ((destructor)) my_fini() {
	writeOut();
}*/

void writeOut() {
	if (filename == NULL)
		filename = generateFilename();

	ll_dumptofile(list, filename);
}

