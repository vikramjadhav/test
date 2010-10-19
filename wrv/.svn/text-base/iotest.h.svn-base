/*******************************************************************************

Filename: iotest.h

Description: iotest header file.

Copyright (c) 1998-1999 Omneon Video Networks (TM)

OMNEON VIDEO NETWORKS CONFIDENTIAL

 ******************************************************************************/
#ifndef  _io_test_h
#define  _io_test_h

#define    _FILE_OFFSET_BITS    64
#define    _THREAD_SAFE
#define    _LARGEFILE_SOURCE

#if defined(linux)
    #include "linux/linuxinclude.h"
#elif defined(Windows)
    #include "windows/wininclude.h"
#elif defined(macosx)
    #include "macosx/macosxinclude.h"
#endif

#define        FAILURE          1 /* to indicate failure of an operation */
#define        SUCCESS          0 /* to indicate success of an operation */

#define        TRUE             1
#define        FALSE            0

#define        NO_TIMEOUT       -1 /* to indicate no timeout value is given */
#define        NO_PROGRESS      -1 /* no progress stats are needed*/
/* Data Type Declarations */
#if !defined(Windows)
typedef int64_t         int64;
typedef int32_t         int32;
typedef int16_t         int16;
typedef uint16_t        uint16;
typedef uint32_t        uint32;
typedef uint64_t        uint64;
#endif
struct omMgIoStat {
    union start {
        uint64    index;
        uint64    offset;
        uint64    baseOffset;
    } start;
    uint64      size;
    uint64      count;
    int         inpFileDesc;
    int         outFileDesc;
    int64       readStartTime;
    int64       readEndTime;
    int64       writeStartTime;
    int64       writeEndTime;
    uint64      readDoneCount;
    uint64      writeDoneCount;
};

/* File operations structure */
struct omFileOperations {
    int (*openInput)            (int *fd);
    int (*openOutput)           (int *fd);
    int (*readDirectSeq)        (int fd, struct omMgIoStat *iostat);
    int (*readMmapSeq)          (int fd, struct omMgIoStat *iostat);
    int (*readBufferedSeq)      (int fd, struct omMgIoStat *iostat);
    int (*writeDirectSeq)       (int fd, struct omMgIoStat *iostat);
    int (*writeMmapSeq)         (int fd, struct omMgIoStat *iostat);
    int (*writeBufferedSeq)     (int fd, struct omMgIoStat *iostat);
    int (*readDirectRandom)     (int fd, struct omMgIoStat *iostat);
    int (*readBufferedRandom)   (int fd, struct omMgIoStat *iostat);
    int (*readMmapRandom)       (int fd, struct omMgIoStat *iostat);
    int (*writeBufferedRandom)  (int fd, struct omMgIoStat *iostat);
    int (*writeMmapRandom)      (int fd, struct omMgIoStat *iostat);
    int (*writeDirectRandom)    (int fd, struct omMgIoStat *iostat);
};


#if defined(linux) || defined(Windows) || defined (macosx)
static const char* const \
        g_short_options = "hvo:i:w:r:y:x:q:t:P:F:f:k:T:R:hu:p:Vs:S:H:n:Z:";

static const struct option g_long_options[] = {
    { "help",       0,    NULL,    'h'},    /* display help */
    { "verbose",    0,    NULL,    'v' },   /* display verbose */
    { "output",     1,    NULL,    'o'},    /* output file */
    { "input",      1,    NULL,    'i'},    /* input file */
    { "verify",     0,    NULL,    'V'},    /* enable verify */
    { "write",      2,    NULL,    'w'},    /* specify write arguments */
    { "read",       2,    NULL,    'r'},    /* specify read arguments */
    { "wthreads",   1,    NULL,    'y'},    /* number of write threads */
    { "rthreads",   1,    NULL,    'x'},    /* number of read threads */
    { "sequence",   1,    NULL,    'q'},    /* random|non-random */
    { "type",       1,    NULL,    't'},    /* sparse|non-sparse */
    { "Pattern",    1,    NULL,    'F'},    /* pattern in file */
    { "pattern",    1,    NULL,    'P'},    /* pattern in "string" */
    { "rflags",     1,    NULL,    'f'},    /* mmap|direct|buffered */
    { "wflags",     1,    NULL,    'k'},    /* mmap|direct|buffered */
    { "timeout",    1,    NULL,    'T'},    /* seconds[.milliseconds]*/
    { "pinterval",  1,    NULL,    'Z'},    /* minutes */
    { "rf",         1,    NULL,    'R'},    /* replication factor*/
    { "host",       0,    NULL,    'H'},    /* host name */
    { "username",   1,    NULL,    'u'},    /* username */
    { "password",   1,    NULL,    'p'},    /* password */
    { "sparseness", 1,    NULL,    's'},    /* sparseness */
    { "seed",       1,    NULL,    'S'},    /* seed */
    { "nice",       1,    NULL,    'n'},    /* nice value */
    { NULL,         0,    NULL,    0}       /* Required at end of array. */
};
#endif

