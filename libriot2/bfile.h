#include <stdio.h>

typedef struct {
	char *buffer;
	int size;
	int pos;
	FILE *out;
} bfile;

bfile *bopen(char *path, int size);
void bflush(bfile *br);
void bclose(bfile *br);
void bwrite(bfile *br, char *string);

