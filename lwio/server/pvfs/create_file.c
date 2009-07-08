/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright Likewise Software
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.  You should have received a copy of the GNU General
 * Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * LIKEWISE SOFTWARE MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING
 * TERMS AS WELL.  IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT
 * WITH LIKEWISE SOFTWARE, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE
 * TERMS OF THAT SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE GNU
 * GENERAL PUBLIC LICENSE, NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU
 * HAVE QUESTIONS, OR WISH TO REQUEST A COPY OF THE ALTERNATE LICENSING
 * TERMS OFFERED BY LIKEWISE SOFTWARE, PLEASE CONTACT LIKEWISE SOFTWARE AT
 * license@likewisesoftware.com
 */

/*
 * Copyright (C) Likewise Software. All rights reserved.
 *
 * Module Name:
 *
 *        create_file.c
 *
 * Abstract:
 *
 *        Likewise Posix File System Driver (PVFS)
 *
 *       Create Dispatch Routine
 *
 * Authors: Gerald Carter <gcarter@likewise.com>
 */

#include "pvfs.h"

/* Forward declarations */

static NTSTATUS
PvfsCreateFileSupersede(
    PPVFS_IRP_CONTEXT pIrpContext
    );

static NTSTATUS
PvfsCreateFileCreate(
    PPVFS_IRP_CONTEXT pIrpContext
    );

NTSTATUS
PvfsCreateFileOpen(
    PPVFS_IRP_CONTEXT pIrpContext
    );

static NTSTATUS
PvfsCreateFileOpenIf(
    PPVFS_IRP_CONTEXT pIrpContext
    );

static NTSTATUS
PvfsCreateFileOverwrite(
    PPVFS_IRP_CONTEXT pIrpContext
    );

static NTSTATUS
PvfsCreateFileOverwriteIf(
    PPVFS_IRP_CONTEXT pIrpContext
	);

typedef LONG PVFS_SET_FILE_PROPERTY_FLAGS;

#define PVFS_SET_PROP_NONE      0x00000000
#define PVFS_SET_PROP_OWNER     0x00000001
#define PVFS_SET_PROP_ATTRIB    0x00000002

static NTSTATUS
PvfsCreateFileDoSysOpen(
    IN PPVFS_IRP_CONTEXT pIrpContext,
    IN PSTR pszDiskFilename,
    IN PPVFS_CCB pCcb,
    IN PPVFS_FCB pFcb,
    IN ACCESS_MASK GrantedAccess,
    IN PVFS_SET_FILE_PROPERTY_FLAGS Flags
    );


/* Code */


/********************************************************
 * Top level driver for creating an actual file.  Splits
 * work based on the CreateDisposition.
 *******************************************************/

NTSTATUS
PvfsCreateFile(
    PPVFS_IRP_CONTEXT pIrpContext
    )
{
    NTSTATUS ntError = STATUS_UNSUCCESSFUL;
    FILE_CREATE_DISPOSITION CreateDisposition = 0;

    CreateDisposition = pIrpContext->pIrp->Args.Create.CreateDisposition;

    switch (CreateDisposition)
    {
    case FILE_SUPERSEDE:
        ntError = PvfsCreateFileSupersede(pIrpContext);
        break;

    case FILE_CREATE:
        ntError = PvfsCreateFileCreate(pIrpContext);
        break;

    case FILE_OPEN:
        ntError = PvfsCreateFileOpen(pIrpContext);
        break;

    case FILE_OPEN_IF:
        ntError = PvfsCreateFileOpenIf(pIrpContext);
        break;

    case FILE_OVERWRITE:
        ntError = PvfsCreateFileOverwrite(pIrpContext);
        break;

    case FILE_OVERWRITE_IF:
        ntError = PvfsCreateFileOverwriteIf(pIrpContext);
        break;

    default:
        ntError = STATUS_INVALID_PARAMETER;
        break;
    }
    BAIL_ON_NT_STATUS(ntError);


cleanup:
    return ntError;

error:
    goto cleanup;
}

