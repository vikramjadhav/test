/*******************************************************************************

Filename: ioutils.c

Description: Contains common utility functions.

Copyright (c) 1998-1999 Omneon Video Networks (TM)

OMNEON VIDEO NETWORKS CONFIDENTIAL

*******************************************************************************/

#include "iotest.h"

/*
 * Function         : omMgTimerHandler
 *
 * Description      : This function handles the timeout alarm.
 *
 * Parameters       :
 *       1. signum       Signal number.
 * Retrun Value
 */
void omMgTimerHandler (int signum)
{
    printf("Timer for %g seconds expired so exiting... \n", omContext.timeout);
    exit(ETIME);
}

/*
 * Function     : omMgCommonAddTimeout
 *
 * Description  : This function adds the timer alarm.
 *
 * Parameters   :
 *
 * Retrun Value
 *                  Returns SUCCESS if timer is set.
 *                  Retuens FAILURE if timer is not set.
 */
int omMgCommonAddTimeout()
{
    struct itimerval    value;
    int                 which;
    int                 i;

    which = ITIMER_REAL;
    i                           = (int)omContext.timeout;
    value.it_interval.tv_sec    = 0;
    value.it_interval.tv_usec   = 0;
    value.it_value.tv_sec       = i;
    value.it_value.tv_usec      = (long)((omContext.timeout - i) * 1000000);
#if defined(Windows)
    if (SetTimeOutTimer(which, &value, NULL) == -1) {
#else    
    if (setitimer(which, &value, NULL) == -1) {
#endif /* windows */
        error("Setting timer for timeout failed with  %s errocode \n",
                strerror(errno));
        return FAILURE;
    }
#if defined(Windows)
    signalT(SIGALRM, &omMgTimerHandler);
#else
    signal(SIGALRM, &omMgTimerHandler);
#endif
    return SUCCESS;
}

/*
 * Function         :  omMgProgressHandler
 *
 * Description      : This function prints the total read/write operations till 
 *                    now. It is called after the given progress interval.
 *
 * Parameters       :
 *       1. signum       Signal number.
 * Retrun Value
 */
void omMgProgressHandler (int signum)
{
    int noOfThreads         = 0;
    int threadCntr          = 0;
    uint64 writeTillNow     = 0;
    uint64 readTillNow      = 0;

    if (omContext.opcode == OP_WRITE) {
        noOfThreads = omContext.numWriteThreads;
    } else {
        noOfThreads = omContext.numReadThreads;
    }
    while (noOfThreads > threadCntr) {
        writeTillNow += omMgIoStats[threadCntr].writeDoneCount;
        readTillNow  += omMgIoStats[threadCntr].readDoneCount;
        threadCntr++;
    }
    printf("Till Now : Total write count %10"PRIu64""
                       "   Total read count %10"PRIu64" \n",
                    writeTillNow, readTillNow);
    omMgCommonAddPinterval();
}

/*
 * Function     : omMgCommonAddPinterval
 *
 * Description  : This function adds an alarm to print the read/write progress.
 *
 * Parameters   :
 *
 * Retrun Value
 *                  Returns SUCCESS if timer is set.
 *                  Retuens FAILURE if timer is not set.
 */
int omMgCommonAddPinterval()
{
    struct itimerval    value;
    int                 which;
    int                 i;

    which = ITIMER_REAL;
    i                           = (int)omContext.pinterval;
    value.it_interval.tv_sec    = 0;
    value.it_interval.tv_usec   = 0;
    value.it_value.tv_sec       = (i * 60);  /* Convert minutes into secs */
    value.it_value.tv_usec      =  0;
#if defined(Windows)
    if (SetProgressTimer(which, &value, NULL) == -1) {
#else
    if (setitimer(which, &value, NULL) == -1) {
#endif
        error("Setting timer for Progress Iterval failed with  %s errocode \n",
                strerror(errno));
        return FAILURE;
    }
#if defined(Windows)
    signalP(SIGALRM, &omMgProgressHandler);
#else
    signal(SIGALRM, &omMgProgressHandler);
#endif
    return SUCCESS;
}


/*
 * Function    : debug
 *
 * Description : This function provides debug msg wrapper
 */
void debug(const char *format, ...)
{
#if defined (DEBUG)
    va_list arg;
    uint32  thread;

    thread = (unsigned long int)pthread_self();
    va_start (arg, format);
    printf("Omneon : %"PRIu32" : ", thread);
    vprintf(format, arg);
    va_end(arg);
#endif /* DEBUG */
}


/*
 * Function    : error
 *
 * Description : This function provides error msg wrapper
 */
void error(const char *format, ...)
{
    va_list arg;
    uint32 thread;

    thread = (unsigned long int)pthread_self();
    va_start(arg, format);
    printf("Omneon : %"PRIu32" : error -> ", thread);
    vprintf(format, arg);
    va_end(arg);
}


/*
 * Function    : omMgReportResults
 *
 * Description : This function provides reporting functionality
 */
void omMgReportResults()
{
    if (g_verbose_level > 0) {
        omMgDisplayContext();
        printf("\nTotal Time               : Read : %6.3lf (mSec) & Write"
                ": %6.3lf (mSec)\n", omMgIoPerf.readTotalTime,
                omMgIoPerf.writeTotalTime);
        printf("Total Write/Read Size    : Read : %"PRIu64" KB  & Write "
                ": %"PRIu64" KB\n", omContext.rarguments.size / BYTES_IN_KB,
                omContext.warguments.size / BYTES_IN_KB);
        if (omMgIoPerf.writeTotalTime != 0) {
            printf("Average Write Speed      : %g MB/sec\n",
                    (((double)omContext.warguments.size /
                    omMgIoPerf.writeTotalTime) * 1000) / BYTES_IN_MB);
        }
        if (omMgIoPerf.readTotalTime != 0) {
            printf("Average Read Speed       : %g MB/sec\n",
                   (((double)omContext.rarguments.size /
                    omMgIoPerf.readTotalTime) * 1000) / BYTES_IN_MB);
        }
        printf("Average Write Latency    :\n");
        printf("Average Read Latency     :\n");
        printf("Good/Bad Verification    : Failed blocks (%"PRIu64")\n\n"
                , omMgIoPerf.failedVerifyOps);
        printf("Bottom 10%% Transfer Rate Average(R)    : \n");
        printf("Top 10%% Transfer Rate Average(R)       : \n");

        printf("Bottom 10%% Transfer Rate Average(W)    : \n");
        printf("Top 10%% Transfer Rate Average(W)       : \n");

        printf("Adjusted 80%% Average Transfer Rate     : \n");
        printf("Bottom 10%% Latency Average             : \n");
        printf("Top 10%% Latency Average                : \n");
        printf("Adjusted 80%% Latency Average           : \n");
    } else {
        printf("\nwrv.fs output -> Total Time(mSec) : read (%6.3lf)"
                "write (%6.3lf);", omMgIoPerf.readTotalTime,
                omMgIoPerf.writeTotalTime);
        printf("Average(MB/s) : ");
        if (omMgIoPerf.writeTotalTime != 0) {
            printf("write (%g)", (((double)omContext.warguments.size /
                                omMgIoPerf.writeTotalTime) * 1000) /
                                BYTES_IN_MB);
        }
        if (omMgIoPerf.readTotalTime != 0) {
            printf("read (%g)", (((double)omContext.rarguments.size /
                                omMgIoPerf.readTotalTime) * 1000) /
                                BYTES_IN_MB);
        }
        if (omMgIoPerf.readTotalTime != 0) {
            printf("; Verification : Failed blocks (%"PRIu64")",
                    omMgIoPerf.failedVerifyOps);
        }
        printf("\n");
    }
}

/*
 * Function    : omMgInitialiseDefaultContext
 *
 * Description : This function populates the default caontext
 */
void omMgInitialiseDefaultContext()
{
    omContext.verify                = FALSE;
    omContext.numWriteThreads       = 1;
    omContext.numReadThreads        = 1;
    omContext.sequence              = IO_SEQUENCE;
    omContext.type                  = IO_NOSPARSE;
    omContext.timeout               = NO_TIMEOUT;
    omContext.pinterval             = NO_PROGRESS;
    omContext.verbose               = FALSE;
    omContext.rarguments.offset     = 0;
    omContext.rarguments.count      = 0;
    omContext.rarguments.blockSize  = 0;
    omContext.rarguments.size       = 0;
    omContext.warguments.offset     = 0;
    omContext.warguments.count      = 0;
    omContext.warguments.blockSize  = 0;
    omContext.warguments.size       = 0;
    omContext.seed                  = 0;
    omContext.rarguments.sleep      = 0;
    omContext.sparseFactor          = 1;
    g_patternLen = 0;
}


/*
 * Function    : omMgDisplayContext
 *
 * Description : Display the context values
 */
void omMgDisplayContext()
{
    printf("--------------------------------------------------------------"
           "-------------------\n");
    if ((strcmp(omContext.inputFile, omContext.outputFile)) != 0) {
        printf("Input File    :");
        printf(" %-25s", omContext.inputFile);
    }
    printf("Output File   :");
    printf(" %s\n", omContext.outputFile);
    printf("Seed          : %-25"PRIu64"" , (omContext.seed));
    if (omContext.verify == FALSE) {
        printf("Verification  : %s\n", "No");
    } else {
        printf("Verification  : %s\n", "Yes");
    }

    printf("Write Threads : %-25d", omContext.numWriteThreads);
    printf("Read Threads  : %"PRIu16"\n", omContext.numReadThreads);

    if (omContext.sequence == 0) {
        printf("Sequence      : %-25s", "Sequential");
    } else {
        printf("Sequence      : %-25s", "Random");
    }
    if (omContext.type == 0) {
        printf("Type          : %s(factor : %"PRIu16")\n", "Sparse",
                omContext.sparseFactor);
    } else {
        printf("Type          : %s\n", "No-Sparse");
    }
    if (omContext.timeout != -1) {
        printf("Timeout       : %-25g", omContext.timeout);
    } else {
        printf("Timeout       : %-25s", "Infinite");
    }
    if (omContext.warguments.rwflag == IO_MMAP) {
        printf("Write IO      : %s\n", "Memory Mapped I/O");
    }
    if (omContext.warguments.rwflag == IO_DIRECT) {
        printf("Write IO      : %s\n", "Direct I/O");
    } else  if (omContext.warguments.rwflag == IO_BUFFERED) {
        printf("Write IO      : %s\n", "Buffered I/O");
    }
    if (omContext.rarguments.rwflag == IO_MMAP) {
        printf("Read IO       : %-25s\n", "Memory Mapped I/O");
    }
    if (omContext.rarguments.rwflag == IO_DIRECT) {
        printf("Read IO       : %-25s\n", "Direct I/O");
    } else if (omContext.rarguments.rwflag == IO_BUFFERED) {
        printf("Read IO       : %-25s\n", "Buffered I/O");
    }
    printf("-------------------------------------------------------------"
           "--------------------\n");

}

int omMgValidateFilename(char *path)
{
    /* kept as a place holeder code will be added later */
    return SUCCESS;
}
