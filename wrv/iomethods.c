/*******************************************************************************

Filename    :   iomethods.c

Description :   This file contains write, read and verify functions for
mmap/directio/bufferedio.

Copyright (c) 1998-1999 Omneon Video Networks (TM)

OMNEON VIDEO NETWORKS CONFIDENTIAL

*******************************************************************************/
#include "iotest.h"

#define _FILE_OFFSET_BITS    64
#define PAGE_SIZE           getpagesize()
#define MAX_ARRAY_SIZE      65356 /* Max size of g_iosizeArray array*/

int g_verifyInitialised     = FALSE;
#if !defined(Windows)
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
#else
pthread_mutex_t g_mutex;
#endif
uint64 **g_iosizeArray;       /* To store the relative offsets and iosize */
uint64 g_globalArrayIteration;/* No. of times iosizes mentioned in
                               * g_iosizeArray will be completely read/written
                               */
uint64 g_sumOfIosizeArray = 0;/* Sum of iosizes mentioned in g_iosizeArray */
uint64 g_offsetMark;          /* Offset mark, incremented by g_sumOfIosizeArray
                               * after each iteration of g_iosizeArray
                               */
int32 g_globalArrayCount;     /* No. of elements in g_iosizeArray */
uint64 **g_iosizeSubArray;    /* To store the relative offsets and iosize
                                which will used after writing
                                g_sumOfIosizeArray * g_globalArrayIteration */
uint64 g_globalSubArrayCount; /* No. of elements in g_iosizeSubArray*/
uint64 g_sumOfIosizeSubArray; /* Sum of iosizes mentioned in g_iosizeSubArray */
uint64 g_firstThreadExtraSize = 0;/* Extra size for first thread */
int g_write = 0;
int g_status                = SUCCESS;

#if defined(Windows)
struct omFileOperations fops = {
        omMgIoOpenInputFile,
        omMgIoOpenOutputFile,
        omMgIoBufferedDirectSeqRead,
        omMgIoMmapSeqRead,
        omMgIoBufferedDirectSeqRead,
        omMgIoBufferedDirectSeqWrite,
        omMgIoMmapSeqWrite,
        omMgIoBufferedDirectSeqWrite,
        omMgIoBufferedDirectRandomRead,
        omMgIoBufferedDirectRandomRead,
        omMgIoMmapRandomRead,
        omMgIoBufferedDirectRandomWrite,
        omMgIoMmapRandomWrite,
        omMgIoBufferedDirectRandomWrite,
};
#else 
struct omFileOperations fops = {
    .openInput          = omMgIoOpenInputFile,
    .openOutput         = omMgIoOpenOutputFile,
    .readDirectSeq      = omMgIoBufferedDirectSeqRead,
    .readDirectRandom   = omMgIoBufferedDirectRandomRead,
    .readBufferedSeq    = omMgIoBufferedDirectSeqRead,
    .readBufferedRandom = omMgIoBufferedDirectRandomRead,
    .readMmapSeq        = omMgIoMmapSeqRead,
    .readMmapRandom     = omMgIoMmapRandomRead,
    .writeDirectSeq     = omMgIoBufferedDirectSeqWrite,
    .writeDirectRandom  = omMgIoBufferedDirectRandomWrite,
    .writeBufferedSeq   = omMgIoBufferedDirectSeqWrite,
    .writeBufferedRandom= omMgIoBufferedDirectRandomWrite,
    .writeMmapSeq       = omMgIoMmapSeqWrite,
    .writeMmapRandom    = omMgIoMmapRandomWrite,
};
#endif


/*
 * Function     : omMgIoCommonAllocateBuffer
 *
 * Description  : This function allocates buffer.
 *
 * Parameters   :
 *      1. size     Size for the the buffer to be allocated.
 *
 * Return Value :
 *                 Pointer to the filled buffer.
 */
char* omMgIoCommonAllocateBuffer(int size)
{
    char *buffer = NULL;

    debug("%s\n", __FUNCTION__);

    buffer = (char *) malloc(size * sizeof(char));
    if (buffer == NULL) {
        error("Failed to allocate buffer of size %"PRIu32"\n", size);
        exit(ENOMEM);
    }
    debug("Allocated buffer of size %"PRIu32" at address %x \n",
          size, buffer);
    return buffer;
}


/*
 * Function     : omMgIoFillBuffer
 *
 * Description  : This function fills the buffer with pattern provided.
 *
 * Parameters   :
 *      1. buffer           Allocated buffer which is to to be filled with the
 *                          pattern.
 *      2. pattern          Pattern.
 *      3. lengthRemaining  Size of the buffer.
 *
 */
void  omMgIoFillBuffer(char *buffer, char *pattern, uint64 lengthRemaining)
{

    debug("%s\n", __FUNCTION__);

    while (lengthRemaining > 0) {
        if (lengthRemaining > g_patternLen) {
            memcpy(buffer, pattern, g_patternLen);
        } else {
            memcpy(buffer, pattern, (size_t)lengthRemaining);
            break;
        }
        lengthRemaining -= g_patternLen;
        buffer += g_patternLen;
    }
}


/*
 * Function    : omMgCollectIoStats
 *
 * Description : This function calculates the io statistics
 *
 */
void  omMgCollectIoStats()
{
    int32 numThreads           = 0;
    int32 i                    = 0;
    int64 minReadStartTime     = 0;
    int64 minWriteStartTime    = 0;
    int64 maxReadEndTime       = 0;
    int64 maxWriteEndTime      = 0;

    debug("%s\n", __FUNCTION__);

    if (omContext.opcode == OP_READ) {
        numThreads = omContext.numReadThreads;
    } else {
        numThreads = omContext.numWriteThreads;
    }
    for (i = 0; i < numThreads; i++) {
        if ((omMgIoStats[i].readStartTime < minReadStartTime) ||
                (minReadStartTime == 0)) {
            minReadStartTime = omMgIoStats[i].readStartTime;
        }
        if (omMgIoStats[i].readEndTime > maxReadEndTime) {
            maxReadEndTime = omMgIoStats[i].readEndTime;
        }
        if ((omMgIoStats[i].writeStartTime < minWriteStartTime) ||
                (minWriteStartTime == 0)) {
            minWriteStartTime = omMgIoStats[i].writeStartTime;
        }
        if (omMgIoStats[i].writeEndTime > maxWriteEndTime) {
            maxWriteEndTime = omMgIoStats[i].writeEndTime;
        }
    }
    omMgIoPerf.readStartTime = minReadStartTime;
    omMgIoPerf.readEndTime = maxReadEndTime;
    omMgIoPerf.writeStartTime = minWriteStartTime;
    omMgIoPerf.writeEndTime = maxWriteEndTime;
    /* store time in milliseconds */
    omMgIoPerf.readTotalTime = (omMgIoPerf.readEndTime -
            omMgIoPerf.readStartTime) * .001;
    omMgIoPerf.writeTotalTime = (omMgIoPerf.writeEndTime -
            omMgIoPerf.writeStartTime) * .001;
}


/*
 * Function     :   omMgIoAllocReadBuffer
 *
 * Description  :   This function chooses the required size
 *                  for read buffer.
 *
 * Parameters   :
 *      1. ioSize       iosize
 *      2. patternLen   Pattern length
 *
 * Return Value :
 *                      Pointer to the buffer which is double
 *                      either iosize or pattern length.
 */
char* omMgIoAllocReadBuffer(uint64 ioSize, uint32 patternLen)
{
    char *buffer;

    debug("%s\n", __FUNCTION__);

    if (ioSize <= patternLen) {
        if ((buffer = omMgIoCommonAllocateBuffer(patternLen *sizeof(char) *
                        2)) == NULL) {
            return NULL;
        }
    } else {
        if ((buffer = omMgIoCommonAllocateBuffer((int)ioSize * sizeof(char) *
                        2)) == NULL) {
            return NULL;
        }
    }
    return buffer;
}


/*
 * Function     :   omMgIoFillPattern
 *
 * Description  :   This function chooses the required buffer size
 *                  and fills it with pattern.
 *
 * Parameters   :
 *      1. ioSize       iosize
 *      2. patternLen   Pattern length
 *
 * Return Value :
 *                      Pointer to the buffer which is double
 *                      either iosize or pattern length.
 */
char* omMgIoFillPattern(uint64 ioSize, uint32 patternLen)
{
    char *buffer;

    debug("%s\n", __FUNCTION__);

    if (ioSize <= patternLen) {
        if ((buffer = omMgIoCommonAllocateBuffer(patternLen *sizeof(char) *
                        2)) == NULL) {
            return NULL;
        }
        omMgIoFillBuffer(buffer, g_pattern, patternLen * 2);
    } else {
        if ((buffer = omMgIoCommonAllocateBuffer((int)ioSize * sizeof(char) *
                        2)) == NULL) {
            return NULL;
        }
        omMgIoFillBuffer(buffer, g_pattern, ioSize * 2);
    }
    return buffer;
}


/*
 * Function     :   getArrayCount
 *
 * Description  :   This function calculates the number elements for global
 *                  array which contains offser and iosize.
 *
 * Parameters   :
 *
 * Return Value :
 *                  Returns the number elements for global array .
 */
uint64 getArrayCount(uint64 size)
{
    uint64 iosize;
    uint64 count = 0;

    debug("%s\n", __FUNCTION__);

    srand48(omContext.seed);
    while (size > 0) {
        if (g_randomIosize) {
            if (omContext.opcode == OP_READ) {
                iosize = (lrand48() % (omContext.rarguments.maxBlockSize +
                            1 - omContext.rarguments.minBlockSize) +
                        omContext.rarguments.minBlockSize);
            } else {
                iosize = (lrand48() % (omContext.warguments.maxBlockSize +
                            1 - omContext.warguments.minBlockSize) +
                        omContext.warguments.minBlockSize);
            }
        } else {
            if (omContext.opcode == OP_READ) {
                iosize = omContext.rarguments.blockSize;
            } else {
                iosize = omContext.warguments.blockSize;
            }
        }
        count++;
        if (size <= iosize) {
            break;
        } else {
            size -= iosize;
        }
    }
    return count;
}


