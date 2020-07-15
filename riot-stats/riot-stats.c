#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "riot-stats.h"
#include "stats.h"
#include "utils.h"
#include "parse.h"

int main(int argc, char *argv[]) {
	options = parseArgs(argc, argv);

	char **files;
	int counter;

	getFiles(options.in, &files, &counter);

	mpistat *mpiStats = malloc(sizeof(mpistat) * counter);

	if (options.mpistats) printMPIHeader();

	int i;
	for (i = 0; i < counter; i++) {
		int nodeid = getNodeID(files[i]);
		mpiStats[nodeid] = parseFile(files[i], nodeid);
	}

	if (options.nodestats) printNodeHeader();

	nodestats *ns = malloc(sizeof(nodestats) * counter);

	for (i = 0; i < counter; i++) {
		ns[i] = makeNodeStats(mpiStats[i]);
		if (options.nodestats) printNodeStats(i, ns[i]);
	}

	if (options.summary) printSummary(ns, counter);
}