/********************************************************
 *******************************************************/

static NTSTATUS
PvfsCreateFileSupersede(
    PPVFS_IRP_CONTEXT pIrpContext
    )
{
    NTSTATUS ntError = STATUS_UNSUCCESSFUL;
    PIRP pIrp = pIrpContext->pIrp;
    IRP_ARGS_CREATE Args = pIrp->Args.Create;
    PIO_CREATE_SECURITY_CONTEXT pSecCtx = Args.SecurityContext;
    PSTR pszFilename = NULL;
    PPVFS_CCB pCcb = NULL;
    FILE_CREATE_RESULT CreateResult = 0;
    ACCESS_MASK GrantedAccess = 0;
    BOOLEAN bFileExisted = FALSE;
    PSTR pszDirectory = NULL;
    PPVFS_FCB pFcb = NULL;
    PSTR pszDirname = NULL;
    PSTR pszRelativeFilename = NULL;
    PSTR pszDiskFilename = NULL;
    PSTR pszDiskDirname = NULL;

    /* Caller had to have asked for DELETE access */

    if (!(Args.DesiredAccess & DELETE)) {
        ntError = STATUS_CANNOT_DELETE;
        BAIL_ON_NT_STATUS(ntError);
    }

    /* Deal with the pathname */

    ntError = PvfsCanonicalPathName(&pszFilename, Args.FileName);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsFileSplitPath(&pszDirname, &pszRelativeFilename, pszFilename);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsLookupPath(&pszDiskDirname, pszDirname, FALSE);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsAllocateCCB(&pCcb);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsAcquireAccessToken(pCcb, pSecCtx);
    BAIL_ON_NT_STATUS(ntError);

    /* Check for file existence.  Remove it if necessary */

    ntError = PvfsLookupFile(
                  &pszDiskFilename,
                  pszDiskDirname,
                  pszRelativeFilename,
                  FALSE);
    bFileExisted = NT_SUCCESS(ntError);

    if (bFileExisted)
    {
        FILE_ATTRIBUTES Attributes = 0;

        ntError = PvfsAccessCheckFile(pCcb->pUserToken,
                                      pszDiskFilename,
                                      Args.DesiredAccess,
                                      &GrantedAccess);
        BAIL_ON_NT_STATUS(ntError);

        /* Check for ReadOnly bit */

        ntError = PvfsGetFilenameAttributes(pszDiskFilename, &Attributes);
        BAIL_ON_NT_STATUS(ntError);

        if (Attributes & FILE_ATTRIBUTE_READONLY) {
            ntError = STATUS_CANNOT_DELETE;
            BAIL_ON_NT_STATUS(ntError);
        }

        ntError = PvfsCheckShareMode(pszDiskFilename,
                                     Args.ShareAccess,
                                     Args.DesiredAccess,
                                     &pFcb);
        BAIL_ON_NT_STATUS(ntError);

        /* Finally remove the file */

        ntError = PvfsSysRemove(pszDiskFilename);
        BAIL_ON_NT_STATUS(ntError);

        /* Seems like this should clear the FCB from
           the open table.  Not sure */

        PvfsReleaseFCB(pFcb);
    }
    else
    {
        /* Did not exist so just make a copy of the filename */

        ntError = RtlCStringAllocatePrintf(&pszDiskFilename,
                                           "%s/%s",
                                           pszDiskDirname,
                                           pszRelativeFilename);
        BAIL_ON_NT_STATUS(ntError);
    }

    ntError = PvfsAccessCheckDir(pCcb->pUserToken,
                                 pszDirectory,
                                 Args.DesiredAccess,
                                 &GrantedAccess);
    BAIL_ON_NT_STATUS(ntError);

    /* This should actually be another check against the
       newly created SD */

    GrantedAccess = FILE_ALL_ACCESS;

    /* Can't set DELETE_ON_CLOSE for ReadOnly files */

    if (Args.CreateOptions & FILE_DELETE_ON_CLOSE)
    {
        if (Args.FileAttributes & FILE_ATTRIBUTE_READONLY) {
            ntError = STATUS_CANNOT_DELETE;
            BAIL_ON_NT_STATUS(ntError);
        }
    }

    /* This should get us a new FCB */

    ntError = PvfsCheckShareMode(pszDiskFilename,
                                 Args.ShareAccess,
                                 Args.DesiredAccess,
                                 &pFcb);
    BAIL_ON_NT_STATUS(ntError);


    ntError = PvfsCreateFileDoSysOpen(
                  pIrpContext,
                  pszDiskFilename,
                  pCcb,
                  pFcb,
                  GrantedAccess,
                  PVFS_SET_PROP_OWNER|PVFS_SET_PROP_ATTRIB);
    BAIL_ON_NT_STATUS(ntError);

    CreateResult = bFileExisted ? FILE_SUPERSEDED : FILE_CREATED;

cleanup:
    pIrp->IoStatusBlock.CreateResult = CreateResult;

    RtlCStringFree(&pszFilename);
    RtlCStringFree(&pszDirname);
    RtlCStringFree(&pszRelativeFilename);
    RtlCStringFree(&pszDiskDirname);

    return ntError;

error:
    CreateResult = bFileExisted ? FILE_EXISTS : FILE_DOES_NOT_EXIST;

    RtlCStringFree(&pszDiskFilename);

    if (pFcb) {
        PvfsReleaseFCB(pFcb);
    }

    if (pCcb) {
        PvfsReleaseCCB(pCcb);
    }

    goto cleanup;
}