/*
 * Function     :   createGlobalSubArray
 *
 * Description  :   This function created the global random array which
 *                  contains offser and iosize.This fuction is called
 *                  only when total size is greater than
 *                  (g_globalArrayIteration * g_sumOfIosizeArray * noOfThreads)
 *
 * Parameters   :
 *          size    Remaining size in bytes.
 *                  total size - (g_globalArrayIteration * g_sumOfIosizeArray *
 *                  noOfThreads)
 *
 * Return Value :
 */
void createGlobalSubArray(uint64 size)
{
    uint64 i      = 0;
    uint64 offset;
    uint64 iosize;
    uint64 sizeRemaining;
    uint64 sizePerThread;
    uint64 bigRand;
    uint64 subArrayCount;
    uint32 noOfThreads;
    uint64 *tmp;
    uint64 directIoOps      = 0;
    uint64 directIoOpsPerThread;

    debug("%s\n", __FUNCTION__);

    offset =  g_globalArrayIteration * g_offsetMark;
    if (omContext.opcode == OP_READ) {
        noOfThreads = omContext.numReadThreads;
        directIoOps = size / g_sectorSize;
        directIoOpsPerThread = directIoOps / noOfThreads;
    } else {
        noOfThreads = omContext.numWriteThreads;
        directIoOps = size / g_sectorSize;
        directIoOpsPerThread = directIoOps / noOfThreads;
    }
    if (omContext.warguments.rwflag == IO_DIRECT ||
                omContext.rarguments.rwflag == IO_DIRECT) {
             sizePerThread = directIoOpsPerThread *  g_sectorSize;
    } else {
        sizePerThread = (size / noOfThreads);
    }
    subArrayCount = getArrayCount(sizePerThread);
    g_globalSubArrayCount = subArrayCount;
    g_iosizeSubArray = (uint64 **)malloc((size_t)(sizeof(uint64 *) * 
                        subArrayCount));
    if (g_iosizeSubArray == NULL) {
        error("malloc failed \n");
        exit(ENOMEM);
    }
    for (i = 0; i < subArrayCount; i++) {
        g_iosizeSubArray[i] = malloc(2 * sizeof(uint64));
        if (g_iosizeSubArray[i] == NULL) {
            error("malloc failed \n");
            exit(ENOMEM);
        }
    }
    srand48(omContext.seed);
    sizeRemaining = sizePerThread;
    for (i = 0; i < subArrayCount; i++) {
        if (g_randomIosize) {
            if (omContext.opcode == OP_READ) {
                iosize = (lrand48() % (omContext.rarguments.maxBlockSize +
                            1 - omContext.rarguments.minBlockSize) +
                        omContext.rarguments.minBlockSize);
            } else {
                iosize = (lrand48() % (omContext.warguments.maxBlockSize +
                            1 - omContext.warguments.minBlockSize) +
                        omContext.warguments.minBlockSize);
            }
        } else {
            if (omContext.opcode == OP_READ) {
                iosize = omContext.rarguments.blockSize;
            } else {
                iosize = omContext.warguments.blockSize;
            }
        }
        if (sizeRemaining < iosize) {
            g_iosizeSubArray[i][1]  = sizeRemaining;
        } else {
            g_iosizeSubArray[i][1] = iosize;
        }
        g_iosizeSubArray[i][0] = offset;
        offset += g_iosizeSubArray[i][1] * omContext.sparseFactor;
        g_sumOfIosizeSubArray += g_iosizeSubArray[i][1];
        sizeRemaining -= iosize;
    }
    /* Randomizing g_ig_iosizeSubArray */
    srand48(omContext.seed);
    for (i = 0; i < subArrayCount; i++) {
        bigRand = lrand48();
        bigRand = bigRand % subArrayCount;
        tmp = g_iosizeSubArray[i];
        g_iosizeSubArray[i] = g_iosizeSubArray[bigRand];
        g_iosizeSubArray[bigRand] = tmp;
    }
    /* Calculate the extra size which will be given for first thread.*/
    if (omContext.opcode == OP_READ) {
        g_firstThreadExtraSize  = omContext.rarguments.size -
                                  (omContext.numReadThreads *
                                  ((g_sumOfIosizeArray *
                                  g_globalArrayIteration)  +
                                  (g_sumOfIosizeSubArray *
                                  g_globalSubArrayCount)));
    } else {
        g_firstThreadExtraSize = omContext.warguments.size -
                                (omContext.numWriteThreads *
                                ((g_sumOfIosizeArray *
                                g_globalArrayIteration)  +
                                (g_sumOfIosizeSubArray)));
    }
}


/*
 * Function     :   createGlobalArray
 *
 * Description  :   This function created the global  random array which
 *                  contains offser and iosize.
 *
 * Parameters   :
 *      size       Total read/write size in bytes.
 *
 * Return Value :
 */
void createGlobalArray(uint64 size)
{
    uint64 i      = 0;
    uint64 offset = 0;
    uint64 iosize;
    uint64 sizeRemaining;
    uint64 sizePerThread;
    uint64 bigRand;
    uint64 arrayCount;
    uint32 noOfThreads;
    uint64 *tmp;
    uint64 subSize;
    uint64 directIoOps      = 0;
    uint64 directIoOpsPerThread;

    debug("%s\n", __FUNCTION__);

    if (omContext.opcode == OP_READ) {
        noOfThreads = omContext.numReadThreads;
        directIoOps = omContext.rarguments.size / g_sectorSize;
        directIoOpsPerThread = directIoOps / noOfThreads;
    } else {
        noOfThreads = omContext.numWriteThreads;
        directIoOps = omContext.warguments.size / g_sectorSize;
        directIoOpsPerThread = directIoOps / noOfThreads;
    }
    if (omContext.warguments.rwflag == IO_DIRECT ||
                omContext.rarguments.rwflag == IO_DIRECT) {
             sizePerThread = directIoOpsPerThread *  g_sectorSize;
    } else {
            sizePerThread = (size / noOfThreads);
    }
    arrayCount = getArrayCount(sizePerThread);
    if (MAX_ARRAY_SIZE < arrayCount) {
        arrayCount =  MAX_ARRAY_SIZE;
    }
    g_globalArrayCount = (int32)arrayCount;
    g_iosizeArray = (uint64 **)malloc((size_t)(sizeof(uint64 *) * arrayCount));
    if (g_iosizeArray == NULL) {
        error("malloc failed \n");
        exit(ENOMEM);
    }
    for (i = 0; i < arrayCount; i++) {
        g_iosizeArray[i] = malloc(2 * sizeof(uint64));
        if (g_iosizeArray[i] == NULL) {
            error("malloc failed \n");
            exit(ENOMEM);
        }
    }
    srand48(omContext.seed);
    sizeRemaining = sizePerThread;
    for (i = 0; i < arrayCount; i++) {
        if (g_randomIosize) {
            if (omContext.opcode == OP_READ) {
                iosize = (lrand48() % (omContext.rarguments.maxBlockSize +
                            1 - omContext.rarguments.minBlockSize) +
                        omContext.rarguments.minBlockSize);
            } else {
                iosize = (lrand48() % (omContext.warguments.maxBlockSize +
                            1 - omContext.warguments.minBlockSize) +
                        omContext.warguments.minBlockSize);
            }
        } else {
            if (omContext.opcode == OP_READ) {
                iosize = omContext.rarguments.blockSize;
            } else {
                iosize = omContext.warguments.blockSize;
            }
        }
        if (sizeRemaining < iosize) {
            g_iosizeArray[i][1] = sizeRemaining;
        } else {
            g_iosizeArray[i][1] = iosize;
        }
        g_iosizeArray[i][0] = offset;
        sizeRemaining -= iosize;
        offset += g_iosizeArray[i][1] * omContext.sparseFactor;
        g_sumOfIosizeArray += g_iosizeArray[i][1];
    }
    /* Setting g_offsetMark to the last offset of g_iosizeArray*/
    g_offsetMark = offset;
    /* Calculation for g_globalArrayIteration */
    g_globalArrayIteration = sizePerThread / g_sumOfIosizeArray;
    subSize = size - (g_globalArrayIteration * g_sumOfIosizeArray *
                        noOfThreads);
    if (subSize != 0) {
    createGlobalSubArray(subSize);
    }
    /* Randomizing g_iosizeArray */
    srand48(omContext.seed);
    for (i = 0; i < arrayCount; i++) {
        bigRand = lrand48();
        bigRand = bigRand % arrayCount;
        tmp = g_iosizeArray[i];
        g_iosizeArray[i] = g_iosizeArray[bigRand];
        g_iosizeArray[bigRand] = tmp;
    }
}


/*
 * Function     :   omMgIoBufferedDirectSeqRead
 *
 * Description  :   This function performs sequential read for buffered and
 *                  direct I/O.
 *
 * Parameters   :
 *      1. fd       File descriptor.
 *      2. iostat   Pointer to thread specific omMgIoStat structure.
 * Return Value :
 *                  Returns SUCCESS if read was successful.
 *                  Returns FAILURE if read failed.
 */
