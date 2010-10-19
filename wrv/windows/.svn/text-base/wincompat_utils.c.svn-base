/*******************************************************************************

Filename: wincompat_utils.c

Description: Functions for Windows compatibility layer.

Copyright (c) 1998-1999 Omneon Video Networks (TM)

OMNEON VIDEO NETWORKS CONFIDENTIAL

*******************************************************************************/
#include "../iotest.h"

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

/* Function     :   gettimeofday
 * Description  :   Windows emulator function of gettimeofday linux.
 * Parameters   :
 *      1. Pointer to timeval structure.
 *      2. Pointer to timezone stucture
 * Return Value :  0 on success
 */
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  FILETIME ft;
  unsigned __int64 tmpres = 0;
  static int tzflag;

  if (NULL != tv) {
        GetSystemTimeAsFileTime(&ft);
        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;
        /*converting file time to unix epoch*/
        tmpres /= 10;  /*convert into microseconds*/
        tmpres -= DELTA_EPOCH_IN_MICROSECS;
        tv->tv_sec = (long)(tmpres / 1000000UL);
        tv->tv_usec = (long)(tmpres % 1000000UL);
  }
  if (NULL != tz) {
    if (!tzflag) {
        _tzset();
        tzflag++;
    }
    tz->tz_minuteswest = _timezone / 60;
    tz->tz_dsttime = _daylight;
  }
  return EXIT_SUCCESS;
}

/* Function     :   strtok_r
 * Description  :   Function to parse tokens from string.
 * Parameters   :
 *      1. str      Contains the string to be tokenised.
 *      2. delim    Deliter to seperate tokens
 *      3. nextp    save point.
 * Return value : 
 *    pointer to the next token OR 
 *    NULL if there are no more tokens.
 */
char *strtok_r(char *str, char *delim, char **nextp)
{
    char *ret;
    if (str == NULL) {
        str = *nextp;
    }
    str += strspn(str, delim);
    if (*str == '\0') {
        return NULL;
    }
    ret = str;
    str += strcspn(str, delim);
    if (*str) {
        *str++ = '\0';
    }
    *nextp = str;
    return ret;
}

/* Function     : getpagesize
 * Description  : This function returns page size used (for Windows).
 * Parameters   :
 * Return Value : Page size.
 */
int getpagesize(void)
{    
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo);
    return (int)siSysInfo.dwAllocationGranularity;
}

/* Function     : pthread_mutex_init
 * Description  : This function initialises critical section. 
 *              Used for windows compatibility.
 * Parameters   : pointer to mutex object 
 * Return Value : Returns 0 if successful or -1 on error.
 */
int pthread_mutex_init(pthread_mutex_t *mutex)
{
    if (!InitializeCriticalSectionAndSpinCount(mutex, 0x00000400)) {
        error("pthread_mutex_init failed\n");
        return -1;
    }    
    debug("pthread_mutex_init succeeded\n");
    return EXIT_SUCCESS;
}

/* Function     : pthread_mutex_lock
 * Description  : Lock the critical section. Compatibility with 
 *                pthread_mutex_lock
 * Parameters   : pointer to mutex object 
 * Return Value : Returns 0 if successful.
 */
int pthread_mutex_lock(pthread_mutex_t *mutex) 
{ 
    EnterCriticalSection(mutex);
    return EXIT_SUCCESS;
}

/* Function     : pthread_mutex_unlock
 * Description  : Unlocks the critical section. Compatibility with 
 *                pthread_mutex_unlock
 * Parameters   : Pointer to mutex object 
 * Return Value : Returns 0 if successful.
 */
int pthread_mutex_unlock(pthread_mutex_t *mutex) 
{
    LeaveCriticalSection(mutex);
    return EXIT_SUCCESS;
}

/* Function     : pthread_mutex_destroy
 * Description  : Remove the critical section. Compatibility with 
 *                pthread_mutex_destroy
 * Parameters   : pointer to mutex object 
 * Return Value : Returns 0 if successful.
 */
int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    DeleteCriticalSection(mutex);
    return EXIT_SUCCESS;
}
 
/* Function     : lseek
 * Description  : Sets the file read-write pointer to 
 *              specified offset from position desribed in whence
 * Parameters   : 1. fd    file descriptor of open file
 *              2. ofset    offset for displacement of file pointer
 *              3. whence    From where to move pointer.pointer to mutex object 
 * Return Value : Returns current pointer position if successful, -1 otherwise 
 *                with errno set to appropriate error code.
 */
