#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "riot-stats.h"

void printUsage(char *arg0) {
	fprintf(stderr, "Usage: %s [OPTION] ...\n", arg0);
	fprintf(stderr, "Generate statistics based on log outputs.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  -in FILE               Read logs in directory FILE\n");
	fprintf(stderr, "  -nodestats FLAGS       Output node statistics according to FLAGS\n");
	fprintf(stderr, "  -summary               Output a summary of I/O statistics\n");
	fprintf(stderr, "  -csv                   Output in comma seperated variable format\n");
	fprintf(stderr, "  -mpistats              Output individual MPI stats in csv (unordered)\n");
	fprintf(stderr, "  -info                  Display output meanings\n");
	fprintf(stderr, "  -plfs                  Perform PLFS path translation\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "FLAGS is a comma seperated list with the following available options:\n");
	fprintf(stderr, "  mw                     MPI_File_write_* statistics\n");
	fprintf(stderr, "  wf                     Write-to-file statistics\n");
	fprintf(stderr, "  wo                     Write-to-other statistics\n");
	fprintf(stderr, "  mr                     MPI_File_read_* statistics\n");
	fprintf(stderr, "  rf                     Read-from-file statistics\n");
	fprintf(stderr, "  ro                     Read-from-other statistics\n");
	fprintf(stderr, "  lk                     File lock statistics\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Examples:\n");
	fprintf(stderr, "  %s -in /path/to/logs -nodestats mw,wf,wo -summary\n", arg0);
	fprintf(stderr, "                         Print all write statistics and summary\n");
}

void printInfo() {
	fprintf(stderr, "RIOT Explaination of output terms.\n\n");
	fprintf(stderr, "Effective POSIX bandwidth - The total amount of data written across all\n");
	fprintf(stderr, "            processors divided by the total time spent in serial.\n");
	fprintf(stderr, "POSIX write time to other files - MPI often writes to other files such\n");
	fprintf(stderr, "            /dev/ipath while performing MPI-I/O. These are represented\n");
	fprintf(stderr, "            here as an overhead.\n");
	fprintf(stderr, "Effective MPI bandwidth - The data amount of data written accross all\n");
	fprintf(stderr, "            processors divided by the total time spent in MPI calls.\n");
	fprintf(stderr, "            Due to MPI's blocking collectives you can often multiply\n");
	fprintf(stderr, "            this number by the number of nodes to get the percieved\n");
	fprintf(stderr, "            bandwidth (the same is not true of POSIX calls).\n");
	fprintf(stderr, "Average POSIX bandwidth - This is the sum of the individual bandwidths\n");
	fprintf(stderr, "            divided by the number of processors. It cannot be assumed\n");
	fprintf(stderr, "            that the percieved bandwidth will be the average multiplied\n");
	fprintf(stderr, "            by the number of processors since perfect parallelism\n");
	fprintf(stderr, "            cannot be assumed.\n");
}

void getFiles(char *dir, char ***filelist, int *filecount) {
	DIR *directory = opendir(dir);

	if (directory == NULL) {
		*filelist = NULL;
		*filecount = 0;
		return;
	}

	struct dirent *ep;

	int cnt=0;
	char **files = malloc(sizeof(void *) * 1);

	while (ep = readdir(directory)) {
		if (ep->d_name[0] != '.') {
			char *buff = malloc(sizeof(char) * (strlen(ep->d_name) + strlen(dir) + 2));
			sprintf(buff, "%s/%s", dir, ep->d_name);
			
			struct stat stats;
			int ret = stat(buff, &stats);
			if (ret != 0) continue;

			if (S_ISREG(stats.st_mode)) {
				files[cnt] = malloc(sizeof(char) * (strlen(buff) + 1));
				strcpy(files[cnt++], buff); 
				void *t = realloc(files, sizeof(void *) * (cnt+1));
				if (t != NULL) files = t;
				else return;
			}
			free(buff);
		}
	}
	*filecount = cnt;
	*filelist = files;
}

int getNodeID(char *filename) {
	char *nodeidstr = malloc(sizeof(char) * 10);
	nodeidstr = strrchr(filename, '-');
	char *idstr = malloc(sizeof(char) * (strlen(nodeidstr) - 4));
	sscanf(nodeidstr, "-%[0123456789].log", idstr);
	return atoi(idstr);
}

opts parseArgs(int argc, char *argv[]) {
	opts ops = { NULL, 0, 0, 0, 0, 0 };
	int i;
	if (argc == 1) {
		printUsage(argv[0]);
		exit(1);
	}
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (strcmp(argv[i], "-in") == 0) {
				if (i < (argc-1)) ops.in = argv[++i];
				else {
					fputs("No input path given.\n", stderr);
					printUsage(argv[0]);
					exit(1);
				}
			} else if (strcmp(argv[i], "-nodestats") == 0) {
				ops.nodestats = 1;
				if ((i < (argc-1)) && (argv[i+1][0] != '-')) {
					i++;
					// theres some flags
					char *curr = strtok(argv[i], ",");
					while (curr != NULL) {
						if (strcmp(curr, "mw") == 0) ops.fields |= MPIWRITE;
						else if (strcmp(curr, "wf") == 0) ops.fields |= WRITEFILE;
						else if (strcmp(curr, "wo") == 0) ops.fields |= WRITEOTHER;
						else if (strcmp(curr, "mr") == 0) ops.fields |= MPIREAD;
						else if (strcmp(curr, "rf") == 0) ops.fields |= READFILE;
						else if (strcmp(curr, "ro") == 0) ops.fields |= READOTHER;
						else if (strcmp(curr, "lk") == 0) ops.fields |= LOCK;
						else {
							fputs("Unrecognised field.\n", stderr);
							printUsage(argv[0]);
							exit(1);
						}
						curr = strtok(NULL, ",");
					}
				} else {
					ops.fields = 127;
					// set default flags
				}
			} else if (strcmp(argv[i], "-summary") == 0) {
				ops.summary = 1;
			} else if (strcmp(argv[i], "-csv") == 0) {
				ops.csv = 1;
			} else if (strcmp(argv[i], "-mpistats") == 0) {
				ops.mpistats = 1;
			} else if (strcmp(argv[i], "-plfs") == 0) {
				ops.plfs = 1;
			} else if (strcmp(argv[i], "-info") == 0) {
				printInfo();
				exit(1);
			} else {
				fputs("Unrecognised Option.\n", stderr);
				printUsage(argv[0]);
				exit(1);
			}
		} else {
			fputs("Unrecognised Option.\n", stderr);
			printUsage(argv[0]);
			exit(1);
		}
	}

	return ops;
}


