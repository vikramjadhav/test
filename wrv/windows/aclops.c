/*******************************************************************************
 *
 * Filename     : aclops.c
 *
 * Description  : This file contains functions for ACL operations.
 * Copyright (c) 1998-1999 Omneon Video Networks (TM)
 *
 * OMNEON VIDEO NETWORKS CONFIDENTIAL
 *
 ******************************************************************************/
#include "win32.h"

/*  Function    :   FindGroup
 *  Description :   Find a group name associated with the owning user
 *                  of the current process
 *  Parameters  :   1.GroupNumber   : Represents Group NUmber
 *                  2.GroupName     : Holds GroupName
 *                  3.dwGroupName   : Holds length for the group Name
 *  Return Value :  none
 */
static VOID FindGroup(DWORD dwGroupNumber, LPTSTR lpGroupName,
                      DWORD dwGroupName)
{
    HANDLE  hToken;
    DWORD   dwTISize;
    DWORD   dwAcctSize = dwGroupName;
    TCHAR   szRefDomain [DOM_SIZE];
    DWORD   dwRefDomCnt = DOM_SIZE;
    SID_NAME_USE GroupSidType  = SidTypeGroup;
    TOKEN_GROUPS TokenG[20]; /* need some space for this. */

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken))
        ReportError("OpenProcessToken error\n");
    if (!GetTokenInformation (hToken, TokenGroups,
                &TokenG, sizeof (TokenG), &dwTISize)) {
        ReportError("GetTokenInfo error\n");
    }
    /* The groups entered are as follows:
       0    -    None
       1    -    Everyone
       2    -    The first non-trivial group
       3, ..-    Keep lobOking up to the dwCount, which is part
       of the structure  */
    if (!LookupAccountSid(NULL, TokenG[0].Groups[dwGroupNumber].Sid,
                lpGroupName, &dwAcctSize, szRefDomain, &dwRefDomCnt,
                &GroupSidType))
        ReportError("Error lobOking up AcdwCount Name\n");
    return;
}


/*  Function    :   InitializeUnixSA
 *  Description :   It allocates structure and sets the UNIX style permission
 *                  - rwx rwx rwx.
 *  Parameters  :
 *                  1.UnixPerms         : Holds unix style permissions for file
 *                                      (e.g.744)
 *                  2.UsrNam            : Holds UserName
 *                  3.GrpNam            : Holds GroupName
 *                  4.AllowedAceMasks   : pointer to allowed acl list
 *                  5.DeniedAceMasks       : pointer to denied acl list
 *                  6.pHeap             : pointer to heap which is destroyed by
 *                                      calling function.
 * Return Value :   Return a pointer to a security attributes structure and a
 *                  pointer to a heap, which can be destroyed by the calling
 *                  function to release all the data structres when it is
 *                  finished with the security structure.
 */
