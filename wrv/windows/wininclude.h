/*********************************************************************

Filename: wininclude.h

Description: Windows include file

Copyright (c) 1998-1999 Omneon Video Networks (TM)

OMNEON VIDEO NETWORKS CONFIDENTIAL

**********************************************************************/
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#ifndef __wininclude__
#define __wininclude__

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <tchar.h>
#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <malloc.h>
#include <process.h>
#include "win32.h"
#include "getopt.h"

#endif /* __wininclude__ */
