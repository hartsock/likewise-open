/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 */

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
 *        notify.c
 *
 * Abstract:
 *
 *        Likewise IO (LWIO) - SRV
 *
 *        Protocols API - SMBV1
 *
 *        Change Notify
 *
 * Authors: Sriram Nambakam (snambakam@likewise.com)
 *
 */

#include "includes.h"

static
VOID
SrvNotifyAsyncCB(
    PVOID pContext
    );

static
NTSTATUS
SrvBuildNotifyExecContext(
    PSRV_CHANGE_NOTIFY_STATE_SMB_V1 pNotifyState,
    PSRV_EXEC_CONTEXT*              ppExecContext
    );

static
VOID
SrvFreeNotifyState(
    PSRV_CHANGE_NOTIFY_STATE_SMB_V1 pNotifyState
    );

NTSTATUS
SrvBuildNotifyState(
    PLWIO_SRV_CONNECTION             pConnection,
    PLWIO_SRV_SESSION                pSession,
    PLWIO_SRV_TREE                   pTree,
    PLWIO_SRV_FILE                   pFile,
    USHORT                           usMid,
    ULONG                            ulPid,
    ULONG                            ulRequestSequence,
    ULONG                            ulCompletionFilter,
    BOOLEAN                          bWatchTree,
    ULONG                            ulMaxBufferSize,
    PSRV_CHANGE_NOTIFY_STATE_SMB_V1* ppNotifyState
    )
{
    NTSTATUS                        ntStatus     = STATUS_SUCCESS;
    PSRV_CHANGE_NOTIFY_STATE_SMB_V1 pNotifyState = NULL;

    ntStatus = SrvAllocateMemory(
                    sizeof(SRV_CHANGE_NOTIFY_STATE_SMB_V1),
                    (PVOID*)&pNotifyState);
    BAIL_ON_NT_STATUS(ntStatus);

    pNotifyState->refCount = 1;

    pthread_mutex_init(&pNotifyState->mutex, NULL);
    pNotifyState->pMutex = &pNotifyState->mutex;

    pNotifyState->pConnection = pConnection;
    InterlockedIncrement(&pNotifyState->refCount);

    pNotifyState->ulCompletionFilter = ulCompletionFilter;
    pNotifyState->bWatchTree         = bWatchTree;

    pNotifyState->usUid = pSession->uid;
    pNotifyState->usTid = pTree->tid;
    pNotifyState->usFid = pFile->fid;
    pNotifyState->usMid = usMid;
    pNotifyState->ulPid = ulPid;

    pNotifyState->ulRequestSequence = ulRequestSequence;

    pNotifyState->ulMaxBufferSize   = ulMaxBufferSize;

    *ppNotifyState = pNotifyState;

cleanup:

    return ntStatus;

error:

    *ppNotifyState = NULL;

    if (pNotifyState)
    {
        SrvFreeNotifyState(pNotifyState);
    }

    goto cleanup;
}

VOID
SrvPrepareNotifyStateAsync(
    PSRV_CHANGE_NOTIFY_STATE_SMB_V1 pNotifyState
    )
{
    pNotifyState->acb.Callback        = &SrvNotifyAsyncCB;

    pNotifyState->acb.CallbackContext = pNotifyState;
    InterlockedIncrement(&pNotifyState->refCount);

    pNotifyState->acb.AsyncCancelContext = NULL;

    pNotifyState->pAcb = &pNotifyState->acb;
}

