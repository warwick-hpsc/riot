int addFile(const char *filename);

int openPOSIXFile(int fh, const char *filename);

int openMPIFile(void *mpifh, const char *filename);

int getPOSIXFile(int fh);

int getMPIFile(void *mpifh);

int closePOSIXFile(int fh);

int closeMPIFile(void *mpifh);

void printFileDB(char *filename);