int omMgIoBufferedDirectSeqRead(int fd, struct omMgIoStat *iostat)
{
    uint64 sizeRemaining    = iostat->size;
    uint64 offset           = iostat->start.offset;
    char *buffer            = NULL;
    bool finalRead          = false;
    uint64 i                = 0;
    uint64 iosize;
    uint64 randomSleep;

    debug("%s\n", __FUNCTION__);

    if (g_randomIosize) {
        if ((buffer = omMgIoAllocReadBuffer(omContext.rarguments.maxBlockSize,
                        g_patternLen)) == NULL) {
            return FAILURE;
        }
    } else {
        if ((buffer = omMgIoAllocReadBuffer(omContext.rarguments.blockSize,
                        g_patternLen)) == NULL) {
            return FAILURE;
        }
    }
    while (sizeRemaining != 0) {
        if (g_randomIosize) {
            iosize = g_iosizeArray[i][1];
            i++;
            if (i == g_globalArrayCount) {
                i = 0;
            }
        } else {
            iosize = omContext.rarguments.blockSize;
        }
        if ((iosize) <= sizeRemaining) {
            if (omMgIoCommonRead(fd, offset, iosize,
                        buffer) == FAILURE) {
                if (buffer != NULL) {
                    free(buffer);
                }
                return FAILURE;
            }
            if (omContext.pinterval != NO_PROGRESS) {
                iostat->readDoneCount++;
            }
        } else {
            if (omMgIoCommonRead(fd, offset, sizeRemaining, buffer) ==
                    FAILURE) {
                if (buffer != NULL) {
                    free(buffer);
                }
                return FAILURE;
            }
            if (omContext.pinterval != NO_PROGRESS) {
                iostat->readDoneCount++;
            }
            finalRead = true;
        }
        if (!g_randomSleep) {
            if (omContext.rarguments.sleep != 0) {
                usleep(omContext.rarguments.sleep);
            }
        } else {
            randomSleep = (rand() % (omContext.rarguments.maxSleep
                        + 1 - omContext.rarguments.minSleep)) +
                omContext.rarguments.minSleep;
            usleep(randomSleep);
        }
        if (omContext.verify == TRUE) {
            if (!finalRead) {
                if (omMgIoVerifyBlock(offset, buffer, iosize) == SUCCESS) {
                    debug("Verification  offset %10"PRIu64" length %10"PRIu64""
                            "\n", offset, iosize);
                } else {
                    error("Verification  offset %10"PRIu64" length %10"PRIu64""
                            "\n", offset, iosize);
                }
            } else {
                if (omMgIoVerifyBlock(offset, buffer, sizeRemaining) ==
                        SUCCESS) {
                    debug("Verification  offset %10"PRIu64" length %10"PRIu64""
                            "\n", offset, sizeRemaining);
                } else {
                    error("Verification  offset %10"PRIu64" length %10"PRIu64""
                            "\n", offset, sizeRemaining);
                }
            }
        }
        if (omContext.sparseFactor > 1) {
            offset +=  (omContext.sparseFactor * iosize);
        } else {
            offset += iosize;
        }
        if (!finalRead) {
            sizeRemaining -= (iosize);
        } else {
            sizeRemaining = 0;
        }
    }
    free(buffer);
    return SUCCESS;
}


/*
 * Function     :   omMgIoBufferedDirectRandomRead
 *
 * Description  :   This function performs random read for buffered and
 *                  direct I/O.
 *
 * Parameters   :
 *      1. fd           File descriptor.
 *      2. iostat       Pointer to thread specific omMgIoStat structure.
 * Return Value :
 *                      Returns SUCCESS if read was successful.
 *                      Returns FAILURE if read failed.
 */
int omMgIoBufferedDirectRandomRead(int fd, struct omMgIoStat *iostat)
{
    uint64 baseOffset       = iostat->start.baseOffset;
    uint64 size             = iostat->size;
    uint64 i                = 0;
    char *buffer            = NULL;
    uint64 count            = 0;
    uint64 offset;
    uint32 randomSleep;
    uint64 iosize;
    uint64 offsetMark;
    uint64 sizeRemaining;

    debug("%s : Baseoffset  %"PRIu64" size %"PRIu64"\n", __FUNCTION__,
            baseOffset, size);

    if (g_randomIosize) {
        if ((buffer = omMgIoAllocReadBuffer(omContext.rarguments.maxBlockSize,
                        g_patternLen)) == NULL) {
            return FAILURE;
        }
    } else {
        if ((buffer = omMgIoAllocReadBuffer(omContext.rarguments.blockSize,
                        g_patternLen)) == NULL) {
            return FAILURE;
        }
    }
    /* Do the I/O for (g_globalArrayIteration * g_sumOfIosizeArray) */
    offsetMark = baseOffset;
    for (count = 0; count < g_globalArrayIteration; count++) {
        for (i = 0; i != g_globalArrayCount; i++) {
            offset = g_iosizeArray[i][0] + offsetMark;
            iosize = g_iosizeArray[i][1];
            if (omMgIoCommonRead(fd, offset, iosize,
                                 buffer) == FAILURE) {
                free(buffer);
                return FAILURE;
            }
            if (omContext.pinterval != NO_PROGRESS) {
                iostat->readDoneCount++;
            }
            if (!g_randomSleep) {
                if (omContext.rarguments.sleep != 0) {
                    usleep(omContext.rarguments.sleep);
                }
            } else {
                randomSleep = (uint32)((rand() % 
                                (omContext.rarguments.maxSleep +
                                1 - omContext.rarguments.minSleep)) +
                                (uint32)omContext.rarguments.minSleep);
                usleep(randomSleep);
            }
            if (omContext.verify == TRUE) {
                if (omMgIoVerifyBlock(offset, buffer, iosize) ==
                        SUCCESS) {
                    debug("Verification  offset %10"PRIu64" length %10"PRIu64""
                            "\n", offset, iosize);
                } else {
                    error("Verification  offset %10"PRIu64" length %10"PRIu64""
                            "\n", offset, iosize);
                }
            }
        }
        offsetMark += g_offsetMark;
    }
    sizeRemaining = (size - (g_globalArrayIteration * g_sumOfIosizeArray));
    if (sizeRemaining != 0) {
        for (i = 0; i < g_globalSubArrayCount; i++) {
            offset =  g_iosizeSubArray[i][0] + baseOffset;
            iosize =  g_iosizeSubArray[i][1];
            if (omMgIoCommonRead(fd, offset, iosize,
                        buffer) == FAILURE) {
                free(buffer);
                return FAILURE;
            }
            if (omContext.pinterval != NO_PROGRESS) {
                iostat->readDoneCount++;
            }
            if (!g_randomSleep) {
                if (omContext.rarguments.sleep != 0) {
                    usleep(omContext.rarguments.sleep);
                }
            } else {
                randomSleep = (uint32)((rand() % 
                                (omContext.rarguments.maxSleep +
                                1 - omContext.rarguments.minSleep)) +
                                omContext.rarguments.minSleep);
                usleep(randomSleep);
            }
            if (omContext.verify == TRUE) {
                if (omMgIoVerifyBlock(offset, buffer, iosize) ==
                        SUCCESS) {
                    debug("Verification  offset %10"PRIu64" length %10"PRIu64""
                            "\n", offset, iosize);
                } else {
                    error("Verification  offset %10"PRIu64" length %10"PRIu64""
                            "\n", offset, iosize);
                }
                sizeRemaining -= g_iosizeSubArray[i][1];
            }
        }
    }
    if (g_firstThreadExtraSize != 0) {
        offset = g_sumOfIosizeSubArray + (g_globalArrayIteration *
                    g_sumOfIosizeArray);
        if (omMgIoCommonRead(fd, offset, g_firstThreadExtraSize,
                    buffer) == FAILURE) {
            free(buffer);
            return FAILURE;
        }
        if (omContext.pinterval != NO_PROGRESS) {
                iostat->readDoneCount++;
        }
        if (!g_randomSleep) {
            if (omContext.rarguments.sleep != 0) {
                usleep(omContext.rarguments.sleep);
            }
        } else {
            randomSleep = (uint32)((rand() % (omContext.rarguments.maxSleep +
                            1 - omContext.rarguments.minSleep)) +
                            omContext.rarguments.minSleep);
            usleep(randomSleep);
        }
        if (omContext.verify == TRUE) {
            if (omMgIoVerifyBlock(offset, buffer, g_firstThreadExtraSize) ==
                    SUCCESS) {
                debug("Verification  offset %10"PRIu64" length %10"PRIu64""
                        "\n", offset, g_firstThreadExtraSize);
            } else {
                error("Verification  offset %10"PRIu64" length %10"PRIu64""
                        "\n", offset, g_firstThreadExtraSize);
            }
        }
    }
    free(buffer);
    return SUCCESS;
}


/*
 * Function     :   omMgIoMmapSeqRead
 *
 * Description  :   This function performs mmap sequentitial read.
 *
 * Parameters   :
 *      1. fd       File descriptor.
 *      2. iostat   Pointer to thread specific omMgIoStat structure.
 * Return Value :
 *                  Returns SUCCESS if read was successful.
 *                  Returns FAILURE if read failied.
 */
