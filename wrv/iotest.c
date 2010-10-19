/*******************************************************************************

Filename: iotest.c

Description: Parses validates the command line options. Also contains the main
             function.

Copyright (c) 1998-1999 Omneon Video Networks (TM)

OMNEON VIDEO NETWORKS CONFIDENTIAL

*******************************************************************************/

#include "iotest.h"

#define PAGE_SIZE   getpagesize()
/* Help message for the tool */

char help[] = {
    "Usage: wrv.fs <options>\n"
    "-o|--output  [/path/to/filename|-]     Destination for writing data\n"
    "                                       If specified as '-', the \n"
    "                                       the file is stdout\n"
    "-i|--input   [/path/to/filename|-]     Source for reading data\n"
    "                                       If specified as '-', the \n"
    "                                       the file is stdin\n"
    "-V|--verify                            Enable or disable the algorithmic\n"
    "                                       verification of data\n"
    "-w|--write offset=[],count=[],iosize=[],miniosize=[],maxiosize=[] \n"
    "                                       size=[],sleep=[],minsleep=[]\n"
    "                                       maxsleep=[] \n"
    "-r|--read  offset=[],count=[],iosize=[],miniosize=[],maxiosize=[] \n"
    "                                       size=[],sleep=[],minsleep=[]\n"
    "                                       maxsleep=[]\n"
    "-y|--writethreads [count]              Number of threads to write\n"
    "                                       data. default 1\n"
    "-x|--readthreads [count]               Number of threads to read\n"
    "                                       data. default 1\n"
    "-q|--sequence [random|seq]             Random/sequential\n"
    "                                       reading/writing\n"
    "-t|--type [sparse|non-sparse]          Sparse-ness of a file to be\n"
    "                                       written/read\n"
    "-P|--pattern [string]                  Data-pattern to use in the\n"
    "                                       read/write and verification\n"
    "-F|--Pattern [filename]                Data-pattern to use in the \n"\
    "                                       read/write and verification\n"
    "-f|--rflags  [mmap|directio|bufferedio]\n"
    "-k|--wflags  [mmap|directio|bufferedio]\n"
    "-T|--timeout [seconds]                 Timeout for the operation\n"
    "-Z|--pinterval [minutes]               Progress Interval\n"
    "-v|-vv|--verbose                       verbose flag for more information\n"
    "-s|--sparseness [factor]               Define the sparseness factor\n"
    "-S|--seed [value]                      Define the seed value to be used\n"
    "-n|--nice [value]                      Specify the nice value to be used\n"
    "-h|--help                              Help\n"
};

#define USAGE_MSG  "\tUsage: For usage information type wrv.fs -h\n\n"

struct omExecutionContext omContext;
struct omMgIoStat omMgIoStats[MAX_THREADS];
struct omMgIoPerf omMgIoPerf;

/*
 * Function     :   omMgGetPattern
 *
 * Description  :   Function to retrieve pattern.
 *
 * Parameters   :
 *      1. pattern  Contains the pattern passed at command line.
 */
static void omMgGetPattern(char *pattern)
{
    g_pattern = (char *) malloc(sizeof(char) * strlen(pattern));
    if (g_pattern == NULL) {
        error("Memory allocation for pattern failed! %s\n", strerror(errno));
        exit(ENOMEM);
    }
    g_patternLen = strlen(pattern);
    strncpy(g_pattern, pattern, g_patternLen);
    debug("The pattern length is %"PRIu32"\n", g_patternLen);
}


/*
 * Function     : omMgGetPatternFromFile
 *
 * Description  : Function to retrieve pattern from file.
 *
 * Parameters   :
 *      1. pattern  File name which contains the pattern.
 */
static void omMgGetPatternFromFile(char *pattern)
{
    int fd;
    uint64 filesize;

    if ((fd = open(pattern, O_RDONLY)) != -1) {
        filesize = lseek(fd, 0, SEEK_END);
        g_patternLen = (uint32)filesize;
        g_pattern = mmap(0, g_patternLen, PROT_READ, MAP_SHARED, fd, 0);
        if (g_pattern == MAP_FAILED) {
            error("Pattern could not be read from file! %s\n", strerror(errno));
            exit(EACCES);
        }
        close(fd);
    } else {
        error("Could not be open the pattern file! %s\n", strerror(errno));
        exit(EACCES);
    }
    debug("The pattern length is %"PRIu32"\n", g_patternLen);
}