static
VOID
SrvNotifyAsyncCB(
    PVOID pContext
    )
{
    NTSTATUS          ntStatus     = STATUS_SUCCESS;
    PSRV_EXEC_CONTEXT pExecContext = NULL;
    BOOLEAN           bInLock      = FALSE;
    PSRV_CHANGE_NOTIFY_STATE_SMB_V1 pNotifyState =
                            (PSRV_CHANGE_NOTIFY_STATE_SMB_V1)pContext;

    LWIO_LOCK_MUTEX(bInLock, &pNotifyState->mutex);

    if (pNotifyState->pAcb->AsyncCancelContext)
    {
        IoDereferenceAsyncCancelContext(
                &pNotifyState->pAcb->AsyncCancelContext);
    }

    pNotifyState->pAcb = NULL;

    if (pNotifyState->ioStatusBlock.Status == STATUS_CANCELLED)
    {
        ntStatus = STATUS_SUCCESS;
        goto cleanup;
    }

    ntStatus = pNotifyState->ioStatusBlock.Status;
    BAIL_ON_NT_STATUS(ntStatus);

    ntStatus = SrvBuildNotifyExecContext(
                    pNotifyState,
                    &pExecContext);
    BAIL_ON_NT_STATUS(ntStatus);

    ntStatus = SrvProdConsEnqueue(
                    gProtocolGlobals_SMB_V1.pWorkQueue,
                    pExecContext);
    BAIL_ON_NT_STATUS(ntStatus);

    pExecContext = NULL;

cleanup:

    LWIO_UNLOCK_MUTEX(bInLock, &pNotifyState->mutex);

    if (pNotifyState)
    {
        SrvReleaseNotifyState(pNotifyState);
    }

    if (pExecContext)
    {
        SrvReleaseExecContext(pExecContext);
    }

    return;

error:

    LWIO_LOG_ERROR("Error: Failed processing change notify call back "
                   "[status:0x%x]",
                   ntStatus);

    // TODO: indicate error on file handle somehow

    goto cleanup;
}

static
NTSTATUS
SrvBuildNotifyExecContext(
    PSRV_CHANGE_NOTIFY_STATE_SMB_V1 pNotifyState,
    PSRV_EXEC_CONTEXT*              ppExecContext
    )
{
    NTSTATUS                 ntStatus         = STATUS_SUCCESS;
    PSRV_EXEC_CONTEXT        pExecContext     = NULL;
    PSMB_PACKET              pSmbRequest      = NULL;
    PBYTE                    pParams          = NULL;
    ULONG                    ulParamLength    = 0;
    PBYTE                    pData            = NULL;
    ULONG                    ulDataLen        = 0;
    ULONG                    ulDataOffset     = 0;
    PBYTE                    pBuffer          = NULL;
    ULONG                    ulBytesAvailable = 0;
    ULONG                    ulOffset         = 0;
    USHORT                   usBytesUsed      = 0;
    USHORT                   usTotalBytesUsed = 0;
    PSMB_HEADER              pHeader          = NULL; // Do not free
    PBYTE                    pWordCount       = NULL; // Do not free
    PANDX_HEADER             pAndXHeader      = NULL; // Do not free
    PUSHORT                  pSetup           = NULL;
    UCHAR                    ucSetupCount     = 0;
    ULONG                    ulParameterOffset     = 0;
    ULONG                    ulNumPackageBytesUsed = 0;
    SMB_NOTIFY_CHANGE_HEADER notifyRequestHeader   = {0};

    ntStatus = SMBPacketAllocate(
                    pNotifyState->pConnection->hPacketAllocator,
                    &pSmbRequest);
    BAIL_ON_NT_STATUS(ntStatus);

    ntStatus = SMBPacketBufferAllocate(
                    pNotifyState->pConnection->hPacketAllocator,
                    (64 * 1024) + 4096,
                    &pSmbRequest->pRawBuffer,
                    &pSmbRequest->bufferLen);
    BAIL_ON_NT_STATUS(ntStatus);

    ntStatus = SrvInitPacket_SMB_V1(pSmbRequest, TRUE);
    BAIL_ON_NT_STATUS(ntStatus);

    ntStatus = SrvBuildExecContext(
                    pNotifyState->pConnection,
                    pSmbRequest,
                    TRUE,
                    &pExecContext);
    BAIL_ON_NT_STATUS(ntStatus);

    pBuffer = pSmbRequest->pRawBuffer;
    ulBytesAvailable = pSmbRequest->bufferLen;

    if (ulBytesAvailable < sizeof(NETBIOS_HEADER))
    {
        ntStatus = STATUS_INVALID_NETWORK_RESPONSE;
        BAIL_ON_NT_STATUS(ntStatus);
    }

    pBuffer += sizeof(NETBIOS_HEADER);
    ulBytesAvailable -= sizeof(NETBIOS_HEADER);

    ntStatus = SrvMarshalHeader_SMB_V1(
                    pBuffer,
                    ulOffset,
                    ulBytesAvailable,
                    COM_NT_TRANSACT,
                    STATUS_SUCCESS,
                    FALSE,  /* not a response */
                    pNotifyState->usTid,
                    pNotifyState->ulPid,
                    pNotifyState->usUid,
                    pNotifyState->usMid,
                    pNotifyState->pConnection->serverProperties.bRequireSecuritySignatures,
                    &pHeader,
                    &pWordCount,
                    &pAndXHeader,
                    &usBytesUsed);
    BAIL_ON_NT_STATUS(ntStatus);

    pBuffer          += usBytesUsed;
    ulOffset         += usBytesUsed;
    ulBytesAvailable -= usBytesUsed;
    usTotalBytesUsed += usBytesUsed;

    notifyRequestHeader.usFid = pNotifyState->usFid;

    pSetup       = (PUSHORT)&notifyRequestHeader;
    ucSetupCount = sizeof(notifyRequestHeader)/sizeof(USHORT);

    *pWordCount = 18 + ucSetupCount;

    ntStatus = WireMarshallNtTransactionResponse(
                    pBuffer,
                    ulBytesAvailable,
                    ulOffset,
                    pSetup,
                    ucSetupCount,
                    pParams,
                    ulParamLength,
                    pData,
                    ulDataLen,
                    &ulDataOffset,
                    &ulParameterOffset,
                    &ulNumPackageBytesUsed);
    BAIL_ON_NT_STATUS(ntStatus);

    // pBuffer          += ulNumPackageBytesUsed;
    // ulOffset         += ulNumPackageBytesUsed;
    // ulBytesAvailable -= ulNumPackageBytesUsed;
    usTotalBytesUsed += ulNumPackageBytesUsed;

    pSmbRequest->bufferUsed += usTotalBytesUsed;

    ntStatus = SMBPacketMarshallFooter(pSmbRequest);
    BAIL_ON_NT_STATUS(ntStatus);

    *ppExecContext = pExecContext;

cleanup:

    if (pSmbRequest)
    {
        SMBPacketRelease(
                pNotifyState->pConnection->hPacketAllocator,
                pSmbRequest);
    }

    return ntStatus;

error:

    *ppExecContext = NULL;

    if (pExecContext)
    {
        SrvReleaseExecContext(pExecContext);
    }

    goto cleanup;
}