/********************************************************
 *******************************************************/

static NTSTATUS
PvfsCreateFileCreate(
    PPVFS_IRP_CONTEXT pIrpContext
    )
{
    NTSTATUS ntError = STATUS_UNSUCCESSFUL;
    PIRP pIrp = pIrpContext->pIrp;
    IRP_ARGS_CREATE Args = pIrp->Args.Create;
    PIO_CREATE_SECURITY_CONTEXT pSecCtx = Args.SecurityContext;
    PSTR pszFilename = NULL;
    PPVFS_CCB pCcb = NULL;
    FILE_CREATE_RESULT CreateResult = 0;
    ACCESS_MASK GrantedAccess = 0;
    PPVFS_FCB pFcb = NULL;
    PSTR pszDirname = NULL;
    PSTR pszRelativeFilename = NULL;
    PSTR pszDiskFilename = NULL;
    PSTR pszDiskDirname = NULL;

    ntError = PvfsCanonicalPathName(&pszFilename, Args.FileName);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsFileSplitPath(&pszDirname, &pszRelativeFilename, pszFilename);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsLookupPath(&pszDiskDirname, pszDirname, FALSE);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsLookupFile(&pszDiskFilename,
                             pszDiskDirname,
                             pszRelativeFilename,
                             FALSE);
    if (ntError == STATUS_SUCCESS) {
        ntError = STATUS_OBJECT_NAME_COLLISION;
        BAIL_ON_NT_STATUS(ntError);
    }

    ntError = RtlCStringAllocatePrintf(&pszDiskFilename,
                                       "%s/%s",
                                       pszDiskDirname,
                                       pszRelativeFilename);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsAllocateCCB(&pCcb);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsAcquireAccessToken(pCcb, pSecCtx);
    BAIL_ON_NT_STATUS(ntError);

    /* Check that we can add files to the parent directory.  If
       we can, then the granted access on the file should be
       ALL_ACCESS. */

    ntError = PvfsAccessCheckDir(pCcb->pUserToken,
                                 pszDiskDirname,
                                 FILE_ADD_FILE,
                                 &GrantedAccess);
    BAIL_ON_NT_STATUS(ntError);

    /* This should actually be another check against the
       newly created SD */

    GrantedAccess = FILE_ALL_ACCESS;

    /* Can't set DELETE_ON_CLOSE for ReadOnly files */

    if (Args.CreateOptions & FILE_DELETE_ON_CLOSE)
    {
        if (Args.FileAttributes & FILE_ATTRIBUTE_READONLY) {
            ntError = STATUS_CANNOT_DELETE;
            BAIL_ON_NT_STATUS(ntError);
        }
    }

    /* Need to go ahead andcreate a share mode entry */

    ntError = PvfsCheckShareMode(pszDiskFilename,
                                 Args.ShareAccess,
                                 Args.DesiredAccess,
                                 &pFcb);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsCreateFileDoSysOpen(
                  pIrpContext,
                  pszDiskFilename,
                  pCcb,
                  pFcb,
                  GrantedAccess,
                  PVFS_SET_PROP_OWNER|PVFS_SET_PROP_ATTRIB);
    BAIL_ON_NT_STATUS(ntError);

    CreateResult = FILE_CREATED;

cleanup:
    pIrp->IoStatusBlock.CreateResult = CreateResult;

    RtlCStringFree(&pszFilename);
    RtlCStringFree(&pszDirname);
    RtlCStringFree(&pszRelativeFilename);
    RtlCStringFree(&pszDiskDirname);

    return ntError;

error:
    CreateResult = (ntError == STATUS_OBJECT_NAME_COLLISION) ?
                   FILE_EXISTS : FILE_DOES_NOT_EXIST;

    RtlCStringFree(&pszDiskFilename);

    if (pFcb) {
        PvfsReleaseFCB(pFcb);
    }

    if (pCcb) {
        PvfsReleaseCCB(pCcb);
    }

    goto cleanup;
}