/*
 * Function     : omMgParseRwOptions
 *
 * Description  : Function to parse the read/write arguments supplied
 *
 * Parameters   :
 *      1. options  Contains the read/write option passed at command line.
 *      2. opcode   Value of opcode.
 * Return Value :
 *                  Returns SUCCESS if all the options are parsed
 *                  Exit otherwise.
 */
int omMgParseRwOptions(char *options, int16 opcode)
{
    char *str1      = NULL;
    char *str2      = NULL;
    char *token     = NULL;
    char *subtoken  = NULL;
    char *saveptr1  = NULL;
    char *saveptr2  = NULL;
    char *arg       = NULL;
    char *val       = NULL;
    int j;
    int k;

    for (j = 1, str1 = options; ;j++, str1 = NULL) {
        token = strtok_r(str1, ",", &saveptr1);
        if (token == NULL) {
            break;
        }
        for (str2 = token, k = 0; ; str2 = NULL, k++) {
            subtoken = strtok_r(str2, "=", &saveptr2);
            if (subtoken == NULL) {
                break;
            }
            if (arg == NULL) {
                arg = subtoken;
            } else {
                val = subtoken;
            }
            if (k == 2) { /* we need only one parameter to one option*/
                error("Invalid option passed\n");
                printf("%s\n", help);
                exit(EINVAL);
                break;
            }
        }
        if (arg != NULL && val == NULL) {
                error("Invalid read/write option passed \n");
                printf("%s\n", help);
                exit(EINVAL);
        }
        if (strcmp(arg, "offset") == 0) {
            if (opcode == OP_READ) {
                omContext.rarguments.offset = atoll(val);
            } else if (opcode == OP_WRITE) {
                omContext.warguments.offset = atoll(val);
            }
        } else if (strcmp(arg, "size") == 0) {
            if (opcode == OP_READ) {
                omContext.rarguments.size = atoll(val);
            } else if (opcode == OP_WRITE) {
                omContext.warguments.size = atoll(val);
            }
        } else if (strcmp(arg, "iosize") == 0) {
            if (opcode == OP_READ) {
                omContext.rarguments.blockSize = atoll(val);
            } else if (opcode == OP_WRITE) {
                omContext.warguments.blockSize = atoll(val);
            }
        } else if (strcmp(arg, "miniosize") == 0) {
            if (opcode == OP_READ) {
                omContext.rarguments.minBlockSize = atoll(val);
            } else if (opcode == OP_WRITE) {
                omContext.warguments.minBlockSize = atoll(val);
            }
        } else if (strcmp(arg, "maxiosize") == 0) {
            if (opcode == OP_READ) {
                omContext.rarguments.maxBlockSize = atoll(val);
            } else if (opcode == OP_WRITE) {
                omContext.warguments.maxBlockSize = atoll(val);
            }
        } else if (strcmp(arg, "count") == 0) {
            if (opcode == OP_READ) {
                omContext.rarguments.count = atoll(val);
            } else if (opcode == OP_WRITE) {
                omContext.warguments.count = atoll(val);
            }
        } else if (strcmp(arg, "sleep") == 0) {
            if (opcode == OP_READ) {
                omContext.rarguments.sleep = (int64)atof(val) * 1000000;
            } else if (opcode == OP_WRITE) {
                omContext.warguments.sleep = (int64)atof(val) * 1000000;
            }
        } else if (strcmp(arg, "minsleep") == 0) {
            if (opcode == OP_READ) {
                omContext.rarguments.minSleep = (int64)atof(val) * 1000000;
            } else if (opcode == OP_WRITE) {
                omContext.warguments.minSleep = (int64)atof(val) * 1000000;
            }
        } else if (strcmp(arg, "maxsleep") == 0) {
            if (opcode == OP_READ) {
                omContext.rarguments.maxSleep = (int64)atof(val) * 1000000;
            } else if (opcode == OP_WRITE) {
                omContext.warguments.maxSleep =  (int64)atof(val) * 1000000;
            }
        } else {
            error("Invalid option passed\n");
            exit(EINVAL);
        }
        arg = NULL;
        val = NULL;
    }
    return SUCCESS;
}


/*
 * Function     : omMgParseArguments
 *
 * Description  : Function to parse the arguments
 *
 * Parameters   :
 *      1. argc :  Count as passed to the main() function on program invocation.
 *      2. argv :  Array  as passed to the main() function on program
 *      invocation.
 *
 */
