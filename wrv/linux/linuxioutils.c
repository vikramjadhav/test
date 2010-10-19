/*******************************************************************************

Filename: linuxioutils.c

Description: linux utility functions

Copyright (c) 1998-1999 Omneon Video Networks (TM)

OMNEON VIDEO NETWORKS CONFIDENTIAL

*******************************************************************************/
#include "../iotest.h"

/*
 * Function     : omMgNicenss
 *
 * Description  : This function changes the sheduling priority (nice value) of 
 *                the process.
 *
 * Parameters   :
 *
 */
void omMgNiceness()
{

    debug("%s\n", __FUNCTION__);
#if !defined(Windows)
    if (nice(omContext.nice) == -1) {
        error("Using default priority. Setting up new scheduling priority"
                "failed %s \n",  strerror(errno));
        exit(-1);
    }
#else
    debug("Niceness not supported under windows");
#endif

}


/*
 * Function     : omMgTimeElapsed
 *
 * Description  : This function calculates the time elapsed.
 *
 * Parameters   :
 *
 * Return Value :
 *                  Retruns the time from gettimeofday function.
 */
int64 omMgTimeElapsed()
{
    struct timeval tp;

    debug("%s\n", __FUNCTION__);

    if (gettimeofday(&tp, (struct timezone *) NULL) == -1) {
            error("gettimeofday failed  %s \n", strerror(errno));
            exit(-1);
    }
    return (int64)(((int64) (tp.tv_sec)) * 1000000 + (int64) (tp.tv_usec));
}


/*
 * Function     : omMgIoOpenInputFile
 *
 * Description  : This function opens the input file.
 *
 * Parameters   :
 *      1. fd       File Desscriptor
 *
 * Return Value :
 *                  Returns SUCCESS if file is opened successfully.
 *                  Returns FAILURE if opening of file fails.
 *
 */
int omMgIoOpenInputFile(int *fd)
{

    debug("%s\n", __FUNCTION__);

    if (strncmp(omContext.inputFile, "-", 1) != 0) {
        if (omContext.rarguments.rwflag == IO_DIRECT) {
            *fd = open(omContext.inputFile, O_RDONLY | O_DIRECT);
            if (*fd == -1) {
                error("open %s errorcode %s \n", omContext.inputFile,
                      strerror(errno));
                return FAILURE;
            } else {
                return SUCCESS;
            }
        }
        *fd = open(omContext.inputFile, O_RDONLY);
        if (*fd == -1) {
            error("open %s errorcode %s \n", omContext.inputFile,
                  strerror(errno));
            return FAILURE;
        } else {
            return SUCCESS;
        }
    } else {
        debug("stdin is the input file\n");
        *fd = 0;
    }
    return SUCCESS;
}


/*
 * Function     : omMgIoOpenOutputFile
 *
 * Description  : This function opens the output file.
 *
 * Parameters   :
 *      1. fd       File Desscriptor
 *
 * Return Value :
 *                  Return SUCCESS if file is opened successfully.
 *                  Return FAILURE if opening of file fails.
 */
int omMgIoOpenOutputFile(int *fd)
{

    debug("%s\n", __FUNCTION__);

    if (strncmp(omContext.outputFile, "-", 1) != 0) {
        if (omContext.warguments.rwflag == IO_DIRECT) {
            *fd = open(omContext.outputFile, O_RDWR | O_CREAT | O_DIRECT,
                       S_IRWXU | S_IRWXG | S_IRWXO);
            if (*fd == -1) {
                error("open %s errocode %s \n", omContext.outputFile,
                       strerror(errno));
                return FAILURE;
            } else {
                return SUCCESS;
            }
        }
        *fd = open(omContext.outputFile, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG |
                   S_IRWXO);
    } else {
        debug("stdout is the output file\n");
        *fd = 1;
    }
    if (*fd == -1) {
        error("open %s errocode %s \n", omContext.outputFile, strerror(errno));
        return FAILURE;
    } else {
        return SUCCESS;
    }
}


/*
 * Function     : omMgIoCommonRead
 *
 * Description  : This function reads from the file.
 *
 *  Parameters  :
 *      1. fd       File descriptor.
 *      2. offset   Offset from which read needs to be started.
 *      3. size     iosize
 *      4. buffer   Buffer which will filled after read operation.
 *
 * Return Value :
 *                  Returns SUCCESS if read was successful.
 *                  Returns FAILURE if read failed.
 */