/********************************************************
 *******************************************************/

NTSTATUS
PvfsCreateFileOpen(
    PPVFS_IRP_CONTEXT pIrpContext
    )
{
    NTSTATUS ntError = STATUS_UNSUCCESSFUL;
    PIRP pIrp = pIrpContext->pIrp;
    IRP_ARGS_CREATE Args = pIrp->Args.Create;
    PIO_CREATE_SECURITY_CONTEXT pSecCtx = Args.SecurityContext;
    PSTR pszFilename = NULL;
    PPVFS_CCB pCcb = NULL;
    FILE_CREATE_RESULT CreateResult = 0;
    ACCESS_MASK GrantedAccess = 0;
    PPVFS_FCB pFcb = NULL;
    PSTR pszDiskFilename = NULL;

    ntError = PvfsCanonicalPathName(&pszFilename, Args.FileName);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsLookupPath(&pszDiskFilename, pszFilename, FALSE);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsAllocateCCB(&pCcb);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsAcquireAccessToken(pCcb, pSecCtx);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsAccessCheckFile(pCcb->pUserToken,
                                  pszDiskFilename,
                                  Args.DesiredAccess,
                                  &GrantedAccess);
    BAIL_ON_NT_STATUS(ntError);

    /* Can't set DELETE_ON_CLOSE for ReadOnly files */

    if (Args.CreateOptions & FILE_DELETE_ON_CLOSE)
    {
        FILE_ATTRIBUTES Attributes = 0;

        ntError = PvfsGetFilenameAttributes(pszDiskFilename, &Attributes);
        BAIL_ON_NT_STATUS(ntError);

        if (Attributes & FILE_ATTRIBUTE_READONLY) {
            ntError = STATUS_CANNOT_DELETE;
            BAIL_ON_NT_STATUS(ntError);
        }
    }

    ntError = PvfsCheckShareMode(pszDiskFilename,
                                 Args.ShareAccess,
                                 Args.DesiredAccess,
                                 &pFcb);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsCreateFileDoSysOpen(
                  pIrpContext,
                  pszDiskFilename,
                  pCcb,
                  pFcb,
                  GrantedAccess,
                  PVFS_SET_PROP_NONE);
    BAIL_ON_NT_STATUS(ntError);

    CreateResult = FILE_OPENED;

cleanup:
    pIrp->IoStatusBlock.CreateResult = CreateResult;

    RtlCStringFree(&pszFilename);

    return ntError;

error:
    CreateResult = (ntError == STATUS_OBJECT_PATH_NOT_FOUND) ?
                   FILE_DOES_NOT_EXIST : FILE_EXISTS;

    RtlCStringFree(&pszDiskFilename);

    if (pFcb) {
        PvfsReleaseFCB(pFcb);
    }

    if (pCcb) {
        PvfsReleaseCCB(pCcb);
    }

    goto cleanup;
}