LPSECURITY_ATTRIBUTES InitializeUnixSA(DWORD dwUnixPerms, LPTSTR lpUsrNam,
                                       LPTSTR lpGrpNam,
                                       LPDWORD lpdwAllowedAceMasks,
                                       LPDWORD lpdwDeniedAceMasks,
                                       LPHANDLE lpHeap)
{
    HANDLE SAHeap = HeapCreate (HEAP_GENERATE_EXCEPTIONS, 0, 0);
    /*  Several memory allocations are necessary to build the SA
        and they are all constructed in this heap. The structures
are:
1.  Security Attributes itself
2.  Security Descriptor
3.  SIDs (for user, group, and everyone)
4.  Group name if it is not supplied as the "GrpNam" parameter
This memory MUST be available to the calling program and must
not be allocated on the stack of this function.
     */
    PACL pAcl                       = NULL;
    BOOL bSuccess, bOk              = TRUE;
    LPSECURITY_ATTRIBUTES pSA       = NULL;
    PSECURITY_DESCRIPTOR pSD        = NULL;
    DWORD dwIBit, dwISid, dwUsrCnt  = ACCT_NAME_SIZE;
    /* Various tables of User, Group, and Everyone Names, SIDs,
       and so on for use first in LookupAccountName and SID creation. */
    LPTSTR  lpGrpNms [3]            = {"", "", _T ("Everyone")};
    PSID    pSidTable [3]           = {NULL, NULL, NULL};
    SID_NAME_USE sNamUse []         = {SidTypeUser, SidTypeGroup,
        SidTypeWellKnownGroup};
    TCHAR szRefDomain [3] [DOM_SIZE];
    DWORD dwRefDomCnt [3]           = {DOM_SIZE, DOM_SIZE, DOM_SIZE};
    DWORD dwSidCnt [3]              = {SID_SIZE, SID_SIZE, SID_SIZE};

    *lpHeap = SAHeap;
    /*  The heap is enabled for HEAP_GENERATE_EXCEPTIONS, so the flag
        is not set in the individual HeapAlloc calls */
    pSA = HeapAlloc(SAHeap, 0, sizeof(SECURITY_ATTRIBUTES));
    pSA->nLength = sizeof(SECURITY_ATTRIBUTES);
    pSA->bInheritHandle = FALSE; /* Programmer can set this later. */

    pSD = HeapAlloc(SAHeap, 0, sizeof(SECURITY_DESCRIPTOR));
    pSA->lpSecurityDescriptor = pSD;
    if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) {
        ReportError("InitializeSecurityDescriptor:\n");
    }
    /* Set up the table names for the user and group.
       Then get a SID for User, Group, and Everyone. */

    lpGrpNms[0] = lpUsrNam;
    if (lpGrpNam == NULL || _tcslen(lpGrpNam) == 0) {
        /*  No group name specified. Get the user's primary group. */
        /*  Allocate a buffer for the group name */
        lpGrpNms[1] = HeapAlloc (SAHeap, 0, ACCT_NAME_SIZE);
        FindGroup (2, lpGrpNms [1], ACCT_NAME_SIZE);
    } else {
        lpGrpNms[1] = lpGrpNam;
    }

    /* LobOk up the three names, creating the SIDs. */
    for (dwISid = 0; dwISid < 3; dwISid++) {
        pSidTable [dwISid] = HeapAlloc(SAHeap, 0, SID_SIZE);
        bOk = bOk && LookupAccountName(NULL, lpGrpNms [dwISid],
                pSidTable [dwISid], &dwSidCnt [dwISid],
                szRefDomain [dwISid], &dwRefDomCnt [dwISid], &sNamUse [dwISid]);
    }
    if (!bOk) {
        ReportError("LobOkupAccntName Error\n");
    }
    /* Set the security descriptor owner & group SIDs. */
    if (!SetSecurityDescriptorOwner(pSD, pSidTable [0], FALSE)) {
        ReportError("SetSecurityDescriptorOwner Error\n");
    }
    if (!SetSecurityDescriptorGroup(pSD, pSidTable [1], FALSE)) {
        ReportError("SetSecurityDescriptorGroup Error\n");
    }
    /* Allocate a structure for the ACL. */
    pAcl = HeapAlloc(SAHeap, 0, ACL_SIZE);
    /* Initialize an ACL. */
    if (!InitializeAcl(pAcl, ACL_SIZE, ACL_REVISION)) {
        ReportError("InitializeAcl Error\n");
    }
    /* Add all the ACEs. Scan the permission bits, adding an allowed ACE when
       the bit is set and a denied ACE when the bit is reset. */
    bSuccess = TRUE;
    for (dwIBit = 0; dwIBit < 9; dwIBit++) {
      if ((dwUnixPerms >> (8 - dwIBit) & 0x1) != 0 &&
                lpdwAllowedAceMasks[dwIBit % 3] != 0) {
            bSuccess = bSuccess && AddAccessAllowedAce (pAcl, ACL_REVISION,
                    lpdwAllowedAceMasks [dwIBit % 3], pSidTable [dwIBit / 3]);
        } else if (lpdwDeniedAceMasks[dwIBit % 3] != 0) {
            bSuccess = bSuccess && AddAccessDeniedAce (pAcl, ACL_REVISION,
                    lpdwDeniedAceMasks [dwIBit % 3], pSidTable [dwIBit / 3]);
        }
    }
    /* Add a final deny all to everyone ACE */
    bSuccess = bSuccess && AddAccessDeniedAce (pAcl, ACL_REVISION,
            STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL, pSidTable[2]);
    if (!bSuccess) {
        ReportError("AddAce Error\n");
    }
    if (!IsValidAcl (pAcl)) {
        ReportError("created bad acl\n");
    }
    /* The ACL is now complete. Associate it with the security descriptor. */
    if (!SetSecurityDescriptorDacl (pSD, TRUE, pAcl, FALSE)) {
        ReportError("SetSecurityDescriptorDacl\n");
    }
    if (!IsValidSecurityDescriptor (pSD)) {
        ReportError("created bad SD\n");
        if (SAHeap != NULL)
            HeapDestroy (SAHeap);
        *lpHeap = NULL;
        pSA = NULL;
    }
    return pSA;
}


/*  Function    :   ConvertUnixModeToWinSecAttr
 *  Description :   returns the pointer to sa corresponding to unix
 *                  file permissions (-rwxrwxrwx)
 *  Paramaters  :   1. hSecHesp    :    handle to heap memory
 *                  2. Mode          :    holds unix style mode
 *  Return Value:   on Success file descriptor
 *                  on error returns null
 */
LPSECURITY_ATTRIBUTES ConvertUnixModeToWinSecAttr(HANDLE hSecHeap, DWORD Mode)
{
    DWORD   dwUsrCnt = ACCT_NAME_SIZE;
    TCHAR   szUsrNam[ACCT_NAME_SIZE];
    LPTSTR lpGroupName = NULL;
    /* Array of rights settings in "UNIX order".
     * There are distinct arrays for the allowed and denied
     * ACE masks so that SYNCHRONIZE is never denied.
     * This is necessary as both FILE_GENERIC_READ and FILE_GENERIC_WRITE
     * require SYNCHRONIZE access.
     * For more information, see:
     */
    DWORD lpdwAllowedAceMasks [] =
    {FILE_GENERIC_READ, FILE_GENERIC_WRITE, FILE_GENERIC_EXECUTE};
    DWORD lpdwDeniedAceMasks [] =
    {~FILE_GENERIC_READ & ~SYNCHRONIZE, ~FILE_GENERIC_WRITE & ~SYNCHRONIZE,
        ~FILE_GENERIC_EXECUTE & ~SYNCHRONIZE};

    if (!GetUserName(szUsrNam, &dwUsrCnt))
        ReportError("Failed to get user name\n");
    return InitializeUnixSA(Mode, szUsrNam, lpGroupName, lpdwAllowedAceMasks,
            lpdwDeniedAceMasks, &hSecHeap);
}