int omMgIoCommonRead(int fd, uint64 offset, uint64  size, char *buffer)
{
    uint64 len;
    char *buf;
    buf = NULL;  /* initialized to suppress warning */

    debug("%s\n", __FUNCTION__);

    if (g_sectorSize == 0) {
            g_sectorSize = 512;
    }
    if (lseek(fd, (off_t)offset, SEEK_SET) == -1) {
            error("lseek failed %s \n", strerror(errno));
            exit(ESPIPE);
    }
    if (omContext.rarguments.rwflag == IO_DIRECT) {
        #if defined(Windows)
        fprintf(stderr, "DirectIO not supported in Windows \n"); 
        #else     
        if (posix_memalign((void **)&buf, g_sectorSize, size) != 0) {
            error("posix memalign failed %s \n", strerror(errno));
            return FAILURE;
        }
        len = read(fd, buf, size);
        strncpy(buffer, buf, (size_t)size);
        free(buf);
        #endif
    } else {
        len = read(fd, buffer, size);
    }
    if (len != size) {
        error("read file %2d offset %"PRIu64" length requested %10"PRIi64" "
              "length read %10"PRIi64" : error %s \n", fd, offset,
               size, len, strerror(errno));
        return FAILURE;
    } else {
        debug("read file %2d offset %"PRIu64" length %10"PRIi64"\n", fd,
                offset, len);
        return SUCCESS;
    }
}


/*
 * Function     : omMgIoCommonWrite
 *
 * Description  : This function performs write on the file.
 *
 * Parameters   :
 *      1. fd       File descriptor
 *      2. offset   File offset
 *      3. size     iosize
 *      4. buffer   Buffer, which is filled with pattern for write operation.
 *
 * Return Value :
 *                  Returns SUCCESS if write was successful.
 *                  Returns FAILURE if write failed.
 */
int omMgIoCommonWrite(int fd, uint64 offset, uint64 size, char *pattern)
{
    uint64 len;
    char *buf = NULL;

    debug("%s\n", __FUNCTION__);

    if (g_sectorSize == 0) {
            g_sectorSize = 512;
    }
    if (lseek(fd, (off_t)offset, SEEK_SET) == -1) {
            error("lseek failed %s\n", strerror(errno));
            exit(ESPIPE);
    }
    if (omContext.warguments.rwflag == IO_DIRECT) {
        #if !defined(Windows)
        if (posix_memalign((void **)&buf, g_sectorSize, size) != 0) {
            error("posix memalign failed %s\n", strerror(errno));
            return FAILURE;
        }
        strncpy(buf, pattern, (size_t)size);
        len = write(fd, buf, size);
        free(buf);
        #endif
    } else {
        len = write(fd, pattern, size);
    }
    if (len != size) {
        error("write file %2d offset %"PRIu64" length requested %10"PRIi64" "
              "length written %10"PRIi64" : error %s \n", fd, offset, size, 
              len, strerror(errno));
        return FAILURE;
    } else {
        debug("write file %2d offset %"PRIu64" length %10"PRIi64"\n", fd,
                offset, len);
        return SUCCESS;
    }
}


/*
 * Function     : omMgIoGetSectorSize
 *
 * Description  : This function returns the sector size.
 *
 * Parameters   :
 *
 * Return Values:
 *                  Returns the sector size if it is computed
 *                  successfully.
 *                  Returns FAILURE otherwise.
 */
int omMgIoGetSectorSize(char *File)
{
    int sectorSize = 512;
#if !defined(Windows)
    FILE *filePointerMtab;
    FILE *filePointerMounts;
    struct stat fileStat;
    struct stat deviceFileStat;
    struct mntent *mntEntry = NULL;
    int deviceFd;
    int rc;

    debug("%s\n", __FUNCTION__);


    if (stat(File, &fileStat) == -1) {
        error("stat for the file failed %s \n", strerror(errno));
    }
    filePointerMtab = setmntent("/etc/mtab", "r");
    filePointerMounts = setmntent("/proc/mounts", "r");
    while ((mntEntry = getmntent(filePointerMtab)) != NULL)
    {
        stat(mntEntry->mnt_fsname, &deviceFileStat);
        if (fileStat.st_dev == deviceFileStat.st_rdev) {
            if ((deviceFd = open(mntEntry->mnt_fsname, O_RDONLY)) < 0) {
                debug("error while opening device file %s, returning default"
                        " sector size 512 \n", strerror(errno));
                return sectorSize;
            }
            rc = ioctl(deviceFd, BLKSSZGET, &sectorSize);
            if (rc != 0) {
                sectorSize = 512;
            }
            return sectorSize;
        }
    }
    while ((mntEntry = getmntent(filePointerMounts)) != NULL)
    {
        stat(mntEntry->mnt_fsname, &deviceFileStat);
        if (fileStat.st_dev == deviceFileStat.st_rdev) {
            if ((deviceFd = open(mntEntry->mnt_fsname, O_RDONLY)) < 0) {
                error("error while opening device file %s \n",
                        strerror(errno));
                return FAILURE;
            }
            rc = ioctl(deviceFd, BLKSSZGET, &sectorSize);
            if (rc != 0) {
                sectorSize = 512;
            }
            return sectorSize;
        }
    }
#endif

    return sectorSize;
}
