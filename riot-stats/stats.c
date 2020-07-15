#include <stdio.h>

#include "riot-stats.h"

extern opts options;

void printMPIHeader() {
	printf("nodeid, filename, mpiop, mpitime, readcalls, readbytes, readtime, readothercalls, readotherbytes, readothertime, writecalls, writebytes, writetime, writeothercalls, writeotherbytes, writeothertime, locks, locktime, unlocktime\n");
}

void printMPIStat(mpistat_s mpiStat) {
	printf("%d, %s, %s, %9.8f, %d, %d, %9.8f, %d, %d, %9.8f, %d, %d, %9.8f, %d, %d, %9.8f, %d, %9.8f, %9.8f\n",
			mpiStat.nodeid, mpiStat.mpifilename, mpiStat.mpiop, mpiStat.mpitime,
			mpiStat.readFile.calls, mpiStat.readFile.count, mpiStat.readFile.time,
			mpiStat.readOther.calls, mpiStat.readOther.count, mpiStat.readOther.time,
			mpiStat.writeFile.calls, mpiStat.writeFile.count, mpiStat.writeFile.time,
			mpiStat.writeOther.calls, mpiStat.writeOther.count, mpiStat.writeOther.time,
			mpiStat.locks, mpiStat.lockT, mpiStat.unlockT);
}

nodestats initNodeStats() {
	nodestats ns;
	ns.writeFile = (stats) { 0, 0.0, 0 };
	ns.writeOther = (stats) { 0, 0.0, 0 };
	ns.readFile = (stats) { 0, 0.0, 0 };
	ns.readOther = (stats) { 0, 0.0, 0 };
	ns.mpiwritet = 0.0;
	ns.mpiwritec = 0;
	ns.mpireadt = 0.0;
	ns.mpireadc = 0.0;
	ns.locks = 0;
	ns.lockt = 0.0;
	ns.unlockt = 0.0;

	return ns;
}

nodestats makeNodeStats(mpistat mpiStats) {
	nodestats ns = initNodeStats();
	int i;
	for (i = 0 ; i < mpiStats.count; i++) {
		ns.writeFile.calls += mpiStats.mpiStat[i].writeFile.calls;
		ns.writeFile.count += mpiStats.mpiStat[i].writeFile.count;
		ns.writeFile.time += mpiStats.mpiStat[i].writeFile.time;

		ns.writeOther.calls += mpiStats.mpiStat[i].writeOther.calls;
		ns.writeOther.count += mpiStats.mpiStat[i].writeOther.count;
		ns.writeOther.time += mpiStats.mpiStat[i].writeOther.time;

		ns.readFile.calls += mpiStats.mpiStat[i].readFile.calls;
		ns.readFile.count += mpiStats.mpiStat[i].readFile.count;
		ns.readFile.time += mpiStats.mpiStat[i].readFile.time;

		ns.readOther.calls += mpiStats.mpiStat[i].readOther.calls;
		ns.readOther.count += mpiStats.mpiStat[i].readOther.count;
		ns.readOther.time += mpiStats.mpiStat[i].readOther.time;

		if (strcmp(mpiStats.mpiStat[i].mpiop, "write") == 0) {
			ns.mpiwritet += mpiStats.mpiStat[i].mpitime;
			ns.mpiwritec++;
		} else if (strcmp(mpiStats.mpiStat[i].mpiop, "read") == 0) {
			ns.mpireadt += mpiStats.mpiStat[i].mpitime;
			ns.mpireadc++;
		} else {
			// ignore?
		}

		ns.locks += mpiStats.mpiStat[i].locks;
		ns.lockt += mpiStats.mpiStat[i].lockT;
		ns.unlockt += mpiStats.mpiStat[i].unlockT;
	}

	return ns;
}

void printNodeHeader() {
	printf("Node");

	char sep = '\t';
	if (options.csv) sep = ',';

	if ((options.fields & WRITEFILE) == WRITEFILE)
		printf("%1$cW2F_Calls%1$cW2F_Bytes%1$cW2F_Time%1$cW2F_MB/s", sep);

	if ((options.fields & WRITEOTHER) == WRITEOTHER)
		printf("%1$cW2O_Calls%1$cW2O_Bytes%1$cW2O_Time%1$cW2O_MB/s", sep);

	if ((options.fields & MPIWRITE) == MPIWRITE)
		printf("%1$cMPIW_Calls%1$cMPIW_Time%1$cMPIW_MB/s", sep);

	if ((options.fields & READFILE) == READFILE)
		printf("%1$cR2F_Calls%1$cR2F_Bytes%1$cR2F_Time%1$cR2F_MB/s", sep);

	if ((options.fields & READOTHER) == READOTHER)
		printf("%1$cR2O_Calls%1$cR2O_Bytes%1$cR2O_Time%1$cR2O_MB/s", sep);

	if ((options.fields & MPIREAD) == MPIREAD)
		printf("%1$cMPIR_Calls%1$cMPIR_Time%1$cMPIR_MB/s", sep);

	if ((options.fields & LOCK) == LOCK)
		printf("%1$cLocks%1$cLockTime%1$cUnlockTime", sep);

	printf("\n");
}