int omMgIoMmapSeqRead(int fd, struct omMgIoStat *iostat)
{
    uint64 sizeRemaining    = iostat->size;
    uint64 fileOffset       = iostat->start.offset;
    void   *file_memory     = NULL;
    char   *buffer          = NULL;
    uint64 pageStart        = 0;
    uint64 skip             = 0;
    bool finalRead          = false;
    uint64 i                = 0;
    uint64 iosize;
    uint64 pageEnd;
    uint64 inpageOffset;
    uint32 randomSleep;

    debug("%s\n", __FUNCTION__);

    if (g_randomIosize) {
        if ((buffer = omMgIoAllocReadBuffer(omContext.rarguments.maxBlockSize,
                        g_patternLen)) == NULL) {
            return FAILURE;
        }
    } else {
        if ((buffer = omMgIoAllocReadBuffer(omContext.rarguments.blockSize,
                        g_patternLen)) == NULL) {
            return FAILURE;
        }
    }
    /* start from the block boundary before the offset */
    while (sizeRemaining != 0) {
        if (g_randomIosize) {
            iosize = g_iosizeArray[i][1];
            i++;
            if (i == g_globalArrayCount) {
                i = 0;
            }
        } else {
            iosize = omContext.rarguments.blockSize;
        }
        pageStart = fileOffset / PAGE_SIZE;
        if(sizeRemaining < PAGE_SIZE) {
            char *tmpBuffer;
            skip = fileOffset % g_patternLen;
            if ((tmpBuffer = omMgIoFillPattern(sizeRemaining, 
                                               g_patternLen)) == NULL) {
                return FAILURE;
            }
             if (omMgIoCommonRead(fd, fileOffset, sizeRemaining, 
                                   tmpBuffer + skip) ==
                    FAILURE) {
                if ((buffer != NULL) && (buffer != g_pattern)) {
                        free(tmpBuffer);
                        free(buffer);
                }
                return FAILURE;
            }
            free(tmpBuffer);
            fileOffset += sizeRemaining;
            sizeRemaining = 0;
        } else {
            if  ((iosize) <= sizeRemaining) {
                pageEnd = (fileOffset + iosize + PAGE_SIZE -1) /
                    PAGE_SIZE;
                debug("mmap          offset %10"PRIu64" page   %10"PRIu64"" \
                        "size %10" PRIu64"\n", fileOffset, pageStart, iosize);
            } else {
                pageEnd = (fileOffset + sizeRemaining  + PAGE_SIZE -1) /
                    PAGE_SIZE;
                debug("mmap          offset %10"PRIu64" page   %10"PRIu64"" \
                      "size %10" PRIu64"\n", fileOffset, pageStart, 
                      sizeRemaining);
                finalRead = true;
            }
            inpageOffset = fileOffset % PAGE_SIZE;
            file_memory = mmap(0, (size_t)((pageEnd - pageStart) * PAGE_SIZE), 
                               PROT_READ, MAP_SHARED, fd, 
                               (off_t)(pageStart * PAGE_SIZE));
            if (file_memory == MAP_FAILED) {
                error("mmap failed ! %s\n", strerror(errno));
                if (buffer != NULL) {
                    free(buffer);
                }
                return FAILURE;
            }
            if (!finalRead) {
                memcpy(buffer, (char *)(file_memory) + inpageOffset,
                        (size_t)iosize);
                munmap(file_memory, (size_t)((pageEnd - pageStart) * PAGE_SIZE));
            } else {
                memcpy(buffer, (char *)(file_memory) + inpageOffset,
                        (size_t)sizeRemaining);
                munmap(file_memory, (size_t)((pageEnd - pageStart) * PAGE_SIZE));
            }
            if (omContext.pinterval != NO_PROGRESS) {
                iostat->readDoneCount++;
            }
            if (!g_randomSleep) {
                if (omContext.rarguments.sleep != 0) {
                    usleep(omContext.rarguments.sleep);
                }
            } else {
                randomSleep = (uint32)((rand() % (omContext.rarguments.maxSleep +
                                1 - omContext.rarguments.minSleep)) +
                                omContext.rarguments.minSleep);
                usleep(randomSleep);
            }
            if (omContext.verify == TRUE) {
                if (!finalRead) {
                    if (omMgIoVerifyBlock(fileOffset, buffer, iosize) == SUCCESS) {
                        debug("Verification  offset %10"PRIu64" length %10"PRIu64""
                                "\n", fileOffset, iosize);
                    } else {
                        error("Verification  offset %10"PRIu64" length %10"PRIu64""
                                "\n", fileOffset, iosize);
                    }
                } else {
                    if (omMgIoVerifyBlock(fileOffset, buffer, sizeRemaining) ==
                            SUCCESS) {
                        debug("Verification  offset %10"PRIu64" length %10"PRIu64""
                                "\n", fileOffset, sizeRemaining);
                    } else {
                        error("Verification  offset %10"PRIu64" length %10"PRIu64""
                                "\n", fileOffset, sizeRemaining);
                    }
                }
            }
            if (omContext.sparseFactor > 1) {
                fileOffset +=  (omContext.sparseFactor * iosize);
            } else {
                fileOffset += iosize;
            }
            sizeRemaining -= iosize;
            if (finalRead) {
                sizeRemaining = 0;
            }
        }
    }
    free(buffer);
    return SUCCESS;
}


/*
 * Function     : omMgIoMmapRandomRead
 *
 * Description  : This function performs mmap random read.
 *
 * Parameters   :
 *      1. fd       File descriptor.
 *      2. iostat   Pointer to thread specific omMgIoStat structure.
 * Return Value :
 *                      Returns SUCCESS if read was successful.
 *                      Returns FAILURE if read failed.
 */
int omMgIoMmapRandomRead(int fd, struct omMgIoStat *iostat)
{
    uint64 offset;
    uint64 baseOffset       = iostat->start.baseOffset;
    uint64  size            = iostat->size;
    void   *file_memory     = NULL;
    char   *buffer          = NULL;
    uint64 inpageOffset     = 0;
    uint64 pageStart        = 0;
    uint64 pageEnd          = 0;
    uint64 count            = 0;
    uint64 i                = 0;
    uint64  randomSleep;
    uint64 offsetMark;
    uint64 iosize;
    uint64 sizeRemaining;

    debug("%s : Baseoffset  %"PRIu64" size %"PRIu64"\n", __FUNCTION__,
            baseOffset, size);

    if (g_randomIosize) {
        if ((buffer = omMgIoAllocReadBuffer(omContext.rarguments.maxBlockSize,
                        g_patternLen)) == NULL) {
            return FAILURE;
        }
    } else {
        if ((buffer = omMgIoAllocReadBuffer(omContext.rarguments.blockSize,
                        g_patternLen)) == NULL) {
            return FAILURE;
        }
    }
    /* Do the I/O for (g_globalArrayIteration * g_sumOfIosizeArray) */
    offsetMark = baseOffset;
    for (count = 0; count < g_globalArrayIteration; count++) {
        for (i = 0; i != g_globalArrayCount; i++) {
            offset = g_iosizeArray[i][0] + offsetMark;
            iosize = g_iosizeArray[i][1];
            pageStart = offset / PAGE_SIZE;
            pageEnd = (offset + iosize + PAGE_SIZE -1) /
                PAGE_SIZE;
            inpageOffset = offset % PAGE_SIZE;
            file_memory = mmap(0, (size_t)((pageEnd - pageStart) * PAGE_SIZE), PROT_READ,
                    MAP_SHARED, fd, (off_t)(pageStart * PAGE_SIZE));
            debug("mmap          offset %10"PRIu64" page   %10"PRIu64" size %10"
                    PRIu64"\n", offset, pageStart, iosize);
            if (file_memory == MAP_FAILED) {
                error("mmap failed ! %s\n", strerror(errno));
                if (buffer != NULL) {
                    free(buffer);
                }
                return FAILURE;
            }
            memcpy(buffer, (char *)(file_memory) + inpageOffset,
                    (size_t)iosize);
            munmap(file_memory, (size_t)((pageEnd - pageStart) * PAGE_SIZE));
            if (omContext.pinterval != NO_PROGRESS) {
                iostat->readDoneCount++;
            }
            if (!g_randomSleep) {
                if (omContext.rarguments.sleep != 0) {
                    usleep(omContext.rarguments.sleep);
                }
            } else {
                randomSleep = (rand() % (omContext.rarguments.maxSleep
                            + 1 - omContext.rarguments.minSleep)) +
                    omContext.rarguments.minSleep;
                usleep(randomSleep);
            }
            if (omContext.verify == TRUE) {
                if (omMgIoVerifyBlock(offset, buffer, iosize) ==
                        SUCCESS) {
                    debug("Verification  offset %10"PRIu64" length %10"PRIu64""
                            "\n", offset, iosize);
                } else {
                    error("Verification  offset %10"PRIu64" length %10"PRIu64""
                            "\n", offset, iosize);
                }
            }
        }
        offsetMark += g_offsetMark;
    }
    sizeRemaining = (size - (g_globalArrayIteration * g_sumOfIosizeArray));
    if (sizeRemaining != 0) {
        for (i = 0; i < g_globalSubArrayCount; i++) {
            offset =  g_iosizeSubArray[i][0] + baseOffset;
            iosize =  g_iosizeSubArray[i][1];
            pageStart = offset / PAGE_SIZE;
            pageEnd = (offset + iosize + PAGE_SIZE -1) /
                PAGE_SIZE;
            inpageOffset = offset % PAGE_SIZE;
            file_memory = mmap(0, (size_t)((pageEnd - pageStart + 1) * PAGE_SIZE), PROT_READ,
                    MAP_SHARED, fd, (off_t)(pageStart * PAGE_SIZE));
            debug("mmap          offset %10"PRIu64" page   %10"PRIu64" size %10"
                    PRIu64"\n", offset, pageStart, iosize);
            if (file_memory == MAP_FAILED) {
                error("mmap failed ! %s\n", strerror(errno));
                if (buffer != NULL) {
                    free(buffer);
                }
                return FAILURE;
            }
            memcpy(buffer, (char *)(file_memory) + inpageOffset,
                    (size_t)iosize);
            munmap(file_memory, (size_t)((pageEnd - pageStart) * PAGE_SIZE));
            if (omContext.pinterval != NO_PROGRESS) {
                iostat->readDoneCount++;
            }
            if (!g_randomSleep) {
                if (omContext.rarguments.sleep != 0) {
                    usleep(omContext.rarguments.sleep);
                }
            } else {
                randomSleep = (rand() % (omContext.rarguments.maxSleep
                            + 1 - omContext.rarguments.minSleep)) +
                    omContext.rarguments.minSleep;
                usleep(randomSleep);
            }
            if (omContext.verify == TRUE) {
                if (omMgIoVerifyBlock(offset,
                            buffer,iosize) == SUCCESS) {
                    debug("Verification  offset %10"PRIu64" length %10"PRIu64""
                            "\n", offset, iosize);
                } else {
                    error("Verification  offset %10"PRIu64" length %10"PRIu64""
                            "\n", offset, iosize);
                }
            }
            sizeRemaining -= iosize;
        }
    }
    if (g_firstThreadExtraSize != 0) {
        offset = g_sumOfIosizeSubArray + (g_globalArrayIteration *
                                          g_sumOfIosizeArray);
        pageStart = offset / PAGE_SIZE;
        pageEnd = (offset + g_firstThreadExtraSize + PAGE_SIZE -1) /
            PAGE_SIZE;
        inpageOffset = offset % PAGE_SIZE;
        file_memory = mmap(0, (size_t)((pageEnd - pageStart + 1) * PAGE_SIZE), PROT_READ,
                           MAP_SHARED, fd, (off_t)(pageStart * PAGE_SIZE));
        debug("mmap          offset %10"PRIu64" page   %10"PRIu64" size %10"
              PRIu64"\n", offset, pageStart, g_firstThreadExtraSize);
        if (file_memory == MAP_FAILED) {
            error("mmap failed ! %s\n", strerror(errno));
            if (buffer != NULL) {
                free(buffer);
            }
            return FAILURE;
        }
        memcpy(buffer, (char *)(file_memory) + inpageOffset, 
               (size_t)g_firstThreadExtraSize);
        munmap(file_memory, (size_t)((pageEnd - pageStart) * PAGE_SIZE));
        if (omContext.pinterval != NO_PROGRESS) {
            iostat->readDoneCount++;
        }
        if (!g_randomSleep) {
            if (omContext.rarguments.sleep != 0) {
                usleep(omContext.rarguments.sleep);
            }
        } else {
            randomSleep = (rand() % (omContext.rarguments.maxSleep
                                     + 1 - omContext.rarguments.minSleep)) +
                omContext.rarguments.minSleep;
            usleep(randomSleep);
        }
        if (omContext.verify == TRUE) {
            if (omMgIoVerifyBlock(offset,
                                  buffer,g_firstThreadExtraSize) == SUCCESS) {
                debug("Verification  offset %10"PRIu64" length %10"PRIu64""
                      "\n", offset, g_firstThreadExtraSize);
            } else {
                error("Verification  offset %10"PRIu64" length %10"PRIu64""
                      "\n", offset, g_firstThreadExtraSize);
            }
        }
    }
    free(buffer);
    return SUCCESS;
}

