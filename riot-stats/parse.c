#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "riot-stats.h"
#include "plfs.h"


extern opts options;

mpistat_s parseBlock(int nodeid, char *block) {
	mpistat_s mpiStat;

	mpiStat.nodeid = nodeid;

	/* Read stats */
	mpiStat.readFile = (stats) {0, 0.0, 0};
	mpiStat.readOther = (stats) {0, 0.0, 0};

	/* Write stats */
	mpiStat.writeFile = (stats) {0, 0.0, 0};
	mpiStat.writeOther = (stats) {0, 0.0, 0};

	/* Lock stats */
	mpiStat.locks = 0;
	mpiStat.lockT = 0.0;
	mpiStat.unlockT = 0.0;

	char *line = strtok(block, "\n");
	
	mpiStat.mpifilename = malloc(sizeof(char) * strlen(line));
	
	while (line != NULL) {
		char *op = malloc(10 * sizeof(char));

		char *filename = malloc(strlen(line) * sizeof(char));
		
		int ret;
		double stime, etime;

		int s;

		if (line[0] == '#') {
			
			if (strstr(line, "## _BEGIN_ ") != NULL) {
				if (strstr(line, "## _BEGIN_ MPI_File_write") != NULL) { 
					s = sscanf(line, "## _BEGIN_ %*[^(](%[^)]), %lf", mpiStat.mpifilename, &mpiStat.mpitime);
					mpiStat.mpiop = "write";
				} else if (strstr(line, "## _BEGIN_ MPI_File_read") != NULL) {
					s = sscanf(line, "## _BEGIN_ %*[^(](%[^)]), %lf", mpiStat.mpifilename, &mpiStat.mpitime);
					mpiStat.mpiop = "read";
				} else {
					mpiStat.mpiop = "DONTCARE";
					// Return as we don't yet care about mpi opens closes etc.
					return mpiStat;
				}

				// This should get just the last part of the filename...
				char *newstr;
				if ((newstr = strrchr(mpiStat.mpifilename, '/')) != NULL) mpiStat.mpifilename = ++newstr;
			} else {
				double endtime;
				s = sscanf(line, "## _END_ %*[^(](%[^)]), %lf", filename, &endtime);
				mpiStat.mpitime = endtime - mpiStat.mpitime;
				
				return mpiStat;
				// output.
			}
		} else {
			s = sscanf(line, "%[^,], %[^,], %ld, %lf, %lf", op, filename, &ret, &stime, &etime);
			
			if (options.plfs) filename = getPath(filename);
			
			char *newstr;
			// just last bit?
			if ((newstr = strrchr(filename, '/')) != NULL) filename = ++newstr;

			if (strstr(op, "read") != NULL) {
				if (strcmp(filename, mpiStat.mpifilename) == 0) {
					// same file.
					mpiStat.readFile.calls++;
					mpiStat.readFile.time += (etime - stime);
					mpiStat.readFile.count += ret;
				} else {
					mpiStat.readOther.calls++;
					mpiStat.readOther.time += (etime - stime);
					mpiStat.readOther.count += ret;
				}
			} else if (strstr(op, "write") != NULL) {
				if (strcmp(filename, mpiStat.mpifilename) == 0) {
					// same file.
					mpiStat.writeFile.calls++;
					mpiStat.writeFile.time += (etime - stime);
					mpiStat.writeFile.count += ret;
				} else {
					mpiStat.writeOther.calls++;
					mpiStat.writeOther.time += (etime - stime);
					mpiStat.writeOther.count += ret;
				}
			} else if (strstr(op, "unlock") != NULL) {
				mpiStat.unlockT += (etime - stime);
			} else if (strstr(op, "lock") != NULL) {
				mpiStat.locks++;
				mpiStat.lockT += (etime - stime);
			} // dont care about others just yet
		}

		line = strtok(NULL, "\n");
	}
}

mpistat parseFile(char *filename, int nodeid) {
	/* Open file for parsing */
	mpistat mpiStats;

	mpiStats.mpiStat = malloc(sizeof(mpistat_s) * 1);
	mpiStats.count = 0;

	FILE *fp = fopen(filename, "r");

	if (fp == NULL) {
		printf("Error\n");
		return mpiStats;
	}

	char *line = NULL;
	size_t n = 0;

	char *block;

	while (getline(&line, &n , fp) >= 0) {
		if (line[0] == '#') {
			if (strspn(line, "## _BEGIN_ ") == 11) {
				block = malloc(sizeof(char) * (strlen(line) + 1));
				block[0] = '\0';
				strcat(block, line);
			} else {
				void *tmp = realloc(block, sizeof(char) * (strlen(block) + strlen(line) + 1));
				if (tmp == NULL) {
					mpiStats.count = 0;
					return mpiStats;
				}
				block = tmp;
				strcat(block, line);

				void *tmp2 = realloc(mpiStats.mpiStat, sizeof(mpistat_s) * (mpiStats.count + 1));
				if (tmp2 == NULL) {
					mpiStats.count = 0;
					return mpiStats;
				}
				mpiStats.mpiStat = tmp2;
				
				mpiStats.mpiStat[mpiStats.count] = parseBlock(nodeid, block);

				if (options.mpistats) {
					if (strcmp(mpiStats.mpiStat[mpiStats.count].mpiop, "DONTCARE") != 0) 
						printMPIStat(mpiStats.mpiStat[mpiStats.count]);
				}
				
				mpiStats.count++;
				free(block);
			}
		} else {
			void *tmp = realloc(block, sizeof(char) * (strlen(block) + strlen(line) + 1));
			if (tmp == NULL) {
				mpiStats.count = 0;
				return mpiStats;
			}
			block = tmp;

			strcat(block, line);
		}
	}

	if (line) free(line);

	fclose(fp);
	return mpiStats;
}
