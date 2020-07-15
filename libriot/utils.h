#include <fcntl.h>

void initLookup();

char *getFilename(int filedes);

char *getMPIFilename(void *mpih);

void setMPIH(char *filename, void *mpih);


char *generateFilename();

char *checkOpenFlags(int flags);

char *checkFcntlFlags(int flags);

char *decodeModet(mode_t mode);

char *decodeFlock(struct flock *filelock);

double getTime();

