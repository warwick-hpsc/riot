double zerotime;
int initialised;

int mpitrace;

int (*_open)(const char *fn, int oflags, ...);
int (*_open64)(const char *fn, int oflags, ...);
int (*_close)(int filedes);
ssize_t (*_read)(int filedes, void *buf, size_t nbyte);
ssize_t (*_pread)(int filedes, void *buf, size_t nbyte, off_t offset);
ssize_t (*_pread64)(int filedes, void *buf, size_t nbyte, off64_t offset);
ssize_t (*_write)(int filedes, const void *buf, size_t nbyte);
ssize_t (*_pwrite)(int filedes, const void *buf, size_t nbyte, off_t offset);
ssize_t (*_pwrite64)(int filedes, const void *buf, size_t nbyte, off64_t offset);
int (*_lockf)(int filedes, int function, off_t size);
int (*_fsync)(int filedes);
int (*_fcntl)(int filedes, int cmd, ...);
off_t (*_lseek)(int filedes, off_t offset, int whence);

//void *(*_mmap)(void *addr, size_t len, int prot, int flags, int fildes, off_t off);

void setZero(double t);

void add(char *data);

void writeOut();
