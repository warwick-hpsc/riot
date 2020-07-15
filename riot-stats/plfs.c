#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	char *mnt;
	char **bk;
	int cnt;
} mntpnt;

mntpnt *getMounts(int *cnt) {
	FILE *fphome, *fpetc;

	char *filename = malloc((strlen(getenv("HOME") + 9)) * sizeof(char));
	sprintf(filename, "%s/.plfsrc", getenv("HOME"));

	// Try to open etc plfsrc and home plfsrc
	fpetc = fopen("/etc/plfsrc", "r");
	fphome = fopen(filename, "r");
	
	if ((fphome == NULL) && (fpetc == NULL)) {
		fprintf(stderr, "No PLFS configs found.\n");
		return NULL;
	}

	char *line = NULL;
	size_t n = 0;
	int mntcnt = 0;

	// look for mnt pnt lines in fpetc
	if (fpetc != NULL) {
		while (getline(&line, &n , fpetc) >= 0) {
			if (strstr(line, "mount_point") != NULL) {
				mntcnt++;
			}
		}
	}
	// look in home
	if (fphome != NULL) {
		while (getline(&line, &n , fphome) >= 0) {
			if (strstr(line, "mount_point") != NULL) {
				mntcnt++;
			}
		}
	}
	
	// allocate an array to hold the mounts
	mntpnt *mounts = malloc(sizeof(mntpnt) * mntcnt);
	mntcnt = 0;
	
	if (fpetc != NULL) fseek(fpetc, SEEK_SET, 0);
	if (fphome != NULL) fseek(fphome, SEEK_SET, 0);

	char buffer[1024];


	if (fpetc != NULL) {
		while (getline(&line, &n, fpetc) >= 0) {
			if (strstr(line, "mount_point") != NULL) {
				sscanf(line, "mount_point%*[ \t]%s", buffer);
				mounts[mntcnt].mnt = malloc((strlen(buffer) + 1) * sizeof(char));
	
				strcpy(mounts[mntcnt].mnt, buffer);

				// allocate memory for 1 backend
				mounts[mntcnt].cnt = 0;
				mounts[mntcnt].bk = malloc(1 * sizeof(char *));
			}
			if (strstr(line, "backends") != NULL) {
				sscanf(line, "backends%*[ \t]%s", buffer);

				char *bkend = strtok(buffer, ",");
				while (bkend != NULL) {
					mounts[mntcnt].bk = realloc(mounts[mntcnt].bk, (mounts[mntcnt].cnt + 1) * sizeof(char *));
					mounts[mntcnt].bk[mounts[mntcnt].cnt] = malloc((strlen(bkend) + 1) * sizeof(char));
					strcpy(mounts[mntcnt].bk[mounts[mntcnt].cnt++], bkend);
					bkend = strtok(NULL, ",");
				}

				mntcnt++;
			}
		}
	}
	if (fphome != NULL) {
		while (getline(&line, &n, fphome) >= 0) {
			if (strstr(line, "mount_point") != NULL) {
				sscanf(line, "mount_point%*[ \t]%s", buffer);
				mounts[mntcnt].mnt = malloc((strlen(buffer) + 1) * sizeof(char));
	
				strcpy(mounts[mntcnt].mnt, buffer);
				
				// allocate memory for 1 backend
				mounts[mntcnt].cnt = 0;
				mounts[mntcnt].bk = malloc(1 * sizeof(char *));
			}
			if (strstr(line, "backends") != NULL) {
				sscanf(line, "backends%*[ \t]%s", buffer);
				
				char *bkend = strtok(buffer, ",");
				while (bkend != NULL) {
					mounts[mntcnt].bk = realloc(mounts[mntcnt].bk, (mounts[mntcnt].cnt + 1) * sizeof(char *));
					mounts[mntcnt].bk[mounts[mntcnt].cnt] = malloc((strlen(bkend) + 1) * sizeof(char));
					strcpy(mounts[mntcnt].bk[mounts[mntcnt].cnt++], bkend);
					bkend = strtok(NULL, ",");
				}

				mntcnt++;
			}
		}
	}
	if (fpetc != NULL) fclose(fpetc);
	if (fphome != NULL) fclose(fphome);

	if (line) free(line);

	*cnt = mntcnt;
	return mounts;
}

int readmounts = 0;
mntpnt *mounts = NULL;
int mcnt;

char *getPath(const char *infile) {
	if (readmounts == 0) {
		mounts = getMounts(&mcnt);
		readmounts = 1;
	}

	char buf1[1024];
	char buf2[1024];
	if (infile == NULL) return NULL;

	char *out = malloc((strlen(infile) + 1) * sizeof(char));

	strcpy(out, infile);
	
	if (mounts == NULL) return out;

	int isplfs = 0;
	int i, j;
	for (i = 0; i < mcnt; i++) {
		for (j = 0; j < mounts[i].cnt; j++) {
			if (strstr(infile, mounts[i].bk[j]) != NULL) {
				char *patt = malloc(sizeof(char) * (strlen(mounts[i].bk[j]) + 11));
				
				sprintf(patt, "%s%%*[/]%%[^/]", mounts[i].bk[j]);
				sscanf(infile, patt, buf1);
				
				sprintf(buf2, "plfs:%s/%s", mounts[i].mnt, buf1);

				isplfs = 1;
			}
		}
	}
	if (isplfs) {
		void *tmp = realloc(out, (strlen(buf2) + 1) * sizeof(char));
		if (tmp != NULL) out = tmp;
		else return NULL;

		strcpy(out, buf2);
	}

	return out;
}