/*
 * Function     :   omMgIoBufferedDirectSeqWrite
 *
 * Description  :   This function performs seq write for buffered and
 *                  direct I/O.
 *
 * Parameters   :
 *      1. fd       File descriptor.
 *      2. iostat   Pointer to thread specific omMgIoStat structure.
 * Return Value :
 *                  Returns SUCCESS if write was successful.
 *                  Returns FAILURE if write failed.
 */
int omMgIoBufferedDirectSeqWrite(int fd, struct omMgIoStat *iostat)

{
    uint64 sizeRemaining    = iostat->size;
    uint64 offset           = iostat->start.offset;
    char   *buffer          = NULL;
    uint64 skip             = 0;
    bool finalWrite         = false;
    uint64 i                = 0;
    uint64 iosize;
    uint32 randomSleep;

    debug("%s\n", __FUNCTION__);

    if (g_randomIosize) {
        if ((buffer = omMgIoFillPattern(omContext.warguments.maxBlockSize,
                        g_patternLen)) == NULL) {
            return FAILURE;
        }
    } else {
        if ((buffer = omMgIoFillPattern(omContext.warguments.blockSize,
                        g_patternLen)) == NULL) {
            return FAILURE;
        }
    }
    while (sizeRemaining != 0) {
        if (g_randomIosize) {
            iosize = g_iosizeArray[i][1];
            i++;
            if (i == g_globalArrayCount) {
                i = 0;
            }
        } else {
            iosize = omContext.warguments.blockSize;
        }
        skip = (offset % g_patternLen);
        if  ((iosize) <= sizeRemaining) {
            if (omMgIoCommonWrite(fd, offset, iosize,
                        buffer + skip) == FAILURE) {
                if (buffer != NULL && (buffer != g_pattern)) {
                    free(buffer);
                }
                return FAILURE;
            }
            if (omContext.pinterval != NO_PROGRESS) {
                iostat->writeDoneCount++;
            }
        } else {
            if (omMgIoCommonWrite(fd, offset, sizeRemaining, buffer + skip) ==
                    FAILURE) {
                if ((buffer != NULL) && (buffer != g_pattern))
                    free(buffer);
                return FAILURE;
            }
            if (omContext.pinterval != NO_PROGRESS) {
                iostat->writeDoneCount++;
            }
            finalWrite = true;
        }
        if (!g_randomSleep) {
            if (omContext.warguments.sleep != 0) {
                usleep(omContext.warguments.sleep);
            }
        } else {
            randomSleep = (uint32)((rand() % (omContext.warguments.maxSleep +
                            1 - omContext.warguments.minSleep)) +
                            omContext.warguments.minSleep);
            usleep(randomSleep);
        }
        if (omContext.sparseFactor > 1) {
            offset +=  (omContext.sparseFactor * iosize);
        } else {
            offset += iosize;
        }
        if (!finalWrite) {
            sizeRemaining -= (iosize);
        } else {
            sizeRemaining = 0;
        }
    }
    free(buffer);
    return SUCCESS;
}


/*
 * Function     :   omMgIoBufferedDirectRandomWrite
 *
 * Description  :   This function performs random write for buffered and
 *                  direct I/O.
 *
 * Parameters   :
 *      1. fd           File descriptor.
 *      2. iostat   Pointer to thread specific omMgIoStat structure.
 * Return Value :
 *                      Returns SUCCESS if write was successful.
 *                      Returns FAILURE if write failed.
 */
int omMgIoBufferedDirectRandomWrite(int fd, struct omMgIoStat *iostat)
{
    uint64 baseOffset   = iostat->start.baseOffset;
    uint64 size         = iostat->size;
    uint64  i           = 0;
    uint64 skip         = 0;
    uint64 count        = 0;
    uint64  offset;
    char   *buffer;
    uint32 randomSleep;
    uint64 iosize;
    uint64 offsetMark;
    uint64 sizeRemaining;

    debug("%s : Baseoffset  %"PRIu64" size %"PRIu64"\n", __FUNCTION__,
            baseOffset, size);


    if (g_randomIosize) {
        if ((buffer = omMgIoFillPattern(omContext.warguments.maxBlockSize,
                        g_patternLen)) == NULL) {
            return FAILURE;
        }
    } else {
        if ((buffer = omMgIoFillPattern(omContext.warguments.blockSize,
                        g_patternLen)) == NULL) {
            return FAILURE;
        }
    }
    /* Do the I/O for (g_globalArrayIteration * g_sumOfIosizeArray) */
    offsetMark = baseOffset;
    for (count = 0; count < g_globalArrayIteration; count++) {
        for (i = 0; i != g_globalArrayCount; i++) {
            offset = g_iosizeArray[i][0] + offsetMark;
            skip = offset % g_patternLen;
            iosize = g_iosizeArray[i][1];
            if (omMgIoCommonWrite(fd, offset, iosize,
                        buffer + skip) == FAILURE) {
                if (buffer != NULL)
                    free(buffer);
                return FAILURE;
            }
            if (omContext.pinterval != NO_PROGRESS) {
                iostat->writeDoneCount++;
            }
            if (!g_randomSleep) {
                if (omContext.warguments.sleep != 0) {
                    usleep(omContext.warguments.sleep);
                }
            } else {
                randomSleep = (uint32)((rand() % (omContext.warguments.maxSleep +
                                1 - omContext.warguments.minSleep)) +
                                omContext.warguments.minSleep);
                usleep(randomSleep);
            }
        }
        offsetMark += g_offsetMark;
    }
    sizeRemaining = (size - (g_globalArrayIteration * g_sumOfIosizeArray));
    if (sizeRemaining != 0) {
        for (i = 0; i < g_globalSubArrayCount; i++) {
            offset =  g_iosizeSubArray[i][0] + baseOffset;
            skip = offset % g_patternLen;
            iosize =  g_iosizeSubArray[i][1];
            if (omMgIoCommonWrite(fd, offset, iosize,
                        buffer + skip) == FAILURE) {
                if (buffer != NULL)
                    free(buffer);
                return FAILURE;
            }
            if (omContext.pinterval != NO_PROGRESS) {
                iostat->writeDoneCount++;
            }
            if (!g_randomSleep) {
                if (omContext.warguments.sleep != 0) {
                    usleep(omContext.warguments.sleep);
                }
            } else {
                randomSleep = (uint32)((rand() % (omContext.warguments.maxSleep +
                                1 - omContext.warguments.minSleep)) +
                                omContext.warguments.minSleep);
                usleep(randomSleep);
            }
            sizeRemaining -= g_iosizeSubArray[i][1];
        }
    }
    if (g_firstThreadExtraSize != 0) {
        offset = g_sumOfIosizeSubArray + (g_globalArrayIteration *
                    g_sumOfIosizeArray);
        skip = offset % g_patternLen;
        if (omMgIoCommonWrite(fd, offset, g_firstThreadExtraSize,
                    buffer + skip) == FAILURE) {
            if (buffer != NULL)
                free(buffer);
            return FAILURE;
        }
        if (!g_randomSleep) {
            if (omContext.warguments.sleep != 0) {
                usleep(omContext.warguments.sleep);
            }
        } else {
            randomSleep = (uint32)((rand() % (omContext.warguments.maxSleep +
                            1 - omContext.warguments.minSleep)) +
                            omContext.warguments.minSleep);
            usleep(randomSleep);
        }
    }
    free(buffer);
    return SUCCESS;
}


/*
 * Function     : omMgIoMmapSeqWrite
 *
 * Description  : This function performs mmap sequential write.
 *
 * Parameters   :
 *      1. fd       File descriptor.
 *      2. iostat   Pointer to thread specific omMgIoStat structure.
 * Return Value :
 *                  Returns SUCCESS if write was successful.
 *                  Returns FAILURE if write failed.
 */