/********************************************************
 *******************************************************/

static NTSTATUS
PvfsCreateFileOpenIf(
    PPVFS_IRP_CONTEXT pIrpContext
    )
{
    NTSTATUS ntError = STATUS_UNSUCCESSFUL;
    PIRP pIrp = pIrpContext->pIrp;
    IRP_ARGS_CREATE Args = pIrp->Args.Create;
    PIO_CREATE_SECURITY_CONTEXT pSecCtx = Args.SecurityContext;
    PSTR pszFilename = NULL;
    PPVFS_CCB pCcb = NULL;
    FILE_CREATE_RESULT CreateResult = 0;
    ACCESS_MASK GrantedAccess = 0;
    BOOLEAN bFileExisted = FALSE;
    PPVFS_FCB pFcb = NULL;
    PSTR pszDirname = NULL;
    PSTR pszRelativeFilename = NULL;
    PSTR pszDiskFilename = NULL;
    PSTR pszDiskDirname = NULL;
    PVFS_SET_FILE_PROPERTY_FLAGS SetPropertyFlags = PVFS_SET_PROP_NONE;

    ntError = PvfsCanonicalPathName(&pszFilename, Args.FileName);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsFileSplitPath(&pszDirname, &pszRelativeFilename, pszFilename);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsLookupPath(&pszDiskDirname, pszDirname, FALSE);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsAllocateCCB(&pCcb);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsAcquireAccessToken(pCcb, pSecCtx);
    BAIL_ON_NT_STATUS(ntError);

    /* Check for file existence */

    ntError = PvfsLookupFile(&pszDiskFilename,
                             pszDiskDirname,
                             pszRelativeFilename,
                             FALSE);
    bFileExisted = NT_SUCCESS(ntError);

    if (!bFileExisted)
    {
        ntError = RtlCStringAllocatePrintf(&pszDiskFilename,
                                           "%s/%s",
                                           pszDiskDirname,
                                           pszRelativeFilename);
        BAIL_ON_NT_STATUS(ntError);

        ntError = PvfsAccessCheckDir(pCcb->pUserToken,
                                     pszDiskDirname,
                                     FILE_ADD_FILE,
                                     &GrantedAccess);
        BAIL_ON_NT_STATUS(ntError);

        /* This should actually be another check against the
           newly created SD */

        GrantedAccess = FILE_ALL_ACCESS;
    }
    else
    {
        ntError = PvfsAccessCheckFile(pCcb->pUserToken,
                                      pszDiskFilename,
                                      Args.DesiredAccess,
                                      &GrantedAccess);
        BAIL_ON_NT_STATUS(ntError);
    }

    /* Can't set DELETE_ON_CLOSE for ReadOnly files */

    if (Args.CreateOptions & FILE_DELETE_ON_CLOSE)
    {
        FILE_ATTRIBUTES Attributes = 0;

        ntError = PvfsGetFilenameAttributes(pszDiskFilename, &Attributes);
        BAIL_ON_NT_STATUS(ntError);

        if (Attributes & FILE_ATTRIBUTE_READONLY) {
            ntError = STATUS_CANNOT_DELETE;
            BAIL_ON_NT_STATUS(ntError);
        }
    }

    ntError = PvfsCheckShareMode(pszDiskFilename,
                                 Args.ShareAccess,
                                 Args.DesiredAccess,
                                 &pFcb);
    BAIL_ON_NT_STATUS(ntError);

    if (!bFileExisted) {
        SetPropertyFlags = PVFS_SET_PROP_OWNER|PVFS_SET_PROP_ATTRIB;
    }

    ntError = PvfsCreateFileDoSysOpen(
                  pIrpContext,
                  pszDiskFilename,
                  pCcb,
                  pFcb,
                  GrantedAccess,
                  SetPropertyFlags);
    BAIL_ON_NT_STATUS(ntError);

    CreateResult = bFileExisted ? FILE_OPENED : FILE_CREATED;

cleanup:
    pIrp->IoStatusBlock.CreateResult = CreateResult;

    RtlCStringFree(&pszFilename);
    RtlCStringFree(&pszDirname);
    RtlCStringFree(&pszRelativeFilename);
    RtlCStringFree(&pszDiskDirname);

    return ntError;

error:
    CreateResult = (ntError == STATUS_OBJECT_NAME_COLLISION) ?
                   FILE_EXISTS : FILE_DOES_NOT_EXIST;

    RtlCStringFree(&pszDiskFilename);

    if (pFcb) {
        PvfsReleaseFCB(pFcb);
    }

    if (pCcb) {
        PvfsReleaseCCB(pCcb);
    }

    goto cleanup;
}

