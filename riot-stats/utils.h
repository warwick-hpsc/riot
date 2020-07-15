void printUsage(char *arg0);

void printInfo();

void getFiles(char *dir, char ***filelist, int *filecount);

int getNodeID(char *filename);

opts parseArgs(int argc, char *argv[]);