int omMgIoMmapSeqWrite(int fd, struct omMgIoStat *iostat)
{
    uint64 sizeRemaining    = iostat->size;
    uint64 fileOffset       = iostat->start.offset;
    void *file_memory       = NULL;
    char *buffer            = NULL;
    uint64 skip             = 0;
    uint64 pageEnd          = 0;
    uint64 pageStart        = 0;
    bool finalWrite         = false;
    uint64  i               = 0;
    uint64 iosize;
    uint64 inpageOffset;
    uint32 randomSleep;

    debug("%s\n", __FUNCTION__);

    if (g_randomIosize) {
        if ((buffer = omMgIoFillPattern(omContext.warguments.maxBlockSize,
                        g_patternLen)) == NULL) {
            return FAILURE;
        }
    } else {
        if ((buffer = omMgIoFillPattern(omContext.warguments.blockSize,
                        g_patternLen)) == NULL) {
            return FAILURE;
        }
    }
    while (sizeRemaining != 0) {
        if (g_randomIosize) {
            iosize = g_iosizeArray[i][1];
            i++;
            if (i == g_globalArrayCount) {
                i = 0;
            }
        } else {
            iosize = omContext.warguments.blockSize;
        }
        
        pageStart = fileOffset / PAGE_SIZE;
        if(sizeRemaining < PAGE_SIZE) {
            char *tmpBuffer;
            skip = fileOffset % g_patternLen;
            if ((tmpBuffer = omMgIoFillPattern(sizeRemaining,g_patternLen)) == NULL) {
                return FAILURE;
            }
             if (omMgIoCommonWrite(fd, fileOffset, sizeRemaining, tmpBuffer + skip) ==
                    FAILURE) {
                if ((buffer != NULL) && (buffer != g_pattern)) {
                        free(tmpBuffer);
                        free(buffer);
                }
                return FAILURE;
            }
            free(tmpBuffer);
            fileOffset += sizeRemaining;
            sizeRemaining = 0;
        } else {
            if ((iosize) <= sizeRemaining) {
                pageEnd = (fileOffset + iosize + PAGE_SIZE - 1) /
                            PAGE_SIZE;
                debug("mmap          offset %10"PRIu64" page   %10"PRIu64" size %10"
                        PRIu64"\n", fileOffset, pageStart, iosize);
            } else {
                pageEnd = (fileOffset + sizeRemaining + PAGE_SIZE - 1) /
                            PAGE_SIZE;
                debug("mmap          offset %10"PRIu64" page   %10"PRIu64" size %10"
                            PRIu64"\n", fileOffset, pageStart, sizeRemaining);
                finalWrite = true;
            }
            inpageOffset = fileOffset % PAGE_SIZE;
           
            file_memory = mmap(0, (size_t)((pageEnd - pageStart) * PAGE_SIZE), 
                               PROT_READ | PROT_WRITE, MAP_SHARED, 
                               fd, (off_t)(pageStart * PAGE_SIZE));
            if (file_memory == MAP_FAILED) {
                error("mmap failed ! %s\n", strerror(errno));
                if (buffer != NULL) {
                    free(buffer);
                }
                return FAILURE;
            }
            skip = fileOffset % g_patternLen;
            if (omContext.sparseFactor > 1) {
                fileOffset +=  (omContext.sparseFactor * iosize);
            } else {
                fileOffset += iosize;
            }
            if (!finalWrite) {
                memcpy((char *)(file_memory) + inpageOffset, buffer + skip,
                        (size_t)iosize);
                munmap(file_memory, (size_t)((pageEnd - pageStart) * PAGE_SIZE));
                sizeRemaining -= iosize;
            }  else {
                memcpy((char *)(file_memory) + inpageOffset, buffer + skip,
                        (size_t)sizeRemaining);
                munmap(file_memory, (size_t)((pageEnd - pageStart) * PAGE_SIZE));
                fileOffset += sizeRemaining;
                sizeRemaining = 0;
            }
        }
        if (omContext.pinterval != NO_PROGRESS) {
            iostat->writeDoneCount++;
        }
        if (!g_randomSleep) {
            if (omContext.warguments.sleep != 0) {
                usleep(omContext.warguments.sleep);
            }
        } else {
            randomSleep = (uint32)((rand() % (omContext.warguments.maxSleep + 
                            1 - omContext.warguments.minSleep)) + 
                            omContext.warguments.minSleep);
            usleep(randomSleep);
        }
    }
    free(buffer);
    return SUCCESS;
}


/*
 * Function     : omMgIoMmapRandomWrite
 *
 * Description  : This function performs mmap random write.
 *
 * Parameters   :
 *      1. fd       File descriptor.
 *      2. iostat   Pointer to thread specific omMgIoStat structure.
 * Return Value :
 *                      Returns SUCCESS if write was successful.
 *                      Returns FAILURE if write failed.
 */
int omMgIoMmapRandomWrite(int fd, struct omMgIoStat *iostat)
{

    uint64  offset;
    uint64 baseOffset       = iostat->start.baseOffset;
    uint64  size            = iostat->size;
    void    *file_memory    = NULL;
    char    *buffer         = NULL;
    uint64  skip            = 0;
    uint64  inpageOffset    = 0;
    uint64  pageStart       = 0;
    uint64  pageEnd         = 0;
    uint32 randomSleep;
    uint64 count = 0;
    uint64 i = 0;
    uint64 offsetMark;
    uint64 iosize;
    uint64 sizeRemaining;

    debug("%s : Baseoffset  %"PRIu64" size %"PRIu64"\n", __FUNCTION__,
            baseOffset, size);

    if (g_randomIosize) {
        if ((buffer = omMgIoFillPattern(omContext.warguments.maxBlockSize,
                        g_patternLen)) == NULL) {
            return FAILURE;
        }
    } else {
        if ((buffer = omMgIoFillPattern(omContext.warguments.blockSize,
                        g_patternLen)) == NULL) {
            return FAILURE;
        }
    }
    /* Do the I/O for (g_globalArrayIteration * g_sumOfIosizeArray) */
    offsetMark = baseOffset;
    for (count = 0; count < g_globalArrayIteration; count++) {
        for (i = 0; i != g_globalArrayCount; i++) {
            offset = g_iosizeArray[i][0] + offsetMark;
            skip = offset % g_patternLen;
            iosize = g_iosizeArray[i][1];
            pageStart = offset / PAGE_SIZE;
            pageEnd = (offset + iosize + PAGE_SIZE -1) /
                PAGE_SIZE;
            inpageOffset = offset % PAGE_SIZE;
            file_memory = mmap(0, (size_t)((pageEnd - pageStart) * PAGE_SIZE), PROT_READ |
                    PROT_WRITE, MAP_SHARED, fd,
                    (off_t)(pageStart * PAGE_SIZE));
            debug("mmap          offset %10"PRIu64" page   %10"PRIu64" size %10"
                    PRIu64"\n", offset, pageStart, iosize);
            if (file_memory == MAP_FAILED) {
                error("mmap failed ! %s\n", strerror(errno));
                if (buffer != NULL) {
                    free(buffer);
                }
                return FAILURE;
            }
            memcpy((char *)(file_memory) + inpageOffset, buffer + skip,
                    (size_t)iosize);
            munmap(file_memory, (size_t)((pageEnd - pageStart) * PAGE_SIZE));
            if (omContext.pinterval != NO_PROGRESS) {
                iostat->writeDoneCount++;
            }
            if (!g_randomSleep) {
                if (omContext.warguments.sleep != 0) {
                    usleep(omContext.warguments.sleep);
                }
            } else {
                randomSleep = (uint32)((rand() % (omContext.warguments.maxSleep + 
                               1 - omContext.warguments.minSleep)) +
                               omContext.warguments.minSleep);
                usleep(randomSleep);
            }
        }
        offsetMark += g_offsetMark;
    }
    sizeRemaining = (size - (g_globalArrayIteration * g_sumOfIosizeArray));
    if (sizeRemaining != 0) {
        for (i = 0; i < g_globalSubArrayCount; i++) {
            offset =  g_iosizeSubArray[i][0] + baseOffset;
            iosize =  g_iosizeSubArray[i][1];
            skip = offset % g_patternLen;
            pageStart = offset / PAGE_SIZE;
            pageEnd = (offset + iosize + PAGE_SIZE -1) /
                        PAGE_SIZE;
            inpageOffset = offset % PAGE_SIZE;
            file_memory = mmap(0, (size_t)((pageEnd - pageStart) * PAGE_SIZE), PROT_READ |
                             PROT_WRITE, MAP_SHARED, fd, (off_t)(pageStart * PAGE_SIZE));
            debug("mmap          offset %10"PRIu64" page   %10"PRIu64" size %10"
                    PRIu64"\n", offset, pageStart, iosize);
            if (file_memory == MAP_FAILED) {
                error("mmap failed ! %s\n", strerror(errno));
                if (buffer != NULL) {
                    free(buffer);
                }
                return FAILURE;
            }
            memcpy((char *)(file_memory) + inpageOffset, buffer + skip,
                    (size_t)iosize);
            munmap(file_memory, (size_t)((pageEnd - pageStart) * PAGE_SIZE));
            if (omContext.pinterval != NO_PROGRESS) {
                iostat->writeDoneCount++;
            }
            if (!g_randomSleep) {
                if (omContext.warguments.sleep != 0) {
                    usleep(omContext.warguments.sleep);
                }
            } else {
                randomSleep = (uint32)((rand() % (omContext.warguments.maxSleep + 
                                1 - omContext.warguments.minSleep)) +
                                omContext.warguments.minSleep);
                usleep(randomSleep);
            }
            sizeRemaining -= iosize;
        }
    }
    if (g_firstThreadExtraSize != 0) {
        offset = g_sumOfIosizeSubArray + (g_globalArrayIteration *
                    g_sumOfIosizeArray);
        skip = offset % g_patternLen;
        pageStart = offset / PAGE_SIZE;
        pageEnd = (offset + g_firstThreadExtraSize + PAGE_SIZE -1) /
            PAGE_SIZE;
        inpageOffset = offset % PAGE_SIZE;
        file_memory = mmap(0, (size_t)((pageEnd - pageStart) * PAGE_SIZE), PROT_READ |
                PROT_WRITE, MAP_SHARED, fd,
                (off_t)(pageStart * PAGE_SIZE));
        debug("mmap          offset %10"PRIu64" page   %10"PRIu64" size %10"
                PRIu64"\n", offset, pageStart, iosize);
        if (file_memory == MAP_FAILED) {
            error("mmap failed ! %s\n", strerror(errno));
            if (buffer != NULL) {
                free(buffer);
            }
            return FAILURE;
        }
        memcpy((char *)(file_memory) + inpageOffset, buffer + skip,
                (size_t)g_firstThreadExtraSize);
        munmap(file_memory, (size_t)((pageEnd - pageStart) * PAGE_SIZE));
        if (!g_randomSleep) {
            if (omContext.warguments.sleep != 0) {
                usleep(omContext.warguments.sleep);
            }
        } else {
            randomSleep = (uint32)((rand() % (omContext.warguments.maxSleep +
                           1 - omContext.warguments.minSleep)) +
                           omContext.warguments.minSleep);
            usleep(randomSleep);
        }
    }
    free(buffer);
    return SUCCESS;
}


/*
 * Function     : omMgIoMmapRead
 *
 * Description  : This function performs mmap read.
 *
 * Parameters   :
 *      1. fd       File descriptor.
 *      2. iostat   Pointer to thread specific omMgIoStat structure.
 * Return Value :
 *                  Calls Mmap read function according to the sequence.
 *                  Returns FAILURE otherwise.
 */