/********************************************************
 *******************************************************/

static NTSTATUS
PvfsCreateFileOverwrite(
    PPVFS_IRP_CONTEXT pIrpContext
    )
{
    NTSTATUS ntError = STATUS_UNSUCCESSFUL;
    PIRP pIrp = pIrpContext->pIrp;
    IRP_ARGS_CREATE Args = pIrp->Args.Create;
    PIO_CREATE_SECURITY_CONTEXT pSecCtx = Args.SecurityContext;
    PSTR pszFilename = NULL;
    PPVFS_CCB pCcb = NULL;
    FILE_CREATE_RESULT CreateResult = 0;
    ACCESS_MASK GrantedAccess = 0;
    PPVFS_FCB pFcb = NULL;
    PSTR pszDiskFilename = NULL;

    ntError = PvfsCanonicalPathName(&pszFilename, Args.FileName);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsLookupPath(&pszDiskFilename, pszFilename, FALSE);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsAllocateCCB(&pCcb);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsAcquireAccessToken(pCcb, pSecCtx);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsAccessCheckFile(pCcb->pUserToken,
                                  pszDiskFilename,
                                  Args.DesiredAccess,
                                  &GrantedAccess);
    BAIL_ON_NT_STATUS(ntError);

    /* Can't set DELETE_ON_CLOSE for ReadOnly files */

    if (Args.CreateOptions & FILE_DELETE_ON_CLOSE)
    {
        FILE_ATTRIBUTES Attributes = 0;

        ntError = PvfsGetFilenameAttributes(pszDiskFilename, &Attributes);
        BAIL_ON_NT_STATUS(ntError);

        if ((Attributes & FILE_ATTRIBUTE_READONLY) ||
            (Args.FileAttributes & FILE_ATTRIBUTE_READONLY))
        {
            ntError = STATUS_CANNOT_DELETE;
            BAIL_ON_NT_STATUS(ntError);
        }
    }

    ntError = PvfsCheckShareMode(pszDiskFilename,
                                 Args.ShareAccess,
                                 Args.DesiredAccess,
                                 &pFcb);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsCreateFileDoSysOpen(
                  pIrpContext,
                  pszDiskFilename,
                  pCcb,
                  pFcb,
                  GrantedAccess,
                  PVFS_SET_PROP_OWNER|PVFS_SET_PROP_ATTRIB);
    BAIL_ON_NT_STATUS(ntError);

    CreateResult = FILE_OVERWRITTEN;

cleanup:
    pIrp->IoStatusBlock.CreateResult = CreateResult;

    RtlCStringFree(&pszFilename);

    return ntError;

error:
    CreateResult = (ntError == STATUS_OBJECT_PATH_NOT_FOUND) ?
        FILE_DOES_NOT_EXIST : FILE_EXISTS;

    RtlCStringFree(&pszDiskFilename);

    if (pFcb) {
        PvfsReleaseFCB(pFcb);
    }

    if (pCcb) {
        PvfsReleaseCCB(pCcb);
    }

    goto cleanup;
}

