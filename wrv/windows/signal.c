/*******************************************************************************
 *
 * Filename     : signal.c
 *
 * Description  : This file contains the functions for signal operations
 * Copyright (c) 1998-1999 Omneon Video Networks (TM)
 *
 * OMNEON VIDEO NETWORKS CONFIDENTIAL
 *
 ******************************************************************************/
#include "../iotest.h"
extern int g_alarmInterval  = 0;

/*  Function    :   ProgressThread
 *  Description :   Thread routine which calls alarm sighandler periodically
 *  Parameters  :   l. argument from CreateThread
 *  Return Value:   returns 0
 */
DWORD WINAPI ProgressThread(LPVOID lpArgs)
{
    int progressPercent = 0;
/* for testing making progresss interval unit from per min to per second */
    Sleep((g_alarmInterval / 60) * 1000);
    (*sigAlarmHandler)(SIGALRM);
    return 0;
}


/*  Function    :   alarm
 *  Description :   alarm system call implementation
 *  Parameters  :   l. seconds    :    seconds after which alarm should be fired.
 *  Return Value:   returns 0
 */
unsigned int alarm(int seconds)
{
    DWORD        dwThreadIDSet;
    HANDLE        hThreadSet;

    g_alarmInterval = seconds;
    if ((hThreadSet = CreateThread(NULL, 0, ProgressThread, NULL, 0,
                    &dwThreadIDSet)) == NULL) {
        printf("CtrareThread Fail...!\n");
        exit(EXIT_FAILURE);
    }
    SetThreadPriority(hThreadSet, 2);
    return 0;
}


/*  Function    :   signalA
 *  Description :   signal system call
 *  Parameters  :   l. signum   :    signal number
 *                  2. hand     :    signal handler to accsociate with
 *                                   specified signal
 *  Return Value:   returns 0
 */
int signalA(int signum, sighandler_t hand)
{
    sigAlarmHandler = hand;
    return 0;
}