int     omMgValidateFilename(char *path);
void    omMgInitialiseDefaultContext(void);
void    omMgReportResults();
void    omMgDisplayContext();
void    createGlobalArray(uint64);
void    omMgNiceness();
void    omMgExecuteTc();
int64   omMgTimeElapsed();
void    omMgDefaultPattern();

int     omMgIoMmapSeqWrite(int fd, struct omMgIoStat *iostat);
int     omMgIoMmapRandomWrite(int fd, struct omMgIoStat *iostat);
int     omMgIoMmapSeqRead(int fd, struct omMgIoStat *iostat);
int     omMgIoMmapRandomRead(int fd, struct omMgIoStat *iostat);
int     omMgIoBufferedDirectSeqRead(int fd, struct omMgIoStat *iostat);
int     omMgIoBufferedDirectRandomRead(int fd, struct omMgIoStat *iostat);
int     omMgIoBufferedDirectSeqWrite(int fd, struct omMgIoStat *iostat);
int     omMgIoBufferedDirectRandomWrite(int fd, struct omMgIoStat *iostat);

int     omMgIoVerifyBlock(uint64 off, char *buffer, uint64 iosize);

int     omMgIoOpenInputFile(int *);
int     omMgIoOpenOutputFile(int *);

int     omMgIoMmapRead(int fd, struct omMgIoStat *iostat);
int     omMgIoMmapWrite(int fd, struct omMgIoStat *iostat);

int     omMgIoBufferedRead();
int     omMgIoBufferedWrite();

int     omMgIoDirectRead();
int     omMgIoDirectWrite();

char*   omMgIoCommonAllocateBuffer(int size);
char*   omMgIoFillPattern(uint64 ioSize, uint32 patternLe);
char*   omMIoAllocReadBuffer(uint64, uint32);

int     omMgIoCommonRead(int fd, uint64 offset, uint64  size, char *buffer);
int     omMgIoCommonWrite(int fd, uint64 offset, uint64 size, char *pattern);
int     omMgIoGetSectorSize(char *);


int     omMgCommonAddTimeout();
int     omMgCommonAddPinterval();

void    debug(const char *format, ...);
void    error(const char *format, ...);


char    *g_pattern;                 /* pattern to be written in file */
uint32  g_patternLen;               /* pattern length */
char    *g_patternBuffer;           /* pattern buffer of block size*/
int     g_verbose_level;            /* level of verbosity */
int     g_sectorSize;
bool    g_randomIosize;
bool    g_randomSleep;

typedef enum omIoMethod {
    IO_MMAP,
    IO_DIRECT,
    IO_BUFFERED,
} omIoMethod;

typedef enum omIoType {
    IO_SPARSE,
    IO_NOSPARSE,
} omIoType;

typedef enum omIoSequence {
    IO_SEQUENCE,
    IO_RANDOM,
} omIoSequence;

/* Global Arguments */
typedef struct omRwargs {
    uint64      offset;             /* offset to start io from */
    int64       size;               /* size of total io */
    uint64      count;              /* no of iops */
    omIoMethod  rwflag;             /* direct|mmap|buffered */
    uint64      blockSize;          /* io block size */
    uint64      minBlockSize;       /* lower limit for io block size */
    uint64      maxBlockSize;       /* upper limit for io block size */
    int64       sleep;              /* sleep between io ops */
    int64       minSleep;           /* lower limit for sleep between io ops */
    int64       maxSleep;           /* Upper limit for sleep between io ops */
} omRwargs;

typedef struct    domainAuthenticationToken {
    char *host;
    char *username;
    char *password;
    char *domain;
} auth_token;

typedef enum omOperation {
    OP_READ  = 0x01,
    OP_WRITE = 0x02,
} omOperation;

#define        MAX_THREADS    16

struct omExecutionContext {
    char            inputFile [256];
    char            outputFile[256];
    char            corruptionFile[256];
    uint16          verify;
    omRwargs        rarguments;
    omRwargs        warguments;
    uint16          numWriteThreads;
    uint16          numReadThreads;
    omIoSequence    sequence;
    omIoType        type;
    double          timeout;
    double          pinterval;
    int16           verbose;
    uint16          replicationFactor;
    auth_token      token;
    int16           opcode;
    int16           sparseFactor;
    uint64          seed;
    int32           nice;
};
extern struct omExecutionContext omContext;


extern struct omMgIoStat omMgIoStats[MAX_THREADS];

struct omMgIoPerf {
    int64       readStartTime;
    int64       readEndTime;
    int64       writeStartTime;
    int64       writeEndTime;
    double      readSpeed;
    double      writeSpeed;
    double      readTotalTime;
    double      writeTotalTime;
    uint64      readOps;
    uint64      writeOps;
    uint64      verifyOps;
    uint64      failedVerifyOps;
}omMgIoPerf;

extern struct omMgIoPerf omMgIoPerf;

#define BYTES_IN_KB    (1024)
#define BYTES_IN_MB    (1024 * 1024)

#endif /* _io_test_h */