/********************************************************
 *******************************************************/

static NTSTATUS
PvfsCreateFileOverwriteIf(
    PPVFS_IRP_CONTEXT pIrpContext
    )
{
    NTSTATUS ntError = STATUS_UNSUCCESSFUL;
    PIRP pIrp = pIrpContext->pIrp;
    IRP_ARGS_CREATE Args = pIrp->Args.Create;
    PIO_CREATE_SECURITY_CONTEXT pSecCtx = Args.SecurityContext;
    PSTR pszFilename = NULL;
    PPVFS_CCB pCcb = NULL;
    FILE_CREATE_RESULT CreateResult = 0;
    ACCESS_MASK GrantedAccess = 0;
    BOOLEAN bFileExisted = FALSE;
    PPVFS_FCB pFcb = NULL;
    PSTR pszDirname = NULL;
    PSTR pszRelativeFilename = NULL;
    PSTR pszDiskFilename = NULL;
    PSTR pszDiskDirname = NULL;

    ntError = PvfsCanonicalPathName(&pszFilename, Args.FileName);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsFileSplitPath(&pszDirname, &pszRelativeFilename, pszFilename);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsLookupPath(&pszDiskDirname, pszDirname, FALSE);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsAllocateCCB(&pCcb);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsAcquireAccessToken(pCcb, pSecCtx);
    BAIL_ON_NT_STATUS(ntError);

    /* Check for file existence */

    ntError = PvfsLookupFile(&pszDiskFilename,
                             pszDiskDirname,
                             pszRelativeFilename,
                             FALSE);
    bFileExisted = NT_SUCCESS(ntError);

    if (!bFileExisted)
    {
        ntError = RtlCStringAllocatePrintf(&pszDiskFilename,
                                           "%s/%s",
                                           pszDiskDirname,
                                           pszRelativeFilename);
        BAIL_ON_NT_STATUS(ntError);

        ntError = PvfsAccessCheckDir(pCcb->pUserToken,
                                     pszDiskDirname,
                                     Args.DesiredAccess,
                                     &GrantedAccess);
        BAIL_ON_NT_STATUS(ntError);

        GrantedAccess = FILE_ALL_ACCESS;
    }
    else
    {
        ntError = PvfsAccessCheckFile(pCcb->pUserToken,
                                      pszDiskFilename,
                                      Args.DesiredAccess,
                                      &GrantedAccess);
        BAIL_ON_NT_STATUS(ntError);

    }

    /* Can't set DELETE_ON_CLOSE for ReadOnly files */

    if (Args.CreateOptions & FILE_DELETE_ON_CLOSE)
    {
        FILE_ATTRIBUTES Attributes = 0;

        if (bFileExisted) {
            ntError = PvfsGetFilenameAttributes(pszDiskFilename, &Attributes);
            BAIL_ON_NT_STATUS(ntError);
        }

        if ((Attributes & FILE_ATTRIBUTE_READONLY) ||
            (Args.FileAttributes & FILE_ATTRIBUTE_READONLY))
        {
            ntError = STATUS_CANNOT_DELETE;
            BAIL_ON_NT_STATUS(ntError);
        }
    }

    ntError = PvfsCheckShareMode(pszDiskFilename,
                                 Args.ShareAccess,
                                 Args.DesiredAccess,
                                 &pFcb);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsCreateFileDoSysOpen(
                  pIrpContext,
                  pszDiskFilename,
                  pCcb,
                  pFcb,
                  GrantedAccess,
                  PVFS_SET_PROP_OWNER|PVFS_SET_PROP_ATTRIB);
    BAIL_ON_NT_STATUS(ntError);

    CreateResult = bFileExisted ? FILE_OVERWRITTEN : FILE_CREATED;

cleanup:
    pIrp->IoStatusBlock.CreateResult = CreateResult;

    RtlCStringFree(&pszFilename);
    RtlCStringFree(&pszDirname);
    RtlCStringFree(&pszRelativeFilename);
    RtlCStringFree(&pszDiskDirname);

    return ntError;

error:
    CreateResult = (ntError == STATUS_OBJECT_NAME_COLLISION) ?
                   FILE_EXISTS : FILE_DOES_NOT_EXIST;

    RtlCStringFree(&pszDiskFilename);

    if (pFcb) {
        PvfsReleaseFCB(pFcb);
    }

    if (pCcb) {
        PvfsReleaseCCB(pCcb);
    }

    goto cleanup;
}

