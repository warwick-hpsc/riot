#define MPIWRITE 1
#define WRITEFILE 2
#define WRITEOTHER 4
#define MPIREAD 8
#define READFILE 16
#define READOTHER 32
#define LOCK 64 

typedef struct {
	long calls;
	double time;
	long count;
} stats;

typedef struct {
	int nodeid;
	char *mpifilename;
	char *mpiop;
	double mpitime;
	stats readFile;
	stats readOther;
	stats writeFile;
	stats writeOther;
	long locks;
	double lockT;
	double unlockT;
} mpistat_s;

typedef struct {
	mpistat_s *mpiStat;
	int count;
} mpistat;

typedef struct {
	stats writeFile;
	stats writeOther;
	stats readFile;
	stats readOther;
	long mpiwritec;
	double mpiwritet;
	long mpireadc;
	double mpireadt;
	long locks;
	double lockt;
	double unlockt;
} nodestats;

typedef struct {
	char *in;
	char nodestats;
	unsigned int fields;
	char summary;
	char csv;
	char plfs;
	char mpistats;
} opts;

opts options;
