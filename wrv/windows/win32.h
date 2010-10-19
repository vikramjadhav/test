/*******************************************************************************
 *
 * Filename     : win32.h 
 *
 * Description  : header file for windows compatibility layer 
 * Copyright (c) 1998-1999 Omneon Video Networks (TM)
 *
 * OMNEON VIDEO NETWORKS CONFIDENTIAL
 *
 ******************************************************************************/
#ifndef __WIN32_H__
#define __WIN32_H__

#include "wininclude.h"

#define LUSIZE 1024
#define ACCT_NAME_SIZE LUSIZE

#ifdef _UNICODE /* This declaration had to be added. */
#define _memtchr wmemchr
#else
#define _memtchr memchr
#endif

/* Macro definitaion for varieous array size require for acl */
#define LUSIZE 1024
#define ACL_SIZE 1024
#define SID_SIZE LUSIZE  /* See support.h */
#define DOM_SIZE LUSIZE

/* DWORD_PTR (pointer precision unsigned integer) is used for integers
 * that are converted to handles or pointers
 * This eliminates Win64 warnings regarding conversion between
 * 32 and 64-bit data, as HANDLEs and pointers are 64 bits in
 * Win64. This is enable only if _Wp64 is defined.
 */
#if !defined(_Wp64)
#define DWORD_PTR DWORD
#define LONG_PTR LONG
#define INT_PTR INT
#endif

/* unix mode constants */
#define S_IRWXU (0400|0200|0100)
#define S_IRUSR (0400)
#define S_IWUSR (0200)
#define S_IXUSR (0100)
#define S_IRWXG ((0400|0200|0100) >> 3)
#define S_IRGRP (0400 >> 3)
#define S_IWGRP (0200 >> 3)
#define S_IXGRP (0100 >> 3)
#define S_IRWXO (((0400|0200|0100) >> 3) >> 3)
#define S_IROTH ((0400 >> 3) >> 3)
#define S_IWOTH ((0200 >> 3) >> 3)
#define S_IXOTH ((0100 >> 3) >> 3)

/* Acl size constants */
#define LUSIZE 1024
#define ACL_SIZE 1024
#define INIT_EXCEPTION 0x3
#define CHANGE_EXCEPTION 0x4
#define SID_SIZE LUSIZE  /* See support.h */
#define DOM_SIZE LUSIZE

/* opendir, readdir, closedir */
#define NAME_MAX 1024
struct dirent {
    char    d_name[NAME_MAX+1];  /* filename (null-terminated) */
}*pDirent;
typedef struct {
    HANDLE hDir;
}DIR;
DIR *pDIR;
WIN32_FIND_DATA FindFileData; /* require for readdir */

/* types */
#define uint64_t  ULONGLONG
#define uint32_t  ULONG
#define uint16_t  USHORT
#define uint8_t   BYTE
#define int32_t   LONG
#define int64_t   LONGLONG

typedef LONGLONG        int64;
typedef LONG            int32;
typedef SHORT            int16;
typedef USHORT          uint16;
typedef ULONG           uint32;
typedef ULONGLONG       uint64; 

#define snprintf _snprintf
#define strcasecmp strcmpi
/* truncate specific */
typedef LONGLONG off_t;
/* Function Prototypes */
BOOL PrintStrings (HANDLE hOut, ...);
BOOL PrintMsg(HANDLE hOut, LPCTSTR pMsg);
VOID ReportError(LPCTSTR lpUserMessage/*, DWORD dwExitCode,
                 BOOL bPrintErrorMsg*/);
static VOID FindGroup(DWORD GroupNumber, LPTSTR GroupName,
                       DWORD cGroupName);
LPSECURITY_ATTRIBUTES InitializeUnixSA(DWORD UnixPerms,LPTSTR UsrNam,
                                       LPTSTR GrpNam, LPDWORD AllowedAceMasks,
                                       LPDWORD DeniedAceMasks, LPHANDLE pHeap);
LPSECURITY_ATTRIBUTES ConvertUnixModeToWinSecAttr(HANDLE hSecHeap, DWORD Mode);

int creat(LPTSTR lpFilePath, int Mode);
int mkdir(LPTSTR lpFilePath , DWORD Mode);
int close(int fd);
int closedir(DIR* pDIR) ;
int fsync(int fd);
int truncate(LPTSTR filePath, off_t offset);
int read(int fd, LPVOID buf, int64 size);
int write(int fd, LPVOID buf, int64 size);
int symlink(LPTSTR, LPTSTR);
int link(LPTSTR, LPTSTR);
int chdir(LPTSTR lpPathName);
int chmod(LPTSTR lpPathName, int Mode);
int link(LPTSTR orgFile, LPTSTR hlinkFile);
int S_ISREG(LPTSTR);
int S_ISDIR(LPTSTR);
int S_ISLNK(LPTSTR);
int stat(LPTSTR, struct stat*);
int lstat(LPTSTR, struct stat*);
LPTSTR getcwd(LPTSTR lpPathName, unsigned int DIRNAME_LEN);
DIR* opendir(LPTSTR filePath);
struct dirent* readdir(DIR* pDIR);
unsigned int alarm(int seconds);
int MapHandleToFD(HANDLE hFile);