off_t lseek(int fd, off_t offset, int whence)
{
    off_t ptr, off;
    LONG lDistanceToMove, lDistanceToMoveHigh;
    if(g_FdToHandlMap[fd].hFile == NULL) { /* Invalid file descriptor */
        strerror(errno);
        printf("fdtoh: %d\n",GetLastError());
        return -1;
    } else {
        lDistanceToMove = LODWORD(offset);
        lDistanceToMoveHigh = HIDWORD(offset);
        if((ptr = SetFilePointer(g_FdToHandlMap[fd].hFile, 
                                 lDistanceToMove,
                                 &lDistanceToMoveHigh,                                 
                                 (DWORD)whence)) < 0) {
            errno = GetLastError();
            return -1;
        } else {
            if(HIDWORD(offset) == 0) {
                off = ptr;
            } else {
                off = lDistanceToMoveHigh;
                off = (off << 32) + ptr; 
            }
            return off;
        }
    }
}

/* Function     : mmap
 * Description  : Function to implement mmap system call
 *               memory mapping of files
 * Parameters   :
 *              1. addr     The starting address for the new mapping
 *              2. length   specifies the length of the mapping
 *              3. prot     describes the desired memory protection
 *              4. flags    specifies whether mapping object is shared
 *              5. fd       file desciptor of source file
 *              6. offset   stating offset in fd
 * Return Value : 
 */
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    /* Create a file mapping object */
    DWORD dwSize;
    dwSize = GetFileSize(g_FdToHandlMap[fd].hFile, NULL);
    hMapObject = CreateFileMapping(g_FdToHandlMap[fd].hFile,
                                   NULL,
                                   PAGE_READWRITE,
                                   0,
                                   dwSize,
                                   NULL);
    if (hMapObject == NULL) {
        errno = GetLastError();
        return MAP_FAILED;
    } else {
    /* Create memeory map of specified length from specified offset*/
      hMemMap = MapViewOfFile(hMapObject,
                              FILE_MAP_WRITE,
                              HIDWORD(offset),
                              LODWORD(offset),
                              length);
        if (hMemMap == NULL) {
            errno = GetLastError();
            return MAP_FAILED;
        } else {
            return hMemMap;
        }
    }
}

/* Function     : mumap
 * Description  : Function to implement mumap system call
 *               unmapping of memory mapped files
 * Parameters   :
 *              1. addr     The starting address for the mapping
 *              2. length   specifies the length of the mapping
 * Return Value : Returns 0 on success, 
 *                       -1 on failure with errno set. 
 */
int munmap(void *addr, size_t length)
{
    if(!UnmapViewOfFile(addr)) {
        errno = GetLastError();
        return -1;
    } else {
        if(!CloseHandle(hMapObject)) {
            errno = GetLastError();
            return -1;
        } else {
            return EXIT_SUCCESS;
        }
    }
}

/* Function     : pthread_create
 * Description  : Function to create thread
 * Parameters   :
 *              1. thread           Pointer to thread handle variable
 *              2. attr             Pointer to pthread attributes structure
 *              3. start_routine    Pointer to thread function
 *              4. arg              Pointer to arguments to be passed 
 *                                  to thread function
 * Return Value : Returns 0 on success, 
 *                 error numner on failure. 
 */
int pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *), void *arg)
{
    DWORD dwTid;
    if(NULL == (*thread = CreateThread(NULL,
                                       0, 
                                       (LPTHREAD_START_ROUTINE)start_routine, 
                                       arg, 
                                       0, 
                                       &dwTid))) {
        errno = GetLastError();
        return errno;
    } else {
        debug("Thread created with thread ID %d\n", dwTid);
        return EXIT_SUCCESS;
    }
}

/* Function     : pthread_join
 * Description  : suspends execution of the calling thread 
 *                until the target thread terminates
 * Parameters   :
 *              1. thread           Pointer to thread handle variable
 *              2. value_ptr        Added for compatibility 
 * Return Value : Returns 0 on success, 
 *                 error numner on failure. 
 */