void printNodeStats(int nodeid, nodestats ns) {
	printf("%d", nodeid);

	char sep = '\t';
	if (options.csv) sep = ',';

	if ((options.fields & WRITEFILE) == WRITEFILE)
		printf("%1$c%2$ld%1$c%3$ld%1$c%4$lf%1$c%5$lf", sep, ns.writeFile.calls, ns.writeFile.count, ns.writeFile.time, 
				ns.writeFile.time != 0.0 ? (ns.writeFile.count / ns.writeFile.time) / (1024*1024) : 0.0);

	if ((options.fields & WRITEOTHER) == WRITEOTHER) 
		printf("%1$c%2$ld%1$c%3$ld%1$c%4$lf%1$c%5$lf", sep, ns.writeOther.calls, ns.writeOther.count, ns.writeOther.time, 
				ns.writeOther.time != 0.0 ? (ns.writeOther.count / ns.writeOther.time) / (1024*1024) : 0.0);

	if ((options.fields & MPIWRITE) == MPIWRITE) 
		printf("%1$c%2$ld%1$c%3$lf%1$c%4$lf", sep, ns.mpiwritec, ns.mpiwritet, 
				ns.mpiwritet != 0.0 ? (ns.writeFile.count / ns.mpiwritet) / (1024*1024) : 0.0);

	if ((options.fields & READFILE) == READFILE)
		printf("%1$c%2$ld%1$c%3$ld%1$c%4$lf%1$c%5$lf", sep, ns.readFile.calls, ns.readFile.count, ns.readFile.time, 
				ns.readFile.time != 0.0 ? (ns.readFile.count / ns.readFile.time) / (1024*1024) : 0.0);

	if ((options.fields & READOTHER) == READOTHER) 
		printf("%1$c%2$ld%1$c%3$ld%1$c%4$lf%1$c%5$lf", sep, ns.readOther.calls, ns.readOther.count, ns.readOther.time, 
				ns.readOther.time != 0.0 ? (ns.readOther.count / ns.readOther.time) / (1024*1024) : 0.0);

	if ((options.fields & MPIREAD) == MPIREAD)
		printf("%1$c%2$d%1$c%3$lf%1$c%4$lf", sep, ns.mpireadc, ns.mpireadt, 
				ns.mpireadt != 0.0 ? (ns.readFile.count / ns.mpireadt) / (1024*1024) : 0.0);

	if ((options.fields & LOCK) == LOCK) 
		printf("%1$c%2$d%1$c%3$lf%1$c%4$lf", sep, ns.locks, ns.lockt, ns.unlockt);	

	printf("\n");	
}