int omMgIoMmapRead(int fd, struct omMgIoStat *iostat)
{

    debug("%s\n", __FUNCTION__);

    if (omContext.sequence == IO_SEQUENCE) {
        return fops.readMmapSeq(fd, iostat);
    } else if (omContext.sequence == IO_RANDOM) {
        return fops.readMmapRandom(fd, iostat);
    }
    return FAILURE;
}


/*
 * Function     : omMgIoBufferedRead
 *
 * Description  : This function performs buffered read.
 *
 * Parameters   :
 *      1. fd      File descriptor.
 *      2. iostat   Pointer to thread specific omMgIoStat structure.
 * Return Value :
 *                 Calls Buffered I/O read function according to the sequence.
 *                 Returns FAILURE otherwise.
 */
int omMgIoBufferedRead(int fd, struct omMgIoStat *iostat)
{

    debug("%s\n", __FUNCTION__);

    if (omContext.sequence == IO_SEQUENCE) {
        return fops.readBufferedSeq(fd, iostat);
    } else if (omContext.sequence == IO_RANDOM) {
        return fops.readBufferedRandom(fd, iostat);
    }
    return FAILURE;
}


/*
 * Function     : omMgIoDirectRead
 *
 * Description  : This function performs direct read.
 *
 * Parameters   :
 *      1. fd       File descriptor.
 *      2. iostat   Pointer to thread specific omMgIoStat structure.
 * Return Value :
 *                  Calls Direct I/O read function according to the sequence.
 *                  Returns FAILURE otherwise.
 */
int omMgIoDirectRead(int fd, struct omMgIoStat *iostat)
{

    debug("%s\n", __FUNCTION__);

    if (omContext.sequence == IO_SEQUENCE) {
        return fops.readDirectSeq(fd, iostat);
    } else if (omContext.sequence == IO_RANDOM) {
        return fops.readDirectRandom(fd, iostat);
    }
    return FAILURE;
}


/*
 * Function     : omMgIoMmapWrite
 *
 * Description  : This function performs mmap write.
 *
 * Parameters   :
 *      1. fd       File descriptor.
 *      2. iostat   Pointer to thread specific omMgIoStat structure.
 * Return Value :
 *                  Calls Mmap write function according to the sequence.
 *                  Returns FAILURE otherwise.
 */
int omMgIoMmapWrite(int fd, struct omMgIoStat *iostat)
{

    debug("%s\n", __FUNCTION__);

    if (omContext.sequence == IO_SEQUENCE) {
        return fops.writeMmapSeq(fd, iostat);
    } else if (omContext.sequence == IO_RANDOM) {
        return fops.writeMmapRandom(fd, iostat);
    }
    return FAILURE;
}


/*
 * Function     : omMgIoDirectWrite
 *
 * Description  : This function performs direct write.
 *
 * Parameters   :
 *      1. fd       File descriptor.
 *      2. iostat   Pointer to thread specific omMgIoStat structure.
 * Return Value :
 *                  Calls Direct I/O write function according to the
 *                  sequence.
 *                  Returns FAILURE otherwise.
 */
int omMgIoDirectWrite(int fd, struct omMgIoStat *iostat)
{

    debug("%s\n", __FUNCTION__);

    if (omContext.sequence == IO_SEQUENCE) {
        return fops.writeDirectSeq(fd, iostat);
    } else if (omContext.sequence == IO_RANDOM) {
        return fops.writeDirectRandom(fd, iostat);
    }
    return FAILURE;
}


/*
 * Function     : omMgIoBufferedWrite
 *
 * Description  : This function performs buffered write.
 *
 * Parameters   :
 *      1. fd       File descriptor.
 *      2. iostat   Pointer to thread specific omMgIoStat structure.
 * Return Value :
 *                  Calls Buffered I/O write function according to the sequence.
 *                  Returns FAILURE otherwise.
 */
int omMgIoBufferedWrite(int fd, struct omMgIoStat *iostat)
{

    debug("%s\n", __FUNCTION__);

    if (omContext.sequence == IO_SEQUENCE) {
        return fops.writeBufferedSeq(fd, iostat);
    } else if (omContext.sequence == IO_RANDOM) {
        return fops.writeBufferedRandom(fd, iostat);
    }
    return FAILURE;
}


/*
 * Function     : omMgIoWrite
 *
 * Description  : This function performs write.
 *
 * Parameters   :
 *      1. fd       File descriptor.
 *      2. iostat   Pointer to thread specific omMgIoStat structure.
 * Return Value :
 *                  Calls write function according to the I/O method .
 *                  Returns FAILURE otherwise.
 */
int omMgIoWrite(int fd, struct omMgIoStat *iostat)
{

    debug("%s\n", __FUNCTION__);

    if (omContext.warguments.rwflag == IO_MMAP) {
        return omMgIoMmapWrite(fd, iostat);
    } else if (omContext.warguments.rwflag == IO_DIRECT) {
        return omMgIoDirectWrite(fd, iostat);
    } else if (omContext.warguments.rwflag == IO_BUFFERED) {
        return omMgIoBufferedWrite(fd, iostat);
    }
    return FAILURE;
}


/*
 * Function     : omMgIoRead
 *
 * Description  : This function performs read.
 *
 * Parameters   :
 *      1. fd       File descriptor.
 *      2. iostat   Pointer to thread specific omMgIoStat structure.
 * Return Value :
 *                  Calls read function according to the I/O method .
 *                  Returns FAILURE otherwise.
 */
int omMgIoRead(int fd, struct omMgIoStat *iostat)
{

    debug("%s\n", __FUNCTION__);

    if (omContext.rarguments.rwflag == IO_MMAP) {
        return omMgIoMmapRead(fd, iostat);
    } else if (omContext.rarguments.rwflag == IO_DIRECT) {
        return omMgIoDirectRead(fd, iostat);
    } else if (omContext.rarguments.rwflag == IO_BUFFERED) {
        return omMgIoBufferedRead(fd, iostat);
    }
    return FAILURE;
}


/*
 * Function    : omMgIoReportCorruption
 *
 * Description : This function reports corruption.
 */
void omMgIoReportCorruption(uint64 offset, int size, char *buffer,
        char *pattern, uint64 skip)
{
    FILE *fp;

    debug("%s\n", __FUNCTION__);

    fp = fopen(omContext.corruptionFile, "a");
    fprintf(fp,"%.8s..%.8s (%.10"PRIu64", %.10"PRIu64")\n",buffer,
            (buffer + size - 8), offset, offset + size);
    fclose(fp);
}


/*
 * Function     :    omMgIoVerifyBlock
 *
 * Description  :    This function verifies block read
 *
 * Parameters   :
 *      1. offset   Offset with file.
 *      2. buffer   Read buffer
 * Return Value :
 *                  Returns SUCCESS if the block is verified.
 *                  Returns FAILURE if the verification failed.
 *
 */
int omMgIoVerifyBlock(uint64 offset, char *buffer, uint64 iosize)
{
    uint64 skip =0;

    debug("%s\n", __FUNCTION__);

    skip = offset % g_patternLen;
    if (memcmp(buffer, g_patternBuffer + skip, (size_t)iosize)
            == 0) {
        return SUCCESS;
    } else {
        #ifdef Windows
        pthread_mutex_init(&g_mutex);
        #endif
        pthread_mutex_lock(&g_mutex);
        omMgIoPerf.failedVerifyOps++;
        omMgIoReportCorruption(offset, (int)iosize , buffer,
                g_patternBuffer, skip);
        pthread_mutex_unlock(&g_mutex);
        #ifdef Windows
        pthread_mutex_destroy(&g_mutex); 
        #endif
        return FAILURE;
    }
}


/*
 * Function     : omMgIoVerifyInit
 *
 * Description  : This function intializes verification structures.
 *
 * Parameters   :
 * Return Value :
 *                  Returns SUCCESS if the verification structures
 *                  intilizes successfully.
 *                  Returns FAILURE  if the verification structure
 *                  intilaization fails.
 */
int omMgIoVerifyInit()
{

    debug("%s\n", __FUNCTION__);
    if (g_verifyInitialised == TRUE) {
        return SUCCESS;
    }

    if (g_randomIosize) {
        if (omContext.rarguments.maxBlockSize <= g_patternLen) {
            if ((g_patternBuffer = omMgIoCommonAllocateBuffer(g_patternLen *
                            sizeof(char) * 2))
                    == NULL) {
                return FAILURE;
            }
            omMgIoFillBuffer(g_patternBuffer, g_pattern, g_patternLen * 2);
        } else {
            if ((g_patternBuffer = omMgIoCommonAllocateBuffer(
                                   (int)(omContext.rarguments.maxBlockSize *
                                    sizeof(char) * 2))) == NULL) {
                return FAILURE;
            }
            omMgIoFillBuffer(g_patternBuffer, g_pattern,
                    omContext.rarguments.maxBlockSize * 2);
        }
    } else {
        if (omContext.rarguments.blockSize <= g_patternLen) {
            if ((g_patternBuffer = omMgIoCommonAllocateBuffer(g_patternLen *
                            sizeof(char) * 2))
                    == NULL) {
                return FAILURE;
            }
            omMgIoFillBuffer(g_patternBuffer, g_pattern, g_patternLen * 2);
        } else {
            if ((g_patternBuffer = omMgIoCommonAllocateBuffer((int)(
                            omContext.rarguments.blockSize *
                            sizeof(char) * 2))) == NULL) {
                return FAILURE;
            }
            omMgIoFillBuffer(g_patternBuffer, g_pattern,
                    omContext.rarguments.blockSize * 2);
        }
    }
    omMgIoPerf.failedVerifyOps = 0;
    snprintf(omContext.corruptionFile,sizeof(omContext.corruptionFile),
                                            "corruption_log_%d", getpid());
    debug("using corruption log file %s\n", omContext.corruptionFile);
    g_verifyInitialised = TRUE;
    return SUCCESS;
}


/*
 * Function    : omMgExecuteTcThread
 *
 * Description : This function executes the thead.
 */