int pthread_join(pthread_t thread, void **value_ptr)
{
    if (WAIT_FAILED == WaitForSingleObject(thread, INFINITE)) {
        return (errno = GetLastError());
    } else {
        return EXIT_SUCCESS;
    }
}

/* Function     : open
 * Description  : Opens a file specified by pathname.
 * Parameters   : 1. pathname       Full path of file to be opened 
 *              2. flags          read/write permission flags 
 *              3. mode              specifies the permissions to use in case a new file is created
 * Return Value : Returns fd if successful, -1 otherwise.
 */
int open(const char *pathname, ...)
{
    int i, fd, flags;
    mode_t modes = S_IRWXU;
    BOOL creatflag = FALSE;
    va_list args;
    typedef enum argtype {PERMS,  MODE};

    va_start(args, pathname);
    for (i = 0;i < 2; i++) {
        switch(i)
        {
        case PERMS :
            flags = va_arg(args, int);
            if(flags >= O_CREAT)
                creatflag = TRUE;
            break;

        case MODE :
            modes = va_arg(args, mode_t);
            break;    
        }
        if (!creatflag)
            break;
    }
    va_end(args);
    if((fd = creat((LPTSTR)pathname, modes)) == -1) {
        errno = GetLastError();
        return -1;
    } else {
        return fd;
    }
}

/*  Function    :   signalP
 *  Description :   signal for Progress interval
 *  Parameters  :   l. signum   :    signal number
 *                  2. hand     :    signal handler to accsociate with
 *                                   specified signal
 *  Return Value:   returns 0
 */
int signalP(int signum, sighandler_t hand)
{
    sigAlarmHandlerP = hand;
    return 0;
}

/*  Function    :   ProgressThread
 *  Description :   Thread routine which calls alarm sighandler periodically
 *  Parameters  :   l. argument from CreateThread
 *  Return Value:   returns 0
 */
DWORD WINAPI ProgressThread(LPVOID lpArgs)
{
    Sleep(ProgressInterval * 1000);
    (*sigAlarmHandlerP)(SIGALRM);
    return 0;
}


/* Function     : SetProgressTimer
 * Description  : Sets timer for progress interval
 * Arguments    : 1. which--NA--
 *              2. value    pointer to itimerval structure
 *              3. ovalue    --NA--
 */
int SetProgressTimer(int which, const struct itimerval *value, struct itimerval *ovalue) 
{
    DWORD    dwThreadID;
    HANDLE    hThread;

    ProgressInterval = value->it_value.tv_sec;

    if ((hThread = CreateThread(NULL, 
                                0, 
                                ProgressThread, 
                                NULL, 
                                0,
                                &dwThreadID)) == NULL) {
        printf("CreateThread Fail...!\n");
        exit(EXIT_FAILURE);
    }
    SetThreadPriority(hThread, 2);
    return 0;
}

/*  Function    :   signalT
 *  Description :   signal for timeout
 *  Parameters  :   l. signum   :    signal number
 *                  2. hand     :    signal handler to accsociate with
 *                                   specified signal
 *  Return Value:   returns 0
 */
int signalT(int signum, sighandler_t hand)
{
    sigAlarmHandlerT = hand;
    return 0;
}

/*  Function    :   TimeOutThread
 *  Description :   Thread routine which calls timeoutHandler after timeout 
 *  Parameters  :   l. argument from CreateThread
 *  Return Value:   returns 0
 */
DWORD WINAPI TimeOutThread(LPVOID lpArgs)
{
    Sleep(TimeOutInterval * 1000);
    (*sigAlarmHandlerT)(SIGALRM);
    return 0;
}


/* Function     : SetTimeOutTimer
 * Description  : Sets timer for progress interval
 * Arguments    : 1. which--NA--
 *              2. value    pointer to itimerval structure
 *              3. ovalue    --NA--
 */
int SetTimeOutTimer(int which, const struct itimerval *value, struct itimerval *ovalue) 
{
    DWORD    dwThreadID;
    HANDLE    hThread;

    TimeOutInterval = value->it_value.tv_sec;

    if ((hThread = CreateThread(NULL,
                                0, 
                                TimeOutThread, 
                                NULL, 
                                0,
                                &dwThreadID)) == NULL) {
        printf("CreateThread Fail...!\n");
        exit(EXIT_FAILURE);
    }
    SetThreadPriority(hThread, 2);
    return 0;
}
