#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uthash.h"
#include "filedb.h"

typedef struct {
	int fh;
	int id;
	char *basename;
	void *mpifh;
	UT_hash_handle hh;
	UT_hash_handle hhP; // POSIX handle
	UT_hash_handle hhM; // MPI handle
} FileRecord;

static FileRecord *records = NULL;
static FileRecord *posixFiles = NULL;
static FileRecord *mpiFiles = NULL;

char *ignorePaths[] = {
	"/proc/",
	"/dev/",
	"/sys/",
	"/etc/",
	"/bin/",
	"/sbin/",
	"/usr/",
	"pipe"
};

const char *getBasename(const char *filename) {
	if (filename == NULL) return NULL;

	char *ret;
	return ((ret = strrchr(filename, '/')) != NULL) ? ret + 1 : filename;
}

int isIgnored(const char *path) {
	int i;
	if (path == NULL) return 1;
	for (i = 0; i < 8; i++) {
		if (strstr(path, ignorePaths[i]) != NULL) return 1;
	}
	return 0;
}

FileRecord *findFile(const char *basename) {
	FileRecord *current, *tmp;

	HASH_ITER(hh, records, current, tmp) {
		if (strcmp(current->basename, basename) == 0) return current;
	}
	
	return NULL;
}

int addFile(const char *filename) {
	if ((filename == NULL) || (strlen(filename) == 0) || (isIgnored(filename))) return -1;

	const char *basename = getBasename(filename);

	FileRecord *r;
	
	if ((r = findFile(basename)) == NULL) {
		r = malloc(sizeof(FileRecord));
		r->id = HASH_COUNT(records);
		r->basename = malloc((strlen(basename) + 1) * sizeof(char));
		strcpy(r->basename, basename);
		r->mpifh = NULL;
		r->fh = -1;
		HASH_ADD_INT(records, id, r);
	}

	return r->id;
}

int openPOSIXFile(int fh, const char *filename) {
	if ((filename == NULL) || (strlen(filename) == 0) || (isIgnored(filename))) return -1;

	const char *basename = getBasename(filename);
	
	FileRecord *r;
	
	if ((r = findFile(basename)) == NULL) return -1;
	
	r->fh = fh;
	
	HASH_ADD(hhP, posixFiles, fh, sizeof(int), r);
	
	return r->id;
}

int openMPIFile(void *mpifh, const char *filename) {
	if ((filename == NULL) || (strlen(filename) == 0) || (isIgnored(filename))) return -1;

	const char *basename = getBasename(filename);
	
	FileRecord *r;
	
	if ((r = findFile(basename)) == NULL) return -1;
	
	r->mpifh = mpifh;
	
	HASH_ADD(hhM, mpiFiles, mpifh, sizeof(void *), r);
	
	return r->id;
}

int getPOSIXFile(int fh) {
	FileRecord *r;
	
	HASH_FIND(hhP, posixFiles, &fh, sizeof(int), r);
	if (r == NULL) return -1;
	
	return r->id;
}

int getMPIFile(void *mpifh) {
	FileRecord *r;
	
	HASH_FIND(hhM, mpiFiles, &mpifh, sizeof(void *), r);
	if (r == NULL) return -1;
	
	return r->id;
}

int closePOSIXFile(int fh) {
	FileRecord *r;
	
	HASH_FIND(hhP, posixFiles, &fh, sizeof(int), r);
	if (r == NULL) return -1;
	
	HASH_DELETE(hhP, posixFiles, r);
	return 0;
}

int closeMPIFile(void *mpifh) {
	FileRecord *r;
	
	HASH_FIND(hhM, mpiFiles, &mpifh, sizeof(void *), r);
	if (r == NULL) return -1;
	
	HASH_DELETE(hhM, mpiFiles, r);
	return 0;
}

void printFileDB(char *filename) {
	FILE *out = fopen(filename, "w");
	
	FileRecord *current, *tmp;
	HASH_ITER(hh, records, current, tmp) {
		if (current->fh != -1)
			fprintf(out, "%d\t%s\n", current->id, current->basename);
	}
	fclose(out);
}