void *omMgExecuteTcThread(void *argument)
{
    struct omMgIoStat *omMgIoStat = argument;
    int16 opcode;
    int retRead;
    int retWrite;

    debug("%s\n", __FUNCTION__);

    opcode = omContext.opcode;
    if (opcode == OP_WRITE) {
        if (fops.openOutput(&(omMgIoStat->outFileDesc)) == FAILURE) {
            exit(-1);
        }
        omMgIoStat->writeStartTime = omMgTimeElapsed();
        retWrite = omMgIoWrite(omMgIoStat->outFileDesc, omMgIoStat);
        if (retWrite == FAILURE) {
            g_status = FAILURE;
            error("Write failed\n");
            exit(FAILURE);
        }
        omMgIoStat->writeEndTime = omMgTimeElapsed();
        close(omMgIoStat->outFileDesc);
        if (omContext.verify == TRUE) {
            omContext.rarguments.offset =  omContext.warguments.offset;
            omContext.rarguments.count  =  omContext.warguments.count;
            omContext.rarguments.size   =  omContext.warguments.size;
            omContext.rarguments.blockSize = omContext.warguments.blockSize;
            omContext.rarguments.minBlockSize =
                omContext.warguments.minBlockSize;
            omContext.rarguments.maxBlockSize =
                omContext.warguments.maxBlockSize;
            omContext.rarguments.minSleep = omContext.warguments.minSleep;
            omContext.rarguments.maxSleep = omContext.warguments.maxSleep;
            if (g_verifyInitialised == FALSE) {
                debug("Initialising the verification structures\n");
                #ifdef Windows
                pthread_mutex_init(&g_mutex);
                #endif
                pthread_mutex_lock(&g_mutex);
                omMgIoVerifyInit();
                pthread_mutex_unlock(&g_mutex);
                #ifdef Windows
                pthread_mutex_destroy(&g_mutex); 
                #endif
            }
            strcpy(omContext.inputFile, omContext.outputFile);
            if (fops.openInput(&(omMgIoStat->inpFileDesc)) == FAILURE) {
                exit(-1);
            }
            omMgIoStat->readStartTime = omMgTimeElapsed();
            retRead = omMgIoRead(omMgIoStat->inpFileDesc, omMgIoStat);
            if (retRead == FAILURE) {
                g_status = FAILURE;
                error("Read failed\n");
                exit(FAILURE);
            }
            omMgIoStat->readEndTime = omMgTimeElapsed();
            close(omMgIoStat->inpFileDesc);
        }
    } else {
        if (fops.openInput(&(omMgIoStat->inpFileDesc)) == FAILURE) {
            exit(-1);
        }
        if (g_verifyInitialised == FALSE) {
            debug("Initialising the verification structures\n");
            #ifdef Windows
            pthread_mutex_init(&g_mutex);
            #endif
            pthread_mutex_lock(&g_mutex);
            omMgIoVerifyInit();
            pthread_mutex_unlock(&g_mutex);
            #ifdef Windows
            pthread_mutex_destroy(&g_mutex); 
            #endif
        }
        omMgIoStat->readStartTime = omMgTimeElapsed();
        retRead = omMgIoRead(omMgIoStat->inpFileDesc, omMgIoStat);
        if (retRead == FAILURE) {
            g_status = FAILURE;
            error("Read failed\n");
            exit(FAILURE);
        }
        omMgIoStat->readEndTime = omMgTimeElapsed();
        close(omMgIoStat->inpFileDesc);
    }
    return NULL; /* to avoid warning */
}


/*
 * Function    : omMgExecuteTc
 *
 * Description : This function sets up the enviroment for threads.
 */
void omMgExecuteTc(void)
{
    pthread_t threads[16];
    int threadCntr          = 0;
    uint64 offset           = 0;
    int noOfThreads         = 0;
    int blocksPerThread     = 0;
    uint64 blocksExtra      = 0;
    uint64 fileSize         = 0;
    uint64 sizePerThread    = 0;
    uint64 sizeExtra        = 0;
    uint64 directIoOps      = 0;
    uint64 i                = 0;
    int fd;
    uint64 directIoOpsPerThread;
    uint64 directIoOpsExtra;

    debug("%s\n", __FUNCTION__);

    if (omContext.opcode == OP_WRITE) {
        fd = open(omContext.outputFile, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG |
                S_IRWXO);
        if (fd == -1) {
            error("Could not open the output file. %s\n", strerror(errno));
            exit(-1);
        }
        debug("Marking the end of the file at %"PRIu64"\n", (
                    (omContext.warguments.size * omContext.sparseFactor) +
                    omContext.warguments.offset) - 1);
        if (lseek(fd, (off_t)(((omContext.warguments.size * omContext.sparseFactor) +
                  omContext.warguments.offset) - 1), SEEK_SET) == -1) {
            error("lseek failed \n", strerror(errno));
            exit(ESPIPE);
        }
        if (write(fd, "", 1) == -1) {
            error("write failed %s exiting...\n", strerror(errno));
            exit(-1);
        }
        close(fd);
        /* Get the sector size for directio. */
        g_sectorSize = omMgIoGetSectorSize(omContext.outputFile);
        /* Calculate the size for each thread for writing. */
        noOfThreads = omContext.numWriteThreads;
        offset = omContext.warguments.offset;
        if (g_randomIosize) {
            sizePerThread = omContext.warguments.size / noOfThreads;
            sizeExtra = omContext.warguments.size % noOfThreads;
        } else {
            if (omContext.warguments.rwflag == IO_DIRECT) {
                directIoOps = omContext.warguments.size / g_sectorSize;
                directIoOpsPerThread = directIoOps / noOfThreads;
                directIoOpsExtra = directIoOps % noOfThreads;
                sizePerThread = directIoOpsPerThread *
                                omContext.warguments.blockSize;
                sizeExtra = directIoOpsExtra * omContext.warguments.blockSize;
            } else {
                blocksPerThread = (int)omContext.warguments.count / noOfThreads;
                blocksExtra = (omContext.warguments.count % noOfThreads);
                sizePerThread = blocksPerThread *
                                 omContext.warguments.blockSize;
                sizeExtra = blocksExtra * omContext.warguments.blockSize;
            }
        }
        createGlobalArray(omContext.warguments.size);
    } else {
        /* Get the sector size for directio. */
        g_sectorSize = omMgIoGetSectorSize(omContext.inputFile);
        /* Calculate the size for each thread for reading. */
        noOfThreads = omContext.numReadThreads;
        offset = omContext.rarguments.offset;
        if (g_randomIosize) {
            sizePerThread = omContext.rarguments.size / noOfThreads;
            sizeExtra = omContext.rarguments.size % noOfThreads;
        } else {
            if (omContext.rarguments.rwflag == IO_DIRECT) {
                directIoOps = omContext.rarguments.size / g_sectorSize;
                directIoOpsPerThread = directIoOps / noOfThreads;
                directIoOpsExtra = directIoOps % noOfThreads;
                sizePerThread = directIoOpsPerThread *
                                omContext.rarguments.blockSize;
                sizeExtra = directIoOpsExtra * omContext.rarguments.blockSize;
            }
            blocksPerThread = (int)omContext.rarguments.count / noOfThreads;
            blocksExtra = (omContext.rarguments.count % noOfThreads);
            sizePerThread = blocksPerThread * omContext.rarguments.blockSize;
            sizeExtra = blocksExtra * omContext.rarguments.blockSize;
        }
        createGlobalArray(omContext.rarguments.size);
        if (fops.openInput(&fd) == FAILURE) {
            exit(-1);
        }
        if ((fileSize = lseek(fd, 0, SEEK_END)) == -1) {
            error("lseek failed \n", strerror(errno));
            exit(ESPIPE);
        }
        if (((omContext.rarguments.size * omContext.sparseFactor) +
                    omContext.rarguments.offset) != fileSize) {
            error("The given size[%"PRIu64"] do not match the original file"
                    "size[%"PRIu64"]\n",((omContext.rarguments.size *
                            omContext.sparseFactor) +
                        omContext.rarguments.offset),
                    fileSize);
            exit(EINVAL);
        }
    }
    while (noOfThreads > threadCntr) {
        if (sizeExtra != 0) {
            omMgIoStats[threadCntr].size = sizePerThread + sizeExtra;
        } else {
            omMgIoStats[threadCntr].size = sizePerThread;
        }
        if (omContext.sequence == IO_RANDOM) {
            omMgIoStats[threadCntr].start.baseOffset = offset;
            debug("Thread %2d offset  %10"PRIu64" size %10"PRIu64"\n",
                    threadCntr, omMgIoStats[threadCntr].start.baseOffset,
                    omMgIoStats[threadCntr].size);
        } else {
            omMgIoStats[threadCntr].start.offset = offset;
            debug("Thread %2d offset %10"PRIu64" size %10"PRIu64"\n",
                    threadCntr, omMgIoStats[threadCntr].start.offset,
                    omMgIoStats[threadCntr].size);
        }
        offset += omMgIoStats[threadCntr].size * omContext.sparseFactor;
        if ((pthread_create(&threads[threadCntr], NULL, omMgExecuteTcThread,
                        (void *)&omMgIoStats[threadCntr])) != 0) {
            error("Thread creation failed %s \n", strerror(errno));
            exit(EINVAL);
        }
        threadCntr++;
        sizeExtra = 0;
    }
    while (threadCntr) {
        pthread_join(threads[threadCntr -1], NULL);
        threadCntr--;
    }
    if (omMgIoPerf.failedVerifyOps != 0) {
        error("Varification of one or more block failed failed \n");
        exit(-1);
    }
    if (g_status == SUCCESS) {
        omMgCollectIoStats();
        omMgReportResults();
    }
    if (g_patternBuffer != NULL) {
        free(g_patternBuffer);
    }
    for (i = 0; i < g_globalArrayCount; i++) {
        free(g_iosizeArray[i]);
    }
    for (i = 0; i < g_globalSubArrayCount; i++) {
        free(g_iosizeSubArray[i]);
    }
    if (g_iosizeSubArray != NULL) {
        free(g_iosizeSubArray);
    }
    if (g_iosizeArray != NULL) {
        free(g_iosizeArray);
    }
    debug("Processing Finished!!!!!!!!!!!!\n");
    exit(SUCCESS);
}
