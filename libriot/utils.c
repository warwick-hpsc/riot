#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>

#include "utils.h"
#include "mpi.h"

struct filerecord {
	char *fn;
	void *mpih;
};

struct filerecord filelookup[64];
int flptr = 0;

char *getFilename(int filedes) {
	char link[1024];
	char buf[1024];
	sprintf(link, "/proc/self/fd/%d", filedes);
	ssize_t t = readlink(link, buf, 1024);
	if (t != -1) buf[t] = '\0';
	else return NULL;
	char *fn = malloc((t + 1) * sizeof(char));
	strcpy(fn, buf);
	return fn;
}

char *getMPIFilename(void *mpih) {
	int i;
	for (i = 0; i < flptr; i++) {
		if (filelookup[i].fn == NULL) continue;
		if (filelookup[i].mpih == mpih) return filelookup[i].fn;
	}
	return NULL;
}

void setMPIH(char *filename, void *mpih) {
	filelookup[flptr].fn = malloc((strlen(filename) + 1) * sizeof(char));
	strcpy(filelookup[flptr].fn, filename);
	filelookup[flptr++].mpih = mpih;
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

	char *fn = malloc(sizeof(char) * 100);

#ifdef JOBID
	sprintf(fn, "%s/%s-%d.log", getJobID(), progname, getRank());
#else
	sprintf(fn, "%s-%d.log", progname, getRank());
#endif
	return fn;
}

char *checkOpenFlags(int flags) {
	char *flagarr[10];
	int i = 0;

	if ((flags & O_WRONLY) == O_WRONLY) flagarr[i++] = "O_WRONLY";
	if ((flags & O_RDWR) == O_RDWR) flagarr[i++] = "O_RDWR";

	if (i == 0) flagarr[i++] = "O_RDONLY";

	if ((flags & O_APPEND) == O_APPEND) flagarr[i++] = "O_APPEND";
	if ((flags & O_CREAT) == O_CREAT) flagarr[i++] = "O_CREAT";
	if ((flags & O_SYNC) == O_SYNC) flagarr[i++] = "O_SYNC";
	
	if ((O_SYNC != O_DSYNC) || (O_SYNC != O_RSYNC)) {
		if ((flags & O_DSYNC) == O_DSYNC) flagarr[i++] = "O_DSYNC";
		if ((flags & O_RSYNC) == O_RSYNC) flagarr[i++] = "O_RSYNC";
	}

	if ((flags & O_EXCL) == O_EXCL) flagarr[i++] = "O_EXCL";
	if ((flags & O_NOCTTY) == O_NOCTTY) flagarr[i++] = "O_NOCTTY";
	if ((flags & O_NONBLOCK) == O_NONBLOCK) flagarr[i++] = "O_NONBLOCK";
	if ((flags & O_TRUNC) == O_TRUNC) flagarr[i++] = "O_TRUNC";

	char *flagstr = malloc(sizeof(char) * 102);
	flagstr[0] = '\0';

	int j = 0;
	for (j = 0; j < i; j++) {
		strcat(flagstr, flagarr[j]);
		if (j != i-1) strcat(flagstr, " | ");
	}

	return flagstr;
}

char *checkFcntlFlags(int flags) {
	char *retval = malloc(sizeof(char) * 9);
	retval[0] = '\0';
	
	if (flags == F_DUPFD) strcat(retval, "F_DUPFD");
	else if (flags == F_GETFD) strcat(retval, "F_GETFD");
	else if (flags == F_SETFD) strcat(retval, "F_SETFD");
	else if (flags == F_GETFL) strcat(retval, "F_GETFL");
	else if (flags == F_SETFL) strcat(retval, "F_SETFL");
	else if (flags == F_GETOWN) strcat(retval, "F_GETOWN");
	else if (flags == F_SETOWN) strcat(retval, "F_SETOWN");
	else if (flags == F_GETLK) strcat(retval, "F_GETLK");
	else if (flags == F_SETLK) strcat(retval, "F_SETLK");
	else if (flags == F_SETLKW) strcat(retval, "F_SETLKW");

	return retval;
}

char *decodeModet(mode_t mode) {
	char *flagarr[12];
	int i = 0;

	if ((mode & S_ISUID) == S_ISUID) flagarr[i++] = "S_ISUID";
	if ((mode & S_ISGID) == S_ISGID) flagarr[i++] = "S_ISGID";
	if ((mode & S_ISVTX) == S_ISVTX) flagarr[i++] = "S_ISVTX";
	if ((mode & S_IRUSR) == S_IRUSR) flagarr[i++] = "S_IRUSR";
	if ((mode & S_IWUSR) == S_IWUSR) flagarr[i++] = "S_IWUSR";
	if ((mode & S_IXUSR) == S_IXUSR) flagarr[i++] = "S_IXUSR";
	if ((mode & S_IRGRP) == S_IRGRP) flagarr[i++] = "S_IRGRP";
	if ((mode & S_IWGRP) == S_IWGRP) flagarr[i++] = "S_IWGRP";
	if ((mode & S_IXGRP) == S_IXGRP) flagarr[i++] = "S_IXGRP";
	if ((mode & S_IROTH) == S_IROTH) flagarr[i++] = "S_IROTH";
	if ((mode & S_IWOTH) == S_IWOTH) flagarr[i++] = "S_IWOTH";
	if ((mode & S_IXOTH) == S_IXOTH) flagarr[i++] = "S_IXOTH";
	
	char *flagstr = malloc(sizeof(char) * 118);
	flagstr[0] = '\0';

	int j = 0;
	for (j = 0; j < i; j++) {
		strcat(flagstr, flagarr[j]);
		if (j != i-1) strcat(flagstr, " | ");
	}

	return flagstr;
}

char *decodeFlock(struct flock *filelock) {
	char *ltype;
	char *lwhence;

	switch (filelock->l_type) {
		case F_RDLCK: ltype = "F_RDLCK";
			      break;
		case F_WRLCK: ltype = "F_WRLCK";
			      break;
		case F_UNLCK: ltype = "F_UNLCK";
			      break;
		default: ltype = "F_UNKNO";
	}

	switch (filelock->l_whence) {
		case SEEK_SET: lwhence = "SEEK_SET";
			       break;
		case SEEK_CUR: lwhence = "SEEK_CUR";
			       break;
		case SEEK_END: lwhence = "SEEK_END";
			       break;
		default: lwhence = "SEEK_UNK";
	}

	char *strstruct = malloc(sizeof(char) * 100);
	sprintf(strstruct, "{ %s, %s, %d, %d, %d }", ltype, lwhence, filelock->l_start, filelock->l_len, filelock->l_pid);

	return strstruct;
}

double getTime() {
	struct timeval t;
	gettimeofday(&t, (struct timezone *)0);
	return (t.tv_sec + t.tv_usec*1.0e-6);
}

