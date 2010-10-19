/*******************************************************************************
 *
 * Filename     : errormsg.c
 *
 * Description  : This file contains functions for printing system error
 * Copyright (c) 1998-1999 Omneon Video Networks (TM)
 *
 * OMNEON VIDEO NETWORKS CONFIDENTIAL
 *
 ******************************************************************************/

#include "wininclude.h"

/*  Function    :   PrintStrings
 *  Description :   Write the messages to the output handle.
 *  Parameters  :   1. hOut: handle to output stream.
 *  Return Value:   on Success TRUE
 *                  on failure FALSE
 */
BOOL PrintStrings(HANDLE hOut, ...)
{
    DWORD    dwMsgLen;
    DWORD    dwCount;
    LPCTSTR  lpMsg = NULL;
    va_list  lpMsgList; /* Current message string. */

    va_start(lpMsgList, hOut); /* Start processing messages. */
    while ((lpMsg = va_arg(lpMsgList, LPCTSTR)) != NULL) {
        dwMsgLen = strlen (lpMsg);
        /* WriteConsole succeeds only for console handles. */
        if (!WriteConsole(hOut, lpMsg, dwMsgLen, &dwCount, NULL)
                /* Call WriteFile only if WriteConsole fails. */
                && !WriteFile (hOut, lpMsg, dwMsgLen * sizeof (TCHAR),
                    &dwCount, NULL))
            return FALSE;
    }
    va_end(lpMsgList);
    return TRUE;
}


/*  Function    :   ReportError
 *  Description :   General-purpose function for reporting system errors.
 *  Parameters  :   1. UserMessage  : holds user messages to print
 *                  2. ExitCode     : decides whether to call exit from
 *                                    process or not
 *                  3. PrintErrorMsg: decides whether to print system's
 *                                    error message or not.
 *  Return value:    none
 */
VOID ReportError(LPCTSTR lpUserMessage/*, DWORD dwExitCode,
                 BOOL bPrintErrorMsg*/)
{
    DWORD    dwErrMsgLen;
    LPTSTR    lpvSysMsg    = NULL;
    DWORD    dwLastErr    = GetLastError();
    HANDLE    hStdErr        = GetStdHandle (STD_ERROR_HANDLE);

    PrintStrings(hStdErr, lpUserMessage, NULL);
    /*if (bPrintErrorMsg) {*/
        dwErrMsgLen = FormatMessage
            (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
             NULL, dwLastErr, MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
             (LPTSTR) &lpvSysMsg, 0, NULL);
        PrintStrings(hStdErr, _T ("\n"), lpvSysMsg,
                _T ("\n"), NULL);
        /* Free the memory block containing the error message. */
        HeapFree(GetProcessHeap (), 0, lpvSysMsg);
    /*}
    if (dwExitCode > 0)
        ExitProcess(dwExitCode);
    else
        return;*/
}
