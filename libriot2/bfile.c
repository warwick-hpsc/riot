#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "bfile.h"

bfile *bopen(char *path, int size) {
	bfile *br = malloc(sizeof(bfile));
	br->out = fopen(path, "w");
	if (br->out == NULL) printf("I've failed to open %s! %d\n", path, errno);
	br->size = size;
	br->pos = 0;
	br->buffer = malloc(sizeof(char) * (size+1));
	*(br->buffer) = '\0';

	return br;
}

void bflush(bfile *br) {
	fputs(br->buffer, br->out);

	*br->buffer = '\0';
	br->pos = 0;
}

void bclose(bfile *br) {
	bflush(br);
	fclose(br->out);
	free(br->buffer);
	free(br);
}

void bwrite(bfile *br, char *string) {
	if (strlen(string) > br->size) {
		bflush(br);
		fputs(string, br->out);
	} else {
		if ((strlen(string) + br->pos) > br->size) {
			bflush(br);
		}

		strcat(br->buffer, string);
		br->pos += strlen(string);
	}
}

