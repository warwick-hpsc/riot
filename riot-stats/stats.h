
void printMPIHeader() ;

void printMPIStat(mpistat_s mpiStat);

nodestats initNodeStats();

nodestats makeNodeStats(mpistat mpiStats);

void printNodeHeader();

void printNodeStats(int nodeid, nodestats ns);

void printSummary(nodestats *ns, int count);