/********************************************************
 *******************************************************/

static NTSTATUS
PvfsCreateFileDoSysOpen(
    IN PPVFS_IRP_CONTEXT pIrpContext,
    IN PSTR pszDiskFilename,
    IN PPVFS_CCB pCcb,
    IN PPVFS_FCB pFcb,
    IN ACCESS_MASK GrantedAccess,
    IN PVFS_SET_FILE_PROPERTY_FLAGS Flags
    )
{
    NTSTATUS ntError = STATUS_UNSUCCESSFUL;
    IRP_ARGS_CREATE Args = pIrpContext->pIrp->Args.Create;
    int fd = -1;
    int unixFlags = 0;
    PIO_CREATE_SECURITY_CONTEXT pSecCtx = Args.SecurityContext;

    /* Do the open() */

    ntError = MapPosixOpenFlags(&unixFlags, GrantedAccess, Args);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsSysOpen(&fd, pszDiskFilename, unixFlags, 0600);
    BAIL_ON_NT_STATUS(ntError);

    /* Save our state */

    pCcb->fd = fd;
    pCcb->ShareFlags = Args.ShareAccess;
    pCcb->AccessGranted = GrantedAccess;
    pCcb->CreateOptions = Args.CreateOptions;
    pCcb->pszFilename = pszDiskFilename;

    ntError = PvfsAddCCBToFCB(pFcb, pCcb);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsSaveFileDeviceInfo(pCcb);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsStoreCCB(pIrpContext->pIrp->FileHandle, pCcb);
    BAIL_ON_NT_STATUS(ntError);

    /* File properties */

    if ((Flags & PVFS_SET_PROP_OWNER) && pSecCtx)
    {
        PIO_SECURITY_CONTEXT_PROCESS_INFORMATION pProcess = NULL;

        pProcess = IoSecurityGetProcessInfo(pSecCtx);

        ntError = PvfsSysChown(pCcb,
                               pProcess->Uid,
                               pProcess->Gid);
        BAIL_ON_NT_STATUS(ntError);
    }

    /* This should always update the File Attributes even if the
       file previously existed */

    if ((Flags & PVFS_SET_PROP_ATTRIB) && (Args.FileAttributes != 0))
    {
        ntError = PvfsSetFileAttributes(pCcb, Args.FileAttributes);
        BAIL_ON_NT_STATUS(ntError);
    }


cleanup:
    return ntError;

error:
    if (fd != -1) {
        close(fd);
    }

    goto cleanup;
}



/*
local variables:
mode: c
c-basic-offset: 4
indent-tabs-mode: nil
tab-width: 4
end:
*/