void omMgParseArguments(int argc, char **argv)
{
    int next_option;

    debug("%s\n", __FUNCTION__);
    do {
        next_option = getopt_long(argc, argv, g_short_options,
                                  g_long_options, NULL);
        switch (next_option) {
        case 'h':    /* -h or --help */
            printf("%s\n", help);
            exit(0);
            break;

        case 'v':    /* -v or  --verbose */
            omContext.verbose = TRUE;
            g_verbose_level += 1;
            break;

        case 'o':    /* -o or  --output */
            if (!(omContext.opcode & OP_WRITE)) {
                omContext.opcode = omContext.opcode | OP_WRITE;
            }
            if (strncmp(optarg, "-", 1) == 0) {
                strcpy(omContext.outputFile, optarg);
                debug("Sending o/p to stdout\n");
            } else if (omMgValidateFilename(optarg) == SUCCESS) {
                strcpy(omContext.outputFile, optarg);
            } else {
                error("Error : Invalid O/P filename %s !!\n", optarg);
                exit(EINVAL);
            }
            break;

        case 'i':    /* -i or  --input */
            if (!(omContext.opcode & OP_READ)) {
                omContext.opcode = omContext.opcode | OP_READ;
            }
            debug("I/P filename %s\n", (char *)optarg);
            if (strncmp(optarg, "-", 1) == 0) {
                debug("Taking input from stdin\n");
                strcpy(omContext.inputFile, optarg);
                } else if (omMgValidateFilename(optarg) == SUCCESS) {
                strcpy(omContext.inputFile, optarg);
            } else {
                error("Error : Invalid I/P filename %s !!\n", optarg);
                exit(EINVAL);
            }
            break;

        case 'V':    /* -V or  --verify */
            omContext.verify = TRUE;
            break;

        case 's':    /* -s or  --sparseness */
            omContext.sparseFactor = atoi(optarg);
            if ( omContext.sparseFactor == 0) {
                error("The sparse factor is incorrect !!\n");
                exit(EINVAL);
            }
            break;

        case 'S':    /* -S or  --seed */
            omContext.seed = atoll(optarg);
            if (omContext.seed < 0) {
                error("The seed can not be -ve!!\n");
                exit(EINVAL);
            }
            break;

        case 'n':    /* -n or  --nice */
            omContext.nice = atoi(optarg);
            if (omContext.nice == 0) {
                exit(EINVAL);
            }
            break;

        case 'w':    /* -w or  --write */
            omMgParseRwOptions(optarg, OP_WRITE);
            if ((omContext.warguments.blockSize != 0) ||
                    (omContext.warguments.count != 0)) {
                g_randomIosize = false;
            } else {
                g_randomIosize = true;
            }
            if(!g_randomIosize) {
                if (omContext.warguments.size == 0) {
                    if ((omContext.warguments.blockSize == 0 ) &&
                            (omContext.warguments.count == 0)) {
                        error("Please specify count and block size !!\n");
                        exit(EINVAL);
                    }
                    if (omContext.warguments.count == 0) {
                        error("Please specify either count or size !!\n");
                        exit(EINVAL);
                    }
                    if (omContext.warguments.blockSize == 0) {
                        error("Please specify either iosize or size!!\n");
                        exit(EINVAL);
                    }
                    omContext.warguments.size = omContext.warguments.count *
                        omContext.warguments.blockSize;
                }
                if (omContext.warguments.size != 0) {
                    if ((omContext.warguments.blockSize == 0) &&
                            (omContext.warguments.count == 0)) {
                        omContext.warguments.blockSize = PAGE_SIZE;
                        if ((omContext.warguments.size % PAGE_SIZE) == 0) {
                            omContext.warguments.count =
                                omContext.warguments.size / PAGE_SIZE;
                        } else {
                            error("Please specify size, which is multiple \
                                    of PAGE_SIZE!!\n");
                            exit(EINVAL);
                        }
                    }
                    if ((omContext.warguments.blockSize != 0) &&
                            (omContext.warguments.count == 0)) {
                        if ((omContext.warguments.size %
                                    omContext.warguments.blockSize) == 0) {
                            omContext.warguments.count =
                                omContext.warguments.size /
                                omContext.warguments.blockSize;
                        } else {
                            error("Please specify valid iosize and size!!\n");
                            exit(EINVAL);
                        }
                    }
                    if ((omContext.warguments.blockSize == 0) &&
                            (omContext.warguments.count != 0)) {
                        if ((omContext.warguments.size %
                                    omContext.warguments.count) == 0) {
                            omContext.warguments.blockSize =
                                omContext.warguments.size /
                                omContext.warguments.count;
                        } else {
                            error("Please specify valid count and size!!\n");
                            exit(EINVAL);
                        }
                    }
                }
            } else {
                if (omContext.warguments.count != 0) {
                    error("count is not a valid option with random iosize!!\n");
                    exit(EINVAL);
                }
                if (omContext.warguments.maxBlockSize <= 0) {
                    error("Please specify valid upper limit for iosize!!\n");
                    exit(EINVAL);
                }
                if (omContext.warguments.size <= 0) {
                    error("Please specify valid total size !!\n");
                    exit(EINVAL);
                }
            }
            if ((omContext.warguments.minSleep != 0) ||
                    (omContext.warguments.maxSleep != 0)) {
                g_randomSleep = true;
            } else {
                g_randomSleep = false;
            }
            break;

        case 'r':    /* -r or  --read */
            omMgParseRwOptions(optarg, OP_READ);
            if ((omContext.rarguments.blockSize != 0) || 
                    (omContext.rarguments.count != 0)) {
                g_randomIosize = false;
            } else {
                g_randomIosize = true;
            }
            if (!g_randomIosize) {
                if (omContext.rarguments.size == 0) {
                    if ((omContext.rarguments.blockSize == 0) &&
                            (omContext.rarguments.count == 0)) {
                        error("Please specify count and block size !!\n");
                        exit(EINVAL);
                    }
                    if ((omContext.rarguments.count == 0)) {
                        error("Please specify either count or size !!\n");
                        exit(EINVAL);
                    }
                    if ((omContext.rarguments.blockSize == 0)) {
                        error("Please specify either iosize or size!!\n");
                        exit(EINVAL);
                    }
                    omContext.rarguments.size = omContext.rarguments.count
                        * omContext.rarguments.blockSize;
                }
                if (omContext.rarguments.size != 0) {
                    if ((omContext.rarguments.blockSize == 0) &&
                            (omContext.rarguments.count == 0)) {
                        omContext.rarguments.blockSize = PAGE_SIZE;
                        if ((omContext.rarguments.size % PAGE_SIZE) == 0) {
                            omContext.rarguments.count =
                                omContext.rarguments.size / PAGE_SIZE;
                        } else {
                            error("Please specify size, which is multiple \
                                    of PAGE_SIZE!!\n");
                            exit(EINVAL);
                        }
                    }
                    if ((omContext.rarguments.blockSize != 0) &&
                            (omContext.rarguments.count == 0)) {
                        if ((omContext.rarguments.blockSize %
                                    omContext.rarguments.blockSize) == 0) {
                            omContext.rarguments.count =
                                omContext.rarguments.size /
                                omContext.rarguments.blockSize;
                        } else {
                            error("Please specify valid iosize and size!!\n");
                            exit(EINVAL);
                        }
                    }
                    if ((omContext.rarguments.blockSize == 0) &&
                            (omContext.rarguments.count != 0)) {
                        if ((omContext.rarguments.size %
                                    omContext.rarguments.count) == 0) {
                            omContext.rarguments.blockSize =
                                omContext.rarguments.size /
                                omContext.rarguments.count;
                        } else {
                            error("Please specify valid count and size!!\n");
                            exit(EINVAL);
                        }
                    }
                }
            } else {
                if (omContext.rarguments.count != 0) {
                    error("count is not a valid option with random iosize!!\n");
                    exit(EINVAL);
                }
                if (omContext.rarguments.maxBlockSize <= 0) {
                    error("Please specify valid upper limit for iosize!!\n");
                    exit(EINVAL);
                }
                if (omContext.rarguments.size <= 0) {
                    error("Please specify valid total size !!\n");
                    exit(EINVAL);
                }
            }
            if ((omContext.rarguments.minSleep != 0) ||
                    (omContext.rarguments.maxSleep != 0)) {
                g_randomSleep = true;
            } else {
                g_randomSleep = false;
            }
            break;

        case 'y':    /* -y or  --wthreads */
            omContext.numWriteThreads = atoi(optarg);
            if (omContext.numWriteThreads == 0) {
                error("The number of write threads is incorrect !!\n");
                exit(EINVAL);
            }
            break;

        case 'x':    /* -x or  --rthreads */
            omContext.numReadThreads = atoi(optarg);
            if (omContext.numReadThreads == 0) {
                error("The number of read threads is incorrect !!\n");
                exit(EINVAL);
            }
            break;

        case 'q':    /* -q or  --sequence */
            if (strcmp(optarg, "seq") == 0) {
                omContext.sequence = IO_SEQUENCE;
            } else if (strcmp(optarg, "random") == 0) {
                omContext.sequence = IO_RANDOM;
            } else {
                error("Incorrect sequence option  given. See help\n");
                exit(EINVAL);
            }
            break;

        case 't':    /* -t or  --type */
            if (strcmp(optarg, "sparse") == 0) {
                omContext.type = IO_SPARSE;
            } else if (strcmp(optarg, "non-sparse") == 0) {
                omContext.type = IO_NOSPARSE;
            } else {
                error("Incorrect sparse value given. See help\n");
                exit(EINVAL);
            }
            break;

        case 'T':    /* -T or  --timeout */
            omContext.timeout = atof(optarg);
            if (omContext.timeout != NO_TIMEOUT)
                omMgCommonAddTimeout();
            break;

        case 'Z':    /* -Z or  --pinterval */
            omContext.pinterval = atof(optarg);
            if (omContext.pinterval != NO_PROGRESS)
                omMgCommonAddPinterval();
            break;

        case 'P':    /* -P or  --pattern */
            omMgGetPattern(optarg);
            break;

        case 'F':    /* -F or  --Pattern */
            omMgGetPatternFromFile(optarg);
            break;

        case 'f':    /* -f or  --rflags */
            if (strcmp(optarg, "mmap") == 0) {
                omContext.rarguments.rwflag = IO_MMAP;
            } else if (strcmp(optarg, "directio") == 0) {
                #if defined(Windows)
                    fprintf(stderr, "DirectIO not supported under Windows\n");
                    exit(EINVAL);
                #else
                    omContext.rarguments.rwflag = IO_DIRECT;
                #endif
            } else if (strcmp(optarg, "bufferedio") == 0) {
                omContext.rarguments.rwflag = IO_BUFFERED;
            } else {
                error("Incorrect read flag value given. See help\n");
                exit(EINVAL);
            }
            break;

        case 'k':    /* -k or  --wflags */
            if (strcmp(optarg, "mmap") == 0) {
                omContext.warguments.rwflag = IO_MMAP;
            } else if (strcmp(optarg, "directio") == 0) {
                omContext.warguments.rwflag = IO_DIRECT;
            } else if (strcmp(optarg, "bufferedio") == 0) {
                omContext.warguments.rwflag = IO_BUFFERED;
            } else {
                error("Incorrect write flag value given. See help\n");
                exit(EINVAL);
            }
            break;

        case '?':    /* The user specified an invalid option. */
            fprintf(stderr, "Invalid option given\n");
            fprintf(stdout, USAGE_MSG);
            exit(EINVAL);
            break;

        case -1:     /* Done with options.  */
            break;

        default:     /* Something else: unexpected.  */
            fprintf(stderr, "Invalid option given\n");
            fprintf(stdout, USAGE_MSG);
            exit(EINVAL);
            break;
        }
    } while (next_option != -1);
}