/* stat system call structure */
struct stat{
    char st_mode[MAX_PATH];
    int64_t st_size;
};
#define strerror(x) strerror(x = GetLastError())
/* creat specific variables */
#define MAX_OPEN_FILES 1024
#define FD_MAX (MAX_OPEN_FILES - 1)
struct FdToHandlMap{
    HANDLE hFile;
};

int g_freeFDIndex; /* to maintain free FD index */
int g_curFDCount; /* current free fd value */
struct FdToHandlMap g_FdToHandlMap[MAX_OPEN_FILES];

/* signal */
enum {SIGALRM, SIG_ERR};
unsigned int alarm(int seconds);
typedef void (* sighandler_t)(int sigNum);
sighandler_t sigAlarmHandler;
int g_alarmInterval;
#define signal signalA
int signalA(int, sighandler_t);
#define SIGALRM 14

/* chmod */
typedef unsigned int mode_t;

/* wrv */
#define PRIu64        "llu"
#define PRIu32        "lu"
#define PRIu16        "u"
#define PRIi64        "ll"
#define PRIi32        "l"
#define bool        BOOL
#define true        TRUE
#define false        FALSE
#define O_RDONLY    OF_READ
#define O_WRONLY    OF_WRITE
#define O_RDWR        OF_READWRITE 
#define O_CREAT        OF_CREATE
#define O_TRUNC        TRUNCATE_EXISTING
#define getpid()    _getpid()

/* settimer */
#define ITIMER_REAL     0
#define ITIMER_VIRTUAL  1
#define ITIMER_PROF     2
#define ETIME            62

#define usleep(x)    Sleep((DWORD)(x / 1000))
#define atoll        _atoi64
#define atof        (double)_atoi64

/* psuedo random number generators */
#define srand48(seed) srand((unsigned int)seed)
#define lrand48()    rand()
#define drand48() (double(rand()) / RAND_MAX)

/* gettimeofday */
struct timezone {
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};

/* setitimer */
struct itimerval {
  struct timeval it_interval;    /* timer interval */
  struct timeval it_value;       /* current value */
};

/* mmap */
#define PROT_NONE   PAGE_NOACCESS     /* page can not be accessed */
#define PROT_READ   PAGE_READONLY     /* page can be read */
#define PROT_WRITE  PAGE_READWRITE    /* page can be written */
#define PROT_EXEC   PAGE_EXECUTE      /* page can be executed */
#define MAP_FAILED    ((void *)-1)  
#define MAP_SHARED    FILE_MAP_WRITE
#define LODWORD(ll)           ((DWORD)((LONGLONG)(ll) & 0xffffffff))
#define HIDWORD(ll)           ((DWORD)((LONGLONG)(ll) >> 32))
HANDLE hMapObject;    /* handle to file mapping object */
HANDLE hMemMap;     /* handle to memory map view of file */ 

/* open */
#define O_CREAT        OF_CREATE
#define O_DIRECT    FILE_FLAG_NO_BUFFERING    

/* lseek */
#ifndef SEEK_SET
    #define SEEK_SET    FILE_BEGIN
#endif

#ifndef SEEK_CUR
    #define SEEK_CUR    FILE_CURRENT
#endif

#ifndef    SEEK_END
    #define SEEK_END    FILE_END
#endif

/* pthread */
#define __SIZEOF_PTHREAD_ATTR_T 36
#define PTHREAD_MUTEX_INITIALIZER    {NULL}
#define pthread_self()    GetCurrentThreadId()
typedef CRITICAL_SECTION pthread_mutex_t;
typedef HANDLE pthread_t;
typedef HANDLE pid_t;
typedef union {
  char __size[__SIZEOF_PTHREAD_ATTR_T];
  long int __align;
} pthread_attr_t;

/* function prototypes for wrv */
char *strtok_r(char *str, char *delim, char **nextp);
off_t lseek(int fd, off_t offset, int whence);
void *mmap(void *addr, size_t length, int prot, int flags, 
           int fd, off_t offset);
int getpagesize(void);
int open(const char *pathname, ...);
int pthread_mutex_init(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int gettimeofday(struct timeval *tv, struct timezone *tz);
int munmap(void *addr, size_t length);
int pthread_join(pthread_t thread, void **value_ptr);
int pthread_create(pthread_t *thread, pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg);

/* Timeout timer */
long TimeOutInterval;
sighandler_t sigAlarmHandlerT;
int signalT(int, sighandler_t);
int SetTimeOutTimer(int which, const struct itimerval *value,
                    struct itimerval *ovalue);
DWORD WINAPI TimeOutThread(LPVOID lpArgs);

/* Progress Timer */
long ProgressInterval;
sighandler_t sigAlarmHandlerP;
int signalP(int, sighandler_t);
int SetProgressTimer(int which, const struct itimerval *value, 
                     struct itimerval *ovalue);
DWORD WINAPI ProgressThread(LPVOID lpArgs);

#endif /* __WIN32_H__ */