VOID
SrvReleaseNotifyStateAsync(
    PSRV_CHANGE_NOTIFY_STATE_SMB_V1 pNotifyState
    )
{
    if (pNotifyState->pAcb)
    {
        pNotifyState->acb.Callback        = NULL;

        if (pNotifyState->pAcb->CallbackContext)
        {
            InterlockedDecrement(&pNotifyState->refCount);

            pNotifyState->pAcb->CallbackContext = NULL;
        }

        if (pNotifyState->pAcb->AsyncCancelContext)
        {
            IoDereferenceAsyncCancelContext(
                    &pNotifyState->pAcb->AsyncCancelContext);
        }

        pNotifyState->pAcb = NULL;
    }
}

VOID
SrvReleaseNotifyState(
    PSRV_CHANGE_NOTIFY_STATE_SMB_V1 pNotifyState
    )
{
    if (InterlockedDecrement(&pNotifyState->refCount) == 0)
    {
        SrvFreeNotifyState(pNotifyState);
    }
}

static
VOID
SrvFreeNotifyState(
    PSRV_CHANGE_NOTIFY_STATE_SMB_V1 pNotifyState
    )
{
    if (pNotifyState->pAcb && pNotifyState->pAcb->AsyncCancelContext)
    {
        IoDereferenceAsyncCancelContext(
                    &pNotifyState->pAcb->AsyncCancelContext);
    }

    if (pNotifyState->pConnection)
    {
        SrvConnectionRelease(pNotifyState->pConnection);
    }

    if (pNotifyState->pBuffer)
    {
        SrvFreeMemory(pNotifyState->pBuffer);
    }

    if (pNotifyState->pMutex)
    {
        pthread_mutex_destroy(&pNotifyState->mutex);
    }

    SrvFreeMemory(pNotifyState);
}