/*
 * Function    : omMgDefaultPattern
 *
 * Description : Function to initialize default pattern if pattern file or
 *               string is not specified.
 */
void omMgDefaultPattern()
{
    int i;
    char *pattern    = "";
    char *tmpPattern = "";

    debug("%s\n", __FUNCTION__);

    if (g_patternLen == 0) {
        pattern = (char *) malloc(11 * sizeof(char));
        if (pattern == NULL) {
            error("Allocation for default pattern failed. \n");
             exit(ENOMEM);
        } else {
            debug("Allocation for default pattern done successfully. \n");
        }
        tmpPattern = (char *) malloc(11 * sizeof(char));
        if (tmpPattern == NULL) {
            error("Allocation for temp pattern failed. \n");
             exit(ENOMEM);
        } else {
            debug("Allocation for temp pattern done successfully. \n");
        }
        memset(tmpPattern, '\0' , 11 * sizeof(char));
        memset(pattern, '\0' , 11 * sizeof(char)); 
        for (i = 0; i <= 10; i++) {
            sprintf(pattern, "%s%d", tmpPattern, i);
            strcpy(tmpPattern, pattern);
        }
        g_pattern = pattern;
        g_patternLen = strlen(g_pattern);
        debug("Default pattern %s\t Pattern length %"PRIu32"\n", g_pattern, g_patternLen);
    }
}