void printSummary(nodestats *ns, int count) {
	double sumwbytes = 0;
	double sumwtime = 0.0;
	double sumrbytes = 0;
	double sumrtime = 0.0;

	double mpiwtime = 0.0;
	double mpiwband = 0.0;
	double mpirtime = 0.0;
	double mpirband = 0.0;

	double sumowtime = 0.0;
	double sumortime = 0.0;

	long sumlocks = 0;
	double sumlockt = 0.0;
	double sumunlockt = 0.0;


	double maxposixw = 0.0;
	int maxposixwrank = -1;
	double minposixw = 0.0;
	int minposixwrank = -1;
	double sumposixw = 0.0;

	double maxposixr = 0.0;
	int maxposixrrank = -1;
	double minposixr = 0.0;
	int minposixrrank = -1;
	double sumposixr = 0.0;

	int i;
	for (i = 0; i < count; i++) {
		sumwbytes += ns[i].writeFile.count;
		sumwtime += ns[i].writeFile.time;
		sumowtime += ns[i].writeOther.time;

		mpiwtime += ns[i].mpiwritet;

		mpiwband += ns[i].mpiwritet != 0.0 ? ns[i].writeFile.count / ns[i].mpiwritet : 0;

		sumrbytes += ns[i].readFile.count;
		sumrtime += ns[i].readFile.time;
		sumortime += ns[i].readOther.time;

		mpirtime += ns[i].mpireadt;

		mpirband += ns[i].mpireadt != 0.0 ? ns[i].readFile.count / ns[i].mpireadt : 0;

		sumlocks += ns[i].locks;
		sumlockt += ns[i].lockt;
		sumunlockt += ns[i].unlockt;

		if ((maxposixwrank == -1) || ((ns[i].writeFile.count / ns[i].writeFile.time) > maxposixw)) {
			maxposixw = ns[i].writeFile.time != 0.0 ? ns[i].writeFile.count / ns[i].writeFile.time : 0.0;
			maxposixwrank = i;
		}

		if ((minposixwrank == -1) || ((ns[i].writeFile.count / ns[i].writeFile.time) < minposixw)) {
			minposixw = ns[i].writeFile.time != 0.0 ? ns[i].writeFile.count / ns[i].writeFile.time : 0.0;
			minposixwrank = i;
		}

		sumposixw += ns[i].writeFile.time != 0.0 ? ns[i].writeFile.count / ns[i].writeFile.time : 0.0;

		if ((maxposixrrank == -1) || ((ns[i].readFile.count / ns[i].readFile.time) > maxposixr)) {
			maxposixr = ns[i].readFile.time != 0.0 ? ns[i].readFile.count / ns[i].readFile.time : 0.0;
			maxposixrrank = i;
		}

		if ((minposixrrank == -1) || ((ns[i].readFile.count / ns[i].readFile.time) < minposixr)) {
			minposixr = ns[i].readFile.time != 0.0 ? ns[i].readFile.count / ns[i].readFile.time : 0.0;
			minposixrrank = i;
		}

		sumposixr += ns[i].readFile.time != 0.0 ? ns[i].readFile.count / ns[i].readFile.time : 0.0;
	}

	printf("============================================\n");
	printf("|        RIOT Performance Summary          |\n");
	printf("============================================\n\n");

	printf("POSIX I/O Summary\n");
	printf("=================\n\n");

	printf("Total Data Written (MB):                  %lf\n", sumwbytes / (1024*1024));
	printf("Sum Total POSIX time writing (s):         %lf\n", sumwtime);
	printf("Effective POSIX bandwidth (MB/s):         %lf\n\n", sumwtime != 0.0 ? (sumwbytes / sumwtime) / (1024*1024) : 0.0);

	printf("Average POSIX bandwidth (MB/s per core):  %lf\n", count != 0 ? (sumposixw / count) / (1024*1024) : 0.0);
	printf("Max POSIX bandwidth [rank] (MB/s):        %lf\t[%d]\n", maxposixw / (1024*1024), maxposixwrank);
	printf("Min POSIX bandwidth [rank] (MB/s):        %lf\t[%d]\n\n", minposixw / (1024*1024), minposixwrank);

	printf("POSIX write time to other files (s):      %lf\n\n", sumowtime);

	printf("Total Data Read (MB):                     %lf\n", sumrbytes/ (1024*1024));
	printf("Sum Total POSIX time reading (s):         %lf\n", sumrtime);
	printf("Effective POSIX bandwidth (MB/s):         %lf\n\n", sumrtime != 0.0 ? (sumrbytes / sumrtime) / (1024*1024) : 0.0);
	
	printf("Average POSIX bandwidth (MB/s per core):  %lf\n", count != 0 ? (sumposixr / count) / (1024*1024) : 0.0);
	printf("Max POSIX bandwidth [rank] (MB/s):        %lf\t[%d]\n", maxposixr / (1024*1024), maxposixrrank);
	printf("Min POSIX bandwidth [rank] (MB/s):        %lf\t[%d]\n\n", minposixr / (1024*1024), minposixrrank);

	printf("POSIX read time from other files (s):     %lf\n\n", sumortime);

	printf("Total Locks requested:                    %ld\n", sumlocks);
	printf("Lock time (s):                            %lf\n", sumlockt);
	printf("Unlock time (s):                          %lf\n\n", sumunlockt);

	printf("MPI I/O Summary\n");
	printf("===============\n\n");

	printf("Total MPI time writing (s):              %lf\n", mpiwtime);
	printf("Effective MPI Bandwidth (MB/s):          %lf\n", mpiwtime != 0.0 ? (sumwbytes / mpiwtime) / (1024*1024) : 0.0);
	printf("Percieved MPI Bandwidth (MB/s):          %lf\n\n", mpiwband / (1024*1024));

	printf("Total MPI time reading (s):              %lf\n", mpirtime);
	printf("Effective MPI Bandwidth (MB/s):          %lf\n", mpirtime != 0.0 ? (sumrbytes / mpirtime) / (1024*1024) : 0.0);
	printf("Percieved MPI Bandwidth (MB/s):          %lf\n", mpirband / (1024*1024));
}
