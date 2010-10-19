/*******************************************************************************
 *
 * Filename     : fileops.c
 *
 * Description  : this file contains functions related with file descriptor
 * Copyright (c) 1998-1999 Omneon Video Networks (TM)
 *
 * OMNEON VIDEO NETWORKS CONFIDENTIAL
 *
 ******************************************************************************/
#include "win32.h"

struct FdToHandlMap g_FdToHandlMap[MAX_OPEN_FILES] = {NULL};
extern int g_FreeFDIndex    = -1;
extern int g_curFDCount     = 0;

/*  Function    :   isLastFD
 *  Description :   check whether last fd. Some error handling needs
 *                  to be done
 *  Parameters  :   none
 *  Return value:   1 if current fd number is last fd
 */
int isLastFD(void)
{
    return (g_curFDCount == FD_MAX);
}


/*  Function    :   MapHandleToFD
 *  Description :   maps next free fd dwCount. Some error handling needs
 *                  to be done. Need to device efficient free handle search
 *                  method.
 *  Parameters  :   1. hFile    : handle to map on fd
 *  Return value:   file descriptor
 */
int MapHandleToFD(HANDLE hFile)
{
    DWORD dwCounter;
    /* need to find some efficient method for the searching next free
        file handle e.g. using bitmap of for file handle */
    if (isLastFD()) {
        return -1;
    }
    for (dwCounter = 3; dwCounter < FD_MAX; dwCounter++) {
        if (g_FdToHandlMap[dwCounter].hFile == NULL) {
            g_FreeFDIndex = dwCounter;
            break;
        }
    }
    g_curFDCount++;
    g_FdToHandlMap[g_FreeFDIndex].hFile = hFile;
    return g_FreeFDIndex;
}


/*    Function    :   creat
 *    Description :   createFile wrapper with creat Unix system call
 *                    interface.
 *    Paramaters  :   1.lpFilePath    :    file path
 *                    2.Mode          :    holds unix style mode
 *    Return Value:   file descriptor on Success
 *                    on error returns -1
 */
int creat(LPTSTR lpFilePath, int Mode)
{
    HANDLE  hFile = NULL;
    HANDLE  hSecHeap = NULL;
    LPSECURITY_ATTRIBUTES pSa;

    pSa = ConvertUnixModeToWinSecAttr(hSecHeap, Mode);
    /* Assuming that file already exists */

    hFile = CreateFile (lpFilePath, GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE, pSa, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL, NULL);
    if ((hFile == INVALID_HANDLE_VALUE) && 
        ((errno = GetLastError()) == ERROR_FILE_NOT_FOUND)) {
        
        hFile = CreateFile (lpFilePath, GENERIC_READ | GENERIC_WRITE,
                            0, pSa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 
                            NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            ReportError("Error creating file");
            return -1;
        }
        HeapDestroy (hSecHeap); 
    }    
    return MapHandleToFD(hFile);
}


/*  Function    :   read
 *  Description :   ReadFile wrapper with read Unix system call interface.
 *  Paramaters  :   1. fd     :   file descriptor
 *                  2. buf    :   buffer to hold data from file
 *                  3. size   :   maximum number of bytes to read
 *  Return Value:   Number of bytes read on Success
 *                  on error returns -1
 */
int read(int fd, LPVOID buf, int64_t size)
{
    int status;
    int NumberOfBytesRead;

    status = ReadFile (g_FdToHandlMap[fd].hFile,
            buf, (unsigned long)size, &NumberOfBytesRead, NULL);

    return status ? NumberOfBytesRead : -1;
}


/*  Function    :    write
 *  Description :    WriteFile wrapper with write Unix system call
 *                       interface.
 *  Paramaters  :    1. fd      :   file descriptor
 *                   2. buf        :   buffer to hold data from file
 *                   3. size    :    maximum number of bytes to read
 *
 *  Return Value:    Number of bytes read on Success
 *                   on error returns -1
 */
int write(int fd, LPVOID buf, int64_t size)
{
    int status;
    int NumberOfBytesWritten;

    status = WriteFile (g_FdToHandlMap[fd].hFile, buf,
                        (unsigned long)size, &NumberOfBytesWritten,
                        NULL);
    if (!status) {
        ReportError("Writefile:\n");
    }
    return status ? NumberOfBytesWritten : -1;

}


/*  Function    :    UnMapHandleFromFD
 *  Description :    Unmaps specified fd from handle
 *  Paramaters  :    1. fd        :   file descriptor from close
 *  Return Value:    filedescriptor on Success
 *                   on error returns -1
 */
int UnMapHandleFromFD(int fd)
{
    int status;

    if (g_FdToHandlMap[fd].hFile == NULL) {
        printf("close : Invalid fd %d\n", fd);
        return -1;
    }
    g_curFDCount--;
    status = CloseHandle(g_FdToHandlMap[fd].hFile);
    g_FdToHandlMap[fd].hFile = NULL;
    return  status ? 0: -1;
}


/*  Function    :    close
 *  Description :    CloseHandle wrapper with close Unix system call
 *                   interface.
 *  Paramaters  :    1. fd        :   file descriptor
 *  Return Value:    Number of bytes read on Success
 *                   on error returns -1
 */
int close(int fd)
{
    return UnMapHandleFromFD(fd);
}


/*  Function    :    fsync
 *  Description :    FlushFileBuffers wrapper with fsync Unix system call
 *                            interface.
 *  Paramaters  :    1. fd        :   file descriptor
 *  Return Value:    0 read on Success
 *                   on error returns -1
 */
int fsync(int fd)
{
    return FlushFileBuffers(g_FdToHandlMap[fd].hFile) ? 0: -1;
}