/*
 * Function    : omMgDefaultPattern
 *
 * Description : Function to validate arguments
 */
void omMgValidateArguments()
{
    debug("%s\n", __FUNCTION__);

    if (strlen(omContext.inputFile) && strlen(omContext.outputFile)) {
        error("Both input and output files are not allowed !\n");
        exit(EINVAL);
    }

    if (omContext.opcode == OP_READ) {
        if (omContext.rarguments.rwflag == IO_DIRECT) {
            if (((omContext.rarguments.blockSize % 512) != 0) ||
               ((omContext.rarguments.size % 512) != 0)) {
                error("For directio, both  offset iosize should \
                       be multiple of 512 (sector aligned) \n");
                exit(EINVAL);
            }
        }
    } else if (omContext.opcode == OP_WRITE) {
        if (omContext.warguments.rwflag == IO_DIRECT) {
            if (((omContext.warguments.blockSize % 512) != 0) ||
               ((omContext.warguments.size % 512) != 0)) {
                error("For directio, both  offset iosize should \
                       be multiple of 512 (sector aligned) \n");
                exit(EINVAL);
            }
        }
    } else {
        error("No I/O operation specified !\n");
        exit(EINVAL);
    }
    if ((strcmp(omContext.inputFile, "-") == 0) &&
        (omContext.numReadThreads > 1)) {
        error("Having more than one thread for stdin input is not allowed\n");
        exit(EINVAL);
    }

    if ((strcmp(omContext.outputFile, "-") == 0) &&
        (omContext.numWriteThreads > 1)) {
        error("Having more than one thread for stdout output is not allowed\n");
        exit(EINVAL);
    }

    if ((strcmp(omContext.outputFile, "-") == 0)) {
        if (omContext.verify == TRUE) {
            error("Verify with stdout is not allowed. \n");
            exit(EINVAL);
        }

        if (omContext.warguments.rwflag == IO_DIRECT) {
            error("Direct I/O for stdout is not allowed\n");
            exit(EINVAL);
        }
        if (omContext.warguments.rwflag == IO_MMAP) {
            error("Mmap I/O for stdout is not allowed \n");
            exit(EINVAL);
        }
        if (omContext.sequence == IO_RANDOM) {
            error("Random sequence for stdout is not allowed\n");
            exit(EINVAL);
        }
    }
    if (strcmp(omContext.inputFile, "-") == 0) {
        if (omContext.rarguments.rwflag == IO_DIRECT) {
            error("Direct I/O method for stdin is not allowed\n");
            exit(EINVAL);
        }
        if (omContext.rarguments.rwflag == IO_MMAP) {
            error("Mmap I/O for stdin is not allowed \n");
            exit(EINVAL);
        }
        if (omContext.sequence == IO_RANDOM) {
            error("Random sequence for stdin is not allowed\n");
            exit(EINVAL);
        }
    }
    if ((omContext.type == IO_NOSPARSE) && (omContext.sparseFactor > 1)) {
        error("Non-sparse cannot have sparse factor more than 1(default)!\n");
        exit(EINVAL);
    }
    if ((omContext.numReadThreads > MAX_THREADS) ||
        (omContext.numReadThreads <= 0)) {
        error("Number of read threads should be between 0 and MAX_THREADS!\n");
        exit(EINVAL);
    }
    if ((omContext.numWriteThreads > MAX_THREADS) ||
        (omContext.numWriteThreads <= 0)) {
        error("Number of write threads should be between 0 and %"PRIu32"!\n",
               MAX_THREADS);
        exit(EINVAL);
    }
    if ((omContext.warguments.rwflag == IO_DIRECT) && g_randomIosize) {
        error(" diectio io can not be used with random iosize!!");
        exit(EINVAL);
    }
    if ((omContext.rarguments.rwflag == IO_DIRECT) && g_randomIosize) {
        error(" diectio io can not be used with random iosize!!");
        exit(EINVAL);
    }
}


int main(int argc, char **argv)
{
    if (argc  <= 1) {
        printf("\n"USAGE_MSG"\n");
        exit(EINVAL);
    }
    omMgInitialiseDefaultContext();
    omMgParseArguments(argc, argv);
    omMgDefaultPattern();
    omMgValidateArguments();
    #if defined(linux) 
    omMgNiceness();
    #endif
    omMgExecuteTc();
    omMgReportResults();
    return SUCCESS;
}
