/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*-
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * Editor Settings: expandtabs and use 4 spaces for indentation */

/*
 * Copyright (c) Likewise Software.  All rights reserved.
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
 * license@likewise.com
 */

/*
 * Copyright (C) Likewise Software. All rights reserved.
 *
 * Module Name:
 *
 *        transport.c
 *
 * Abstract:
 *
 *        Likewise IO (LWIO) - SRV
 *
 *        Protocols API
 *
 *        Protocol Transport Driver
 *
 *        [OPTIONAL FILE DESCRIPTION]
 *
 * Authors: Danilo Almeida (dalmeida@likewise.com)
 *
 */

#include "includes.h"

// TODO-Remove this duplicate definition (from smb2/structs.h)
// and use a common SMB2 wire protocol structure header.
typedef struct __SMB2_WRITE_REQUEST_HEADER
{
    USHORT   usLength;
    USHORT   usDataOffset;
    ULONG    ulDataLength;
    ULONG64  ullFileOffset;
    SMB2_FID fid;
    ULONG    ulRemaining;
    ULONG    ulChannel;
    USHORT   usWriteChannelInfoOffset;
    USHORT   usWriteChannelInfoLength;
    ULONG    ulFlags;

} __attribute__((__packed__)) SMB2_WRITE_REQUEST_HEADER,
                             *PSMB2_WRITE_REQUEST_HEADER;

static
int
SrvProtocolTransportConnectionCompare(
    PVOID pKey1,
    PVOID pKey2
    );

static
VOID
SrvProtocolTransportConnectionRelease(
    PVOID pConnection
    );

// Transport Callbacks

static
NTSTATUS
SrvProtocolTransportDriverConnectionNew(
    OUT PSRV_CONNECTION* ppConnection,
    IN PSRV_PROTOCOL_TRANSPORT_CONTEXT pProtocolDispatchContext,
    IN PSRV_SOCKET pSocket
    );

static
NTSTATUS
SrvProtocolTransportDriverConnectionData(
    IN PSRV_CONNECTION pConnection,
    IN ULONG Length
    );

static
VOID
SrvProtocolTransportDriverConnectionDone(
    IN PSRV_CONNECTION pConnection,
    IN NTSTATUS Status
    );

static
NTSTATUS
SrvProtocolTransportDriverSendPrepare(
    IN PSRV_SEND_CONTEXT pSendContext
    );

static
VOID
SrvProtocolTransportDriverSendDone(
    IN PSRV_SEND_CONTEXT pSendContext,
    IN NTSTATUS Status
    );

// Helpers

static
VOID
SrvProtocolTransportDriverSocketFree(
    IN OUT PSRV_SOCKET pSocket
    );

static
VOID
SrvProtocolTransportDriverSocketDisconnect(
    IN PSRV_SOCKET pSocket
    );

static
VOID
SrvProtocolTransportDriverSocketGetClientAddress(
    IN PSRV_SOCKET pSocket,
    OUT const struct sockaddr** ppAddress,
    OUT SOCKLEN_TYPE* pAddressLength
    );

static
VOID
SrvProtocolTransportDriverSocketGetServerAddress(
    IN PSRV_SOCKET pSocket,
    OUT const struct sockaddr** ppAddress,
    OUT SOCKLEN_TYPE* pAddressLength
    );

static
VOID
SrvProtocolTransportDriverSocketResetTimeout(
    IN PSRV_SOCKET pSocket,
    IN BOOLEAN bIsEnabled,
    IN ULONG ulTimeoutSeconds
    );

static
NTSTATUS
SrvProtocolTransportDriverAllocatePacket(
    IN PSRV_CONNECTION pConnection
    );

static
NTSTATUS
SrvProtocolTransportDriverUpdateBuffer(
    IN PSRV_CONNECTION pConnection
    );

static
VOID
SrvProtocolTransportDriverRemoveBuffer(
    IN PSRV_CONNECTION pConnection
    );

static
NTSTATUS
SrvProtocolTransportDriverDetectPacket(
    IN PSRV_CONNECTION pConnection,
    IN OUT PULONG pulBytesAvailable,
    OUT PSMB_PACKET* ppPacket,
    OUT PSRV_EXEC_CONTEXT* ppZctExecContext
    );

static
NTSTATUS
SrvProtocolTransportDriverDispatchPacket(
    IN PSRV_CONNECTION pConnection,
    IN PSMB_PACKET pPacket
    );

static
NTSTATUS
SrvProtocolTransportDriverSetStatistics(
    PLWIO_SRV_CONNECTION  pConnection,     /* IN     */
    SMB_PROTOCOL_VERSION  protocolVersion, /* IN     */
    ULONG                 ulRequestLength, /* IN     */
    PSRV_STAT_INFO*       ppStatInfo       /*    OUT */
    );

static
NTSTATUS
SrvProtocolTransportDriverCheckSignature(
    IN PSRV_CONNECTION pConnection,
    IN PSMB_PACKET pPacket
    );

static
ULONG
SrvProtocolTransportDriverGetNextSequence(
    IN PSRV_CONNECTION pConnection,
    IN PSMB_PACKET pPacket
    );

static
VOID
SrvProtocolTransportDriverFreeResources(
    IN PSRV_SEND_CONTEXT pSendContext
    );

static
PFN_SRV_CONNECTION_IO_COMPLETE
SrvProtocolTransportDriverGetZctCallback(
    IN PLWIO_SRV_CONNECTION pConnection,
    OUT PVOID* ppCallbackContext
    );

// Implementations

NTSTATUS
SrvProtocolTransportDriverInit(
    IN PSRV_PROTOCOL_API_GLOBALS pGlobals,
    IN PLW_THREAD_POOL ThreadPool
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PSRV_PROTOCOL_TRANSPORT_CONTEXT pTransportContext = &pGlobals->transportContext;
    PSRV_TRANSPORT_PROTOCOL_DISPATCH pTransportDispatch = &pTransportContext->dispatch;
    PSRV_CONNECTION_SOCKET_DISPATCH pSocketDispatch = &pTransportContext->socketDispatch;

    RtlZeroMemory(pTransportContext, sizeof(*pTransportContext));

    pTransportContext->pGlobals = pGlobals;
    pTransportContext->ThreadPool = ThreadPool;

    pTransportDispatch->pfnConnectionNew = SrvProtocolTransportDriverConnectionNew;
    pTransportDispatch->pfnConnectionData = SrvProtocolTransportDriverConnectionData;
    pTransportDispatch->pfnConnectionDone = SrvProtocolTransportDriverConnectionDone;
    pTransportDispatch->pfnSendPrepare = SrvProtocolTransportDriverSendPrepare;
    pTransportDispatch->pfnSendDone = SrvProtocolTransportDriverSendDone;

    pSocketDispatch->pfnFree = SrvProtocolTransportDriverSocketFree;
    pSocketDispatch->pfnDisconnect = SrvProtocolTransportDriverSocketDisconnect;
    pSocketDispatch->pfnGetClientAddress = SrvProtocolTransportDriverSocketGetClientAddress;
    pSocketDispatch->pfnGetServerAddress = SrvProtocolTransportDriverSocketGetServerAddress;
    pSocketDispatch->pfnResetTimeout = SrvProtocolTransportDriverSocketResetTimeout;

    uuid_generate(pTransportContext->guid);

    ntStatus = LwRtlRBTreeCreate(
                    &SrvProtocolTransportConnectionCompare,
                    NULL,
                    &SrvProtocolTransportConnectionRelease,
                    &pGlobals->pConnections);
    BAIL_ON_NT_STATUS(ntStatus);

cleanup:

    return ntStatus;

error:

    SrvProtocolTransportDriverShutdown(pGlobals);

    goto cleanup;
}

NTSTATUS
SrvProtocolTransportDriverStart(
    IN PSRV_PROTOCOL_API_GLOBALS pGlobals
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PSRV_PROTOCOL_TRANSPORT_CONTEXT pTransportContext = &pGlobals->transportContext;
    PSRV_TRANSPORT_PROTOCOL_DISPATCH pTransportDispatch = &pTransportContext->dispatch;
    BOOLEAN bInLock = FALSE;

    LWIO_LOCK_RWMUTEX_EXCLUSIVE(bInLock, pGlobals->pTransportStartStopMutex);

    if (pTransportContext->hTransport)
    {
        // Already started.
        ntStatus = STATUS_SUCCESS;
        goto cleanup;
    }

    ntStatus = SrvTransportInit(
                    &pTransportContext->hTransport,
                    pTransportContext->ThreadPool,
                    pTransportDispatch,
                    pTransportContext,
                    SrvProtocolConfigIsNetbiosEnabled());
    BAIL_ON_NT_STATUS(ntStatus);

cleanup:
    LWIO_UNLOCK_RWMUTEX(bInLock, pGlobals->pTransportStartStopMutex);

    return ntStatus;

error:
    goto cleanup;
}

static
NTSTATUS
SrvProtocolTransportDriverCanStopConnectionCallback(
    IN PVOID pKey,
    IN PVOID pData,
    IN PVOID pUserData,
    IN PBOOLEAN pbContinue
    )
{
    PLWIO_SRV_CONNECTION pConnection = (PLWIO_SRV_CONNECTION)pData;
    BOOLEAN bInLock = FALSE;
    BOOLEAN isActive = FALSE;

    LWIO_LOCK_RWMUTEX_SHARED(bInLock, &pConnection->mutex);

    // This is the same criteria used in SrvConnectionSetInvalidEx()
    // for determining whether a connection is not active.

    if (0 != pConnection->ullSessionCount)
    {
        isActive = TRUE;
    }

    LWIO_UNLOCK_RWMUTEX(bInLock, &pConnection->mutex);

    *(PBOOLEAN)pUserData = isActive;
    *pbContinue = isActive ? FALSE : TRUE;

    return STATUS_SUCCESS;
}

typedef struct _SRV_STOP_CONNECTION_CONTEXT {
    BOOLEAN IsForce;
    BOOLEAN FailedDisconnect;
} SRV_STOP_CONNECTION_CONTEXT, *PSRV_STOP_CONNECTION_CONTEXT;

static
NTSTATUS
SrvProtocolTransportDriverStopConnectionCallback(
    IN PVOID pKey,
    IN PVOID pData,
    IN PVOID pUserData,
    IN PBOOLEAN pbContinue
    )
{
    PLWIO_SRV_CONNECTION pConnection = (PLWIO_SRV_CONNECTION)pData;
    PSRV_STOP_CONNECTION_CONTEXT pContext = (PSRV_STOP_CONNECTION_CONTEXT)pUserData;
    BOOLEAN isDisconnected = FALSE;

    if (pContext->IsForce)
    {
        SrvConnectionSetInvalid(pConnection);
        isDisconnected = TRUE;
    }
    else
    {
        isDisconnected = SrvConnectionSetInvalidIfNoSessions(pConnection);
        if (!isDisconnected)
        {
            pContext->FailedDisconnect = TRUE;
        }
    }

    *pbContinue = isDisconnected;

    return STATUS_SUCCESS;
}

BOOLEAN
SrvProtocolTransportDriverStop(
    IN PSRV_PROTOCOL_API_GLOBALS pGlobals,
    IN BOOLEAN IsForce
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PSRV_PROTOCOL_TRANSPORT_CONTEXT pTransportContext = &pGlobals->transportContext;
    BOOLEAN bInLock = FALSE;
    BOOLEAN bInTransportLock = FALSE;
    BOOLEAN isStopped = FALSE;
    SRV_STOP_CONNECTION_CONTEXT stopContext = { 0 };

    LWIO_LOCK_RWMUTEX_EXCLUSIVE(bInTransportLock, pGlobals->pTransportStartStopMutex);

    if (!pTransportContext->hTransport)
    {
        // Already stopped.
        isStopped = TRUE;
        goto cleanup;
    }

    LWIO_LOCK_RWMUTEX_EXCLUSIVE(bInLock, &gProtocolApiGlobals.mutex);

    if (!IsForce)
    {
        BOOLEAN isActive = FALSE;

        ntStatus = LwRtlRBTreeTraverse(
                        pGlobals->pConnections,
                        LWRTL_TREE_TRAVERSAL_TYPE_IN_ORDER,
                        SrvProtocolTransportDriverCanStopConnectionCallback,
                        &isActive);
        LWIO_ASSERT(!ntStatus);
        BAIL_ON_NT_STATUS(ntStatus);

        if (isActive)
        {
            isStopped = FALSE;
            goto cleanup;
        }
    }

    //
    // It is possible for a session setup (and more) to happen since
    // the check above.  Therefore, the stop operation below will
    // do a conditional disconnect (based on the presense of sessions)
    // if IsForce is FALSE.  If a connection cannot be disconnected due
    // to sessions that got established in the meantime, the traversal
    // of the connections is aborted and the overall stop operation
    // will fail.
    //

    stopContext.IsForce = IsForce;
    stopContext.FailedDisconnect = FALSE;

    ntStatus = LwRtlRBTreeTraverse(
                    pGlobals->pConnections,
                    LWRTL_TREE_TRAVERSAL_TYPE_IN_ORDER,
                    SrvProtocolTransportDriverStopConnectionCallback,
                    &stopContext);
    LWIO_ASSERT(!ntStatus);
    BAIL_ON_NT_STATUS(ntStatus);

    isStopped = !stopContext.FailedDisconnect;
    if (!isStopped)
    {
        goto cleanup;
    }

    // Must set connection rundown before dropping the lock so as to prevent
    // any new connections from getting added.
    pGlobals->IsConnectionRundown = TRUE;

    LWIO_UNLOCK_RWMUTEX(bInLock, &gProtocolApiGlobals.mutex);

    // This will cause done notifications to occur,
    // which will get rid of any last remaining connection
    // references.
    SrvTransportShutdown(pTransportContext->hTransport);
    pTransportContext->hTransport = NULL;

    // Conenction rundown is complete.
    LWIO_LOCK_RWMUTEX_EXCLUSIVE(bInLock, &gProtocolApiGlobals.mutex);
    pGlobals->IsConnectionRundown = FALSE;
    LWIO_UNLOCK_RWMUTEX(bInLock, &gProtocolApiGlobals.mutex);

cleanup:
    LWIO_UNLOCK_RWMUTEX(bInLock, &gProtocolApiGlobals.mutex);
    LWIO_UNLOCK_RWMUTEX(bInTransportLock, pGlobals->pTransportStartStopMutex);

    LWIO_ASSERT(isStopped || !IsForce);
    return isStopped;

error:
    LWIO_ASSERT(FALSE);
    goto cleanup;
}

BOOLEAN
SrvProtocolTransportDriverIsStarted(
    IN PSRV_PROTOCOL_API_GLOBALS pGlobals
    )
{
    PSRV_PROTOCOL_TRANSPORT_CONTEXT pTransportContext = &pGlobals->transportContext;
    BOOLEAN bInLock = FALSE;
    BOOLEAN isStarted = FALSE;

    LWIO_LOCK_RWMUTEX_SHARED(bInLock, pGlobals->pTransportStartStopMutex);

    isStarted = pTransportContext->hTransport ? TRUE : FALSE;

    LWIO_UNLOCK_RWMUTEX(bInLock, pGlobals->pTransportStartStopMutex);

    return isStarted;
}

static
int
SrvProtocolTransportConnectionCompare(
    PVOID pKey1,
    PVOID pKey2
    )
{
    PCSTR pszAddress1 = (PCSTR)pKey1;
    PCSTR pszAddress2 = (PCSTR)pKey2;

    return strcasecmp(pszAddress1, pszAddress2);
}

static
VOID
SrvProtocolTransportConnectionRelease(
    PVOID pConnection
    )
{
    SrvConnectionRelease((PLWIO_SRV_CONNECTION)pConnection);
}

VOID
SrvProtocolTransportDriverShutdown(
    PSRV_PROTOCOL_API_GLOBALS pGlobals
    )
{
    PSRV_PROTOCOL_TRANSPORT_CONTEXT pTransportContext = &pGlobals->transportContext;

    SrvProtocolTransportDriverStop(pGlobals, TRUE);

    pTransportContext->ThreadPool = NULL;

    // Zero the transport dispatch but leave the socket dispatch for
    // the Producer/Consumer Queue shutdown in case there is an existing socket
    // on a connection

    RtlZeroMemory(&pTransportContext->dispatch, sizeof(pTransportContext->dispatch));

    if (pGlobals->pConnections)
    {
        LwRtlRBTreeFree(pGlobals->pConnections);
        pGlobals->pConnections = NULL;
    }
}

static
NTSTATUS
SrvProtocolTransportDriverConnectionNew(
    OUT PSRV_CONNECTION* ppConnection,
    IN PSRV_PROTOCOL_TRANSPORT_CONTEXT pProtocolDispatchContext,
    IN PSRV_SOCKET pSocket
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PSRV_PROTOCOL_API_GLOBALS pGlobals = pProtocolDispatchContext->pGlobals;
    SRV_PROPERTIES properties = { 0 };
    PLWIO_SRV_CONNECTION pConnection = NULL;
    BOOLEAN bInLock = FALSE;

    properties.preferredSecurityMode = SMB_SECURITY_MODE_USER;
    properties.bEncryptPasswords = TRUE;
    properties.bEnableSecuritySignatures = SrvProtocolConfigIsSigningEnabled();
    properties.bRequireSecuritySignatures = SrvProtocolConfigIsSigningRequired();
    properties.ulZctReadThreshold = SrvProtocolConfigGetZctReadThreshold();
    properties.ulZctWriteThreshold = SrvProtocolConfigGetZctWriteThreshold();
    properties.MaxRawSize = 64 * 1024;
    // TODO: Get MaxMpxCount from config.
    properties.MaxMpxCount = 50;
    properties.MaxNumberVCs = 1;
    properties.MaxBufferSize = 16644;
    properties.MaxBufferSize_SMB_V2 = 65536;
    properties.Capabilities = 0;
    properties.Capabilities |= CAP_UNICODE;
    properties.Capabilities |= CAP_LARGE_FILES;
    properties.Capabilities |= CAP_NT_SMBS;
    properties.Capabilities |= CAP_RPC_REMOTE_APIS;
    properties.Capabilities |= CAP_STATUS32;
    properties.Capabilities |= CAP_LEVEL_II_OPLOCKS;
    properties.Capabilities |= CAP_NT_FIND;
    properties.Capabilities |= CAP_INFOLEVEL_PASSTHRU;
    properties.Capabilities |= CAP_LARGE_READX;
    properties.Capabilities |= CAP_LARGE_WRITEX;
    properties.Capabilities |= CAP_EXTENDED_SECURITY;
# if 0  /* DISABLED - WIP */
    properties.Capabilities |= CAP_DFS;
#endif
    uuid_copy(properties.GUID, pProtocolDispatchContext->guid);

    ntStatus = SrvConnectionCreate(
                    pSocket,
                    pGlobals->hPacketAllocator,
                    pGlobals->pShareList,
                    &properties,
                    &pProtocolDispatchContext->socketDispatch,
                    &pConnection);
    BAIL_ON_NT_STATUS(ntStatus);

    pConnection->ulIdleTimeoutSeconds = SrvProtocolConfigGetIdleTimeout();

    ntStatus = SrvElementsRegisterResource(&pConnection->resource, NULL);
    BAIL_ON_NT_STATUS(ntStatus);

    ntStatus = SrvOEMCreateClientConnection(
                    pConnection->pClientAddress,
                    pConnection->clientAddrLen,
                    pConnection->pServerAddress,
                    pConnection->serverAddrLen,
                    pConnection->resource.ulResourceId,
                    &pConnection->pOEMConnection,
                    &pConnection->ulOEMConnectionLength);
    BAIL_ON_NT_STATUS(ntStatus);

    pConnection->pProtocolTransportDriverContext = pProtocolDispatchContext;

    // Allocate buffer space
    ntStatus = SrvProtocolTransportDriverAllocatePacket(pConnection);
    BAIL_ON_NT_STATUS(ntStatus);

    // Update remaining buffer space
    ntStatus = SrvProtocolTransportDriverUpdateBuffer(pConnection);
    BAIL_ON_NT_STATUS(ntStatus);

    LWIO_LOCK_RWMUTEX_EXCLUSIVE(bInLock, &gProtocolApiGlobals.mutex);

    if (gProtocolApiGlobals.IsConnectionRundown)
    {
        ntStatus = STATUS_CONNECTION_DISCONNECTED;
        BAIL_ON_NT_STATUS(ntStatus);
    }

    ntStatus = LwRtlRBTreeAdd(
                    gProtocolApiGlobals.pConnections,
                    &pConnection->resource.ulResourceId,
                    pConnection);
    BAIL_ON_NT_STATUS(ntStatus);

    *ppConnection = SrvConnectionAcquire(pConnection);;

cleanup:

    LWIO_UNLOCK_RWMUTEX(bInLock, &gProtocolApiGlobals.mutex);

    return ntStatus;

error:

    if (pConnection)
    {
        SrvConnectionRelease(pConnection);
    }

    goto cleanup;
}

// This implements that state machine for the connection.
// TODO-Add ZCT SMB write support
static
NTSTATUS
SrvProtocolTransportDriverConnectionData(
    IN PSRV_CONNECTION pConnection,
    IN ULONG BytesAvailable
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    BOOLEAN bInLock = FALSE;
    ULONG bytesRemaining = BytesAvailable;
    PSMB_PACKET pPacket = NULL;
    PSRV_EXEC_CONTEXT pZctExecContext = NULL;
    PFN_SRV_PROTOCOL_SEND_COMPLETE pfnZctCallback = NULL;
    PVOID pZctCallbackContext = NULL;

    LWIO_ASSERT(BytesAvailable > 0);

    // TODO-Remove this lock?  This should be doable
    // because the transport guarantees that only one thread can be
    // processing a connection data callback.  The only caveat
    // might be when we introduce buffer manipulation from another thread.
    LWIO_LOCK_RWMUTEX_EXCLUSIVE(bInLock, &pConnection->mutex);

    pfnZctCallback = SrvProtocolTransportDriverGetZctCallback(
                        pConnection,
                        &pZctCallbackContext);
    if (pfnZctCallback)
    {
        goto cleanup;
    }

    while (bytesRemaining > 0)
    {
        ntStatus = SrvProtocolTransportDriverDetectPacket(
                        pConnection,
                        &bytesRemaining,
                        &pPacket,
                        &pZctExecContext);
        BAIL_ON_NT_STATUS(ntStatus);

        // Note that a ZCT packet is not signed.

        if (pZctExecContext)
        {
            LWIO_ASSERT(pPacket == NULL);
            goto cleanup;
        }

        if (pPacket)
        {
            ntStatus = SrvProtocolTransportDriverCheckSignature(pConnection, pPacket);
            BAIL_ON_NT_STATUS(ntStatus);

            // allocate a new packet buffer
            ntStatus = SrvProtocolTransportDriverAllocatePacket(pConnection);
            BAIL_ON_NT_STATUS(ntStatus);

            if (bytesRemaining)
            {
                PVOID pFromBuffer = NULL;

                pFromBuffer = LwRtlOffsetToPointer(
                                    pPacket->pRawBuffer,
                                    pPacket->bufferUsed);

                // need to copy over the bytes into allocated packet
                RtlCopyMemory(
                        pConnection->readerState.pRequestPacket->pRawBuffer,
                        pFromBuffer,
                        bytesRemaining);
            }

            // dispatch packet -- takes a reference
            // NOTE: Does unlock/lock pConnection if this is SMB1 NT CANCEL.
            ntStatus = SrvProtocolTransportDriverDispatchPacket(
                            pConnection,
                            pPacket);
            BAIL_ON_NT_STATUS(ntStatus);

            SMBPacketRelease(
                    pConnection->hPacketAllocator,
                    pPacket);
            pPacket = NULL;
        }
    }

    // Update remaining buffer space
    ntStatus = SrvProtocolTransportDriverUpdateBuffer(pConnection);
    BAIL_ON_NT_STATUS(ntStatus);

cleanup:

    LWIO_UNLOCK_RWMUTEX(bInLock, &pConnection->mutex);

    if (pfnZctCallback)
    {
        LWIO_ASSERT(STATUS_SUCCESS == ntStatus);
        pfnZctCallback(pZctCallbackContext, STATUS_SUCCESS);
    }

    if (pZctExecContext)
    {
        NTSTATUS ntStatus2 = STATUS_SUCCESS;

        // Do not give any buffer to the socket.
        SrvProtocolTransportDriverRemoveBuffer(pConnection);

        LWIO_UNLOCK_RWMUTEX(bInLock, &pConnection->mutex);

        // The packet execution will resume the connection as needed.
        ntStatus2 = SrvProtocolExecute(pZctExecContext);
        if (ntStatus2)
        {
            LWIO_LOG_ERROR("Failed to synchronously execute server task (status = 0x%08x)", ntStatus2);
            // TODO: Do we need to disconnect if we fail to execute?
            //       Need to check w/Sriram.
            LWIO_ASSERT(FALSE);
        }

        SrvReleaseExecContext(pZctExecContext);
    }

    return ntStatus;

error:

    // Do not give any buffer to the socket.
    SrvProtocolTransportDriverRemoveBuffer(pConnection);

    LWIO_UNLOCK_RWMUTEX(bInLock, &pConnection->mutex);

    SrvConnectionSetInvalid(pConnection);

    if (pPacket)
    {
        SMBPacketRelease(
                pConnection->hPacketAllocator,
                pPacket);
    }

    goto cleanup;
}

static
VOID
SrvProtocolTransportDriverConnectionDone(
    IN PSRV_CONNECTION pConnection,
    IN NTSTATUS Status
    )
{
    BOOLEAN bInLock = FALSE;
    PFN_SRV_PROTOCOL_SEND_COMPLETE pfnZctCallback = NULL;
    PVOID pZctCallbackContext = NULL;

    if (STATUS_CONNECTION_RESET == Status)
    {
        LWIO_LOG_DEBUG("Connection reset by peer '%s' (fd = %d)",
                SrvTransportSocketGetAddressString(pConnection->pSocket),
                SrvTransportSocketGetFileDescriptor(pConnection->pSocket));
    }


    LWIO_LOCK_RWMUTEX_EXCLUSIVE(bInLock, &pConnection->mutex);
    pfnZctCallback = SrvProtocolTransportDriverGetZctCallback(
                        pConnection,
                        &pZctCallbackContext);
    LWIO_UNLOCK_RWMUTEX(bInLock, &pConnection->mutex);

    if (pfnZctCallback)
    {
        LWIO_ASSERT(STATUS_SUCCESS != Status);
        pfnZctCallback(pZctCallbackContext, Status);
    }

    if (pConnection->resource.ulResourceId)
    {
        PSRV_RESOURCE pResource = NULL;

        SrvElementsUnregisterResource(
                pConnection->resource.ulResourceId,
                &pResource);

        LWIO_LOCK_RWMUTEX_EXCLUSIVE(bInLock, &gProtocolApiGlobals.mutex);

        LwRtlRBTreeRemove(
                gProtocolApiGlobals.pConnections,
                &pConnection->resource.ulResourceId);

        LWIO_UNLOCK_RWMUTEX(bInLock, &gProtocolApiGlobals.mutex);
    }

    SrvConnectionSetInvalid(pConnection);
    SrvConnectionRelease(pConnection);
}

static
NTSTATUS
SrvProtocolTransportDriverSendPrepare(
    IN PSRV_SEND_CONTEXT pSendContext
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PSRV_CONNECTION pConnection = pSendContext->pConnection;
    PSMB_PACKET pPacket = pSendContext->pPacket;

    // Sign the packet.

    if (SrvConnectionIsSigningActive_inlock(pConnection))
    {
        switch (pPacket->protocolVer)
        {
            case SMB_PROTOCOL_VERSION_1:

                pPacket->pSMBHeader->flags2 |= FLAG2_SECURITY_SIG;

                ntStatus = SMBPacketSign(
                                pPacket,
                                pPacket->sequence,
                                pConnection->pSessionKey,
                                pConnection->ulSessionKeyLength);

                break;

            case SMB_PROTOCOL_VERSION_2:

                if (pPacket->pSMB2Header->ullCommandSequence != -1)
                {
                    ntStatus = SMB2PacketSign(
                                   pPacket,
                                   pConnection->pSessionKey,
                                   pConnection->ulSessionKeyLength);
                }

                break;

            default:

                ntStatus = STATUS_INTERNAL_ERROR;

                break;
        }
        BAIL_ON_NT_STATUS(ntStatus);
    }

cleanup:

    return ntStatus;

error:

    goto cleanup;
}

static
VOID
SrvProtocolTransportDriverSendDone(
    IN PSRV_SEND_CONTEXT pSendContext,
    IN NTSTATUS Status
    )
{
    PFN_SRV_PROTOCOL_SEND_COMPLETE pfnCallback = NULL;
    PVOID pCallbackContext = NULL;

    // There is no need to close the socket here on an error Status as
    // the transport will take care of doing a ConnectionDone which will
    // trigger the closing of the socket.

    if (pSendContext->bIsZct)
    {
        pfnCallback = pSendContext->pfnCallback;
        pCallbackContext = pSendContext->pCallbackContext;
    }

    SrvProtocolTransportDriverFreeResources(pSendContext);

    if (pfnCallback)
    {
        pfnCallback(pCallbackContext, Status);
    }
}

static
VOID
SrvProtocolTransportDriverSocketFree(
    IN OUT PSRV_SOCKET pSocket
    )
{
    SrvTransportSocketClose(pSocket);
}

static
VOID
SrvProtocolTransportDriverSocketDisconnect(
    IN PSRV_SOCKET pSocket
    )
{
    SrvTransportSocketDisconnect(pSocket);
}

static
VOID
SrvProtocolTransportDriverSocketGetClientAddress(
    IN PSRV_SOCKET pSocket,
    OUT const struct sockaddr** ppAddress,
    OUT SOCKLEN_TYPE* pAddressLength
    )
{
    SrvTransportSocketGetAddress(pSocket, ppAddress, pAddressLength);
}

static
VOID
SrvProtocolTransportDriverSocketGetServerAddress(
    IN PSRV_SOCKET pSocket,
    OUT const struct sockaddr** ppAddress,
    OUT SOCKLEN_TYPE* pAddressLength
    )
{
    SrvTransportSocketGetServerAddress(pSocket, ppAddress, pAddressLength);
}

static
VOID
SrvProtocolTransportDriverSocketResetTimeout(
    IN PSRV_SOCKET pSocket,
    IN BOOLEAN bIsEnabled,
    IN ULONG ulTimeoutSeconds
    )
{
    SrvTransportSocketSetTimeout(pSocket,
                                 bIsEnabled,
                                 ulTimeoutSeconds);
}

static
NTSTATUS
SrvProtocolTransportDriverAllocatePacket(
    IN PSRV_CONNECTION pConnection
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    LWIO_ASSERT(!pConnection->readerState.pRequestPacket);

    ntStatus = SMBPacketAllocate(
                    pConnection->hPacketAllocator,
                    &pConnection->readerState.pRequestPacket);
    BAIL_ON_NT_STATUS(ntStatus);

    ntStatus = SMBPacketBufferAllocate(
                    pConnection->hPacketAllocator,
                    SMB_PACKET_DEFAULT_SIZE,
                    &pConnection->readerState.pRequestPacket->pRawBuffer,
                    &pConnection->readerState.pRequestPacket->bufferLen);
    BAIL_ON_NT_STATUS(ntStatus);

    pConnection->readerState.bNeedHeader = TRUE;
    pConnection->readerState.sNumBytesToRead = sizeof(NETBIOS_HEADER);
    pConnection->readerState.sOffset = 0;
    pConnection->readerState.pRequestPacket->bufferUsed = 0;
    pConnection->readerState.zctState = SRV_ZCT_STATE_UNKNOWN;

cleanup:

    return ntStatus;

error:

    SMBPacketRelease(
            pConnection->hPacketAllocator,
            pConnection->readerState.pRequestPacket);

    pConnection->readerState.pRequestPacket = NULL;

    goto cleanup;
}

static
ULONG
SrvProtocolTransportDriverGetMaxZctWriteFileHeader(
    IN SMB_PROTOCOL_VERSION ProtocolVersion
    )
{
    ULONG maximum = 0;

    // NOTE: Must match SrvDetectZctWrite_SMB_V1 and SrvDetectZctWrite_SMB_V2.
    switch (ProtocolVersion)
    {
    case SMB_PROTOCOL_VERSION_1:
        maximum = (sizeof(SMB_HEADER) +
                   LW_MAX(LW_FIELD_OFFSET(WRITE_REQUEST_HEADER, dataLength) +
			  LW_FIELD_SIZE(WRITE_REQUEST_HEADER, dataLength),
			  sizeof(ANDX_HEADER) + LW_FIELD_OFFSET(WRITE_ANDX_REQUEST_HEADER_WC_14, offsetHigh) +
			  LW_FIELD_SIZE(WRITE_ANDX_REQUEST_HEADER_WC_14, offsetHigh)));
        break;
    case SMB_PROTOCOL_VERSION_2:
        maximum = sizeof(SMB2_HEADER) + sizeof(SMB2_WRITE_REQUEST_HEADER);
        break;
    default:
        LWIO_ASSERT(FALSE);
        break;
    }

    return maximum;
}

static
NTSTATUS
SrvProtocolTransportDriverUpdateBuffer(
    IN PSRV_CONNECTION pConnection
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PVOID pBuffer = NULL;
    ULONG Size = 0;
    ULONG Minimum = 0;

    // TODO-Perhaps remove sOffset as it appears to be exactly pRequestPacket->bufferUsed.
    LWIO_ASSERT(pConnection->readerState.sOffset == pConnection->readerState.pRequestPacket->bufferUsed);
    LWIO_ASSERT(pConnection->readerState.pRequestPacket->bufferLen >= pConnection->readerState.pRequestPacket->bufferUsed);

    pBuffer = LwRtlOffsetToPointer(
                    pConnection->readerState.pRequestPacket->pRawBuffer,
                    pConnection->readerState.pRequestPacket->bufferUsed);
    Size = pConnection->readerState.pRequestPacket->bufferLen - pConnection->readerState.pRequestPacket->bufferUsed;
    Minimum = pConnection->readerState.sNumBytesToRead;

    // Adjust min/max in case of potential ZCT for SMB write file
    if ((pConnection->serverProperties.ulZctWriteThreshold > 0) &&
        (pConnection->protocolVer != SMB_PROTOCOL_VERSION_UNKNOWN) &&
        !SrvConnectionIsSigningActive_inlock(pConnection))
    {
        ULONG maxHeader = SrvProtocolTransportDriverGetMaxZctWriteFileHeader(pConnection->protocolVer);
        LWIO_ASSERT(maxHeader > 0);

        //
        // Cases:
        //
        // 1) Need framing header --> max read to just include potential
        //    ZCT write file header.
        //
        // 2) Need rest of packet & unknown ZCT state --> (a) min read to
        //    just include potential ZCT write file header and (b) max read
        //    to just include this packet plus next framing and potential
        //    ZCT write file header.
        //
        // 3) Need rest of packet & known ZCT state --> max read to just
        //    include this packet plus next framing and potential ZCT
        //    write file header.
        //
        if (pConnection->readerState.bNeedHeader)
        {
            // Case 1
            LWIO_ASSERT(Minimum <= sizeof(NETBIOS_HEADER));
            Size = LW_MIN(Size, Minimum + maxHeader);
        }
        else
        {
            // Cases 2 and 3:

            Size = LW_MIN(Size, Minimum + sizeof(NETBIOS_HEADER) + maxHeader);

            if (pConnection->readerState.zctState == SRV_ZCT_STATE_UNKNOWN)
            {
                // Case 2 also needs to set the min
                LWIO_ASSERT(pConnection->readerState.pRequestPacket->bufferUsed <= (maxHeader + sizeof(NETBIOS_HEADER)));

                Minimum = LW_MIN(Minimum, (maxHeader + sizeof(NETBIOS_HEADER)) - pConnection->readerState.pRequestPacket->bufferUsed);
            }
        }
    }

    // TODO-Test out setting Size = Minimum (perhaps registry config) -- IFF not doing ZCT SMB write support.
    ntStatus = SrvTransportSocketSetBuffer(
                    pConnection->pSocket,
                    pBuffer,
                    Size,
                    Minimum);
    BAIL_ON_NT_STATUS(ntStatus);

cleanup:

    return ntStatus;

error:

    goto cleanup;
}

static
VOID
SrvProtocolTransportDriverRemoveBuffer(
    IN PSRV_CONNECTION pConnection
    )
{
    // Do not give any buffer to the socket.
    SrvTransportSocketSetBuffer(
            pConnection->pSocket,
            NULL,
            0,
            0);
}

static
NTSTATUS
SrvProtocolTransportDriverDetectPacket(
    IN PSRV_CONNECTION pConnection,
    IN OUT PULONG pulBytesAvailable,
    OUT PSMB_PACKET* ppPacket,
    OUT PSRV_EXEC_CONTEXT* ppZctExecContext
    )
{
    NTSTATUS ntStatus = 0;
    ULONG ulBytesAvailable = *pulBytesAvailable;
    PSMB_PACKET pPacketFound = NULL;
    PSRV_EXEC_CONTEXT pZctExecContext = NULL;

    LWIO_ASSERT(ulBytesAvailable > 0);

    if (pConnection->readerState.ulSkipBytes)
    {
        size_t sNumBytesRead = LW_MIN(pConnection->readerState.ulSkipBytes, ulBytesAvailable);

        LWIO_ASSERT(pConnection->readerState.bNeedHeader);
        LWIO_ASSERT(0 == pConnection->readerState.pRequestPacket->bufferUsed);

        if (ulBytesAvailable > sNumBytesRead)
        {
            PVOID pRemainder = LwRtlOffsetToPointer(
                                    pConnection->readerState.pRequestPacket->pRawBuffer,
                                    sNumBytesRead);
            RtlMoveMemory(pConnection->readerState.pRequestPacket->pRawBuffer,
                          pRemainder,
                          ulBytesAvailable - sNumBytesRead);
        }

        pConnection->readerState.ulSkipBytes -= sNumBytesRead;
        ulBytesAvailable -= sNumBytesRead;
    }

    if (ulBytesAvailable && pConnection->readerState.bNeedHeader)
    {
        size_t sNumBytesRead = LW_MIN(pConnection->readerState.sNumBytesToRead, ulBytesAvailable);

        pConnection->readerState.sNumBytesToRead -= sNumBytesRead;
        pConnection->readerState.sOffset += sNumBytesRead;
        pConnection->readerState.pRequestPacket->bufferUsed += sNumBytesRead;

        if (!pConnection->readerState.sNumBytesToRead)
        {
            PSMB_PACKET pPacket = pConnection->readerState.pRequestPacket;
            ULONG ulPacketBytesAvailable = pPacket->bufferLen - sizeof(NETBIOS_HEADER);

            pConnection->readerState.bNeedHeader = FALSE;

            pPacket->pNetBIOSHeader = (NETBIOS_HEADER *) pPacket->pRawBuffer;
            pPacket->pNetBIOSHeader->len = ntohl(pPacket->pNetBIOSHeader->len);

            if (pConnection->pServerAddress->sa_family == AF_INET &&
                ((struct sockaddr_in *)pConnection->pServerAddress)->sin_port
                == htons(NETBIOS_SERVER_PORT))
            {
                // A netbios session can only be 17 bits in length
                pPacket->netbiosOpcode = (pPacket->pNetBIOSHeader->len>>24);
                pPacket->pNetBIOSHeader->len &= 0xFFFFFF;
                if (pPacket->pNetBIOSHeader->len & 0xFE0000)
                {
                    // Unknown flags
                    ntStatus = STATUS_INVALID_NETWORK_RESPONSE;
                    BAIL_ON_NT_STATUS(ntStatus);
                }
            }
            else if (pPacket->pNetBIOSHeader->len > 0xFFFFFF)
            {
                // "Naked" SMB can only be 24 bits in length.  This packet
                // is too large
                ntStatus = STATUS_INVALID_NETWORK_RESPONSE;
                BAIL_ON_NT_STATUS(ntStatus);
            }

            pConnection->readerState.sNumBytesToRead = pPacket->pNetBIOSHeader->len;

            // check if the message fits in our currently allocated buffer
            if (pConnection->readerState.sNumBytesToRead > ulPacketBytesAvailable)
            {
                ntStatus = STATUS_INVALID_BUFFER_SIZE;
                BAIL_ON_NT_STATUS(ntStatus);
            }
        }

        ulBytesAvailable -= sNumBytesRead;
    }

    if (ulBytesAvailable &&
        !pConnection->readerState.bNeedHeader &&
        pConnection->readerState.sNumBytesToRead)
    {
        size_t sNumBytesRead = LW_MIN(pConnection->readerState.sNumBytesToRead, ulBytesAvailable);

        pConnection->readerState.sNumBytesToRead            -= sNumBytesRead;
        pConnection->readerState.sOffset                    += sNumBytesRead;
        pConnection->readerState.pRequestPacket->bufferUsed += sNumBytesRead;

        ulBytesAvailable -= sNumBytesRead;
    }

    if (pConnection->readerState.pRequestPacket->netbiosOpcode !=
         SRV_NETBIOS_OPCODE_SESSION_MESSAGE &&
         !pConnection->readerState.sNumBytesToRead)
    {
        // Netbios packet

        pPacketFound = pConnection->readerState.pRequestPacket;
        pConnection->readerState.pRequestPacket = NULL;
    }
    else if (!pConnection->readerState.bNeedHeader &&
        !pConnection->readerState.sNumBytesToRead)
    {
        // Packet is complete

        PSMB_PACKET pPacket    = pConnection->readerState.pRequestPacket;
        PBYTE pBuffer          = pPacket->pRawBuffer + sizeof(NETBIOS_HEADER);
        ULONG ulBytesAvailable = pPacket->bufferUsed - sizeof(NETBIOS_HEADER);

        switch (*pBuffer)
        {
            case 0xFF:
                pPacket->protocolVer = SMB_PROTOCOL_VERSION_1;

                if (ulBytesAvailable < sizeof(SMB_HEADER))
                {
                    ntStatus = STATUS_INVALID_NETWORK_RESPONSE;
                    BAIL_ON_NT_STATUS(ntStatus);
                }

                pPacket->pSMBHeader = (PSMB_HEADER)(pBuffer);
                pBuffer            += sizeof(SMB_HEADER);
                ulBytesAvailable   -= sizeof(SMB_HEADER);

                if (SMBIsAndXCommand(pPacket->pSMBHeader->command))
                {
                    if (ulBytesAvailable < sizeof(ANDX_HEADER))
                    {
                        ntStatus = STATUS_INVALID_NETWORK_RESPONSE;
                        BAIL_ON_NT_STATUS(ntStatus);
                    }

                    pPacket->pAndXHeader = (PANDX_HEADER)pBuffer;
                    pBuffer             += sizeof(ANDX_HEADER);
                    ulBytesAvailable    -= sizeof(ANDX_HEADER);
                }

                pPacket->pParams = (ulBytesAvailable > 0) ? pBuffer : NULL;
                pPacket->pData = NULL;

                break;

            case 0xFE:

                if (SrvProtocolConfigIsSmb2Enabled())
                {
                    pPacket->protocolVer = SMB_PROTOCOL_VERSION_2;

                    if (ulBytesAvailable < sizeof(SMB2_HEADER))
                    {
                        ntStatus = STATUS_INVALID_NETWORK_RESPONSE;
                        BAIL_ON_NT_STATUS(ntStatus);
                    }

                    pPacket->pSMB2Header = (PSMB2_HEADER)pBuffer;
                    pBuffer             += sizeof(SMB2_HEADER);
                    ulBytesAvailable    -= sizeof(SMB2_HEADER);

                    pPacket->pParams    = NULL;
                    pPacket->pData      = NULL;
                }
                else
                {
                    ntStatus = STATUS_INVALID_NETWORK_RESPONSE;
                    BAIL_ON_NT_STATUS(ntStatus);
                }

                break;

            default:
                ntStatus = STATUS_INVALID_NETWORK_RESPONSE;
                BAIL_ON_NT_STATUS(ntStatus);

                break;
        }

        switch (pConnection->protocolVer)
        {
            case SMB_PROTOCOL_VERSION_UNKNOWN:
                ntStatus = SrvConnectionSetProtocolVersion_inlock(
                                pConnection,
                                pPacket->protocolVer);
                break;

            default:
                if (pConnection->protocolVer != pPacket->protocolVer)
                {
                    ntStatus = STATUS_INVALID_NETWORK_RESPONSE;
                }
                break;
        }
        BAIL_ON_NT_STATUS(ntStatus);

        pPacketFound = pConnection->readerState.pRequestPacket;
        pConnection->readerState.pRequestPacket = NULL;
    }
    // Partial packet with known protocol version
    else if (!pConnection->readerState.bNeedHeader &&
             (pConnection->serverProperties.ulZctWriteThreshold > 0) &&
             (pConnection->readerState.zctState == SRV_ZCT_STATE_UNKNOWN) &&
             ((pConnection->state == LWIO_SRV_CONN_STATE_NEGOTIATE) ||
              (pConnection->state == LWIO_SRV_CONN_STATE_READY)) &&
             (pConnection->protocolVer != SMB_PROTOCOL_VERSION_UNKNOWN) &&
             !SrvConnectionIsSigningActive_inlock(pConnection))
    {
        PSMB_PACKET pPacket = pConnection->readerState.pRequestPacket;

        switch (pConnection->protocolVer)
        {
            case SMB_PROTOCOL_VERSION_1:
                ntStatus = SrvDetectZctWrite_SMB_V1(
                                pConnection,
                                pPacket,
                                &pZctExecContext);
                break;
            case SMB_PROTOCOL_VERSION_2:
                ntStatus = SrvDetectZctWrite_SMB_V2(
                                pConnection,
                                pPacket,
                                &pZctExecContext);
                break;
            default:
                ntStatus = STATUS_SUCCESS;
                break;

        }
        if (ntStatus == STATUS_MORE_PROCESSING_REQUIRED)
        {
            ntStatus = STATUS_SUCCESS;
        }
        else
        {
            BAIL_ON_NT_STATUS(ntStatus);

            if (pZctExecContext)
            {
                pConnection->readerState.zctState = SRV_ZCT_STATE_IS_ZCT;

                pPacketFound = pConnection->readerState.pRequestPacket;
                pConnection->readerState.pRequestPacket = NULL;
            }
            else
            {
                pConnection->readerState.zctState = SRV_ZCT_STATE_NOT_ZCT;
            }
        }
    }

    if (pZctExecContext)
    {
        ntStatus = SrvProtocolTransportDriverSetStatistics(
                        pConnection,
                        pPacketFound->protocolVer,
                        pPacketFound->bufferUsed,
                        &pZctExecContext->pStatInfo);
        BAIL_ON_NT_STATUS(ntStatus);

        ntStatus = SrvMpxTrackerAddExecContext_inlock(
                        pConnection,
                        pZctExecContext);
        BAIL_ON_NT_STATUS(ntStatus);

        // pZctExecContext is holding a ref, drop ours
        SMBPacketRelease(
                pConnection->hPacketAllocator,
                pPacketFound);
        pPacketFound = NULL;
    }

cleanup:

    if (pZctExecContext || pPacketFound)
    {
        SrvConnectionResetIdleTimeout(pConnection);
    }

    *pulBytesAvailable = ulBytesAvailable;
    *ppPacket = pPacketFound;
    *ppZctExecContext = pZctExecContext;

    return ntStatus;

error:

    pPacketFound = NULL;
    ulBytesAvailable = 0;

    goto cleanup;
}

static
NTSTATUS
SrvProtocolTransportDriverDispatchPacket(
    IN PSRV_CONNECTION pConnection,
    IN PSMB_PACKET pPacket
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PSRV_EXEC_CONTEXT pContext = NULL;
    BOOLEAN bInLock = TRUE;

    if (pConnection->readerState.pContinueExecContext)
    {
        pContext = pConnection->readerState.pContinueExecContext;
        pConnection->readerState.pContinueExecContext = NULL;
    }
    else
    {
        // Note that building the context takes its own reference on the packet.
        ntStatus = SrvBuildExecContext(pConnection, pPacket, FALSE, &pContext);
        BAIL_ON_NT_STATUS(ntStatus);

        ntStatus = SrvProtocolTransportDriverSetStatistics(
                        pConnection,
                        pPacket->protocolVer,
                        pPacket->bufferUsed,
                        &pContext->pStatInfo);
        BAIL_ON_NT_STATUS(ntStatus);

        if (!SrvMpxTrackerIsNtCancelPacket(pPacket))
        {
            ntStatus = SrvMpxTrackerAddExecContext_inlock(pConnection, pContext);
            BAIL_ON_NT_STATUS(ntStatus);
        }
    }

    // Directly process in line if possible
    LWIO_UNLOCK_RWMUTEX(bInLock, &pConnection->mutex);
    ntStatus =  SrvProtocolExecute(pContext);
    LWIO_LOCK_RWMUTEX_EXCLUSIVE(bInLock, &pConnection->mutex);

cleanup:

    if (pContext)
    {
        SrvReleaseExecContext(pContext);
    }

    return ntStatus;

error:

    goto cleanup;
}

static
NTSTATUS
SrvProtocolTransportDriverSetStatistics(
    PLWIO_SRV_CONNECTION  pConnection,     /* IN     */
    SMB_PROTOCOL_VERSION  protocolVersion, /* IN     */
    ULONG                 ulRequestLength, /* IN     */
    PSRV_STAT_INFO*       ppStatInfo       /*    OUT */
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PSRV_STAT_INFO pStatInfo = NULL;
    SRV_STAT_CONNECTION_INFO statConnInfo =
    {
            .pClientAddress = pConnection->pClientAddress,
            .clientAddrLen = pConnection->clientAddrLen,
            .pServerAddress = pConnection->pServerAddress,
            .serverAddrLen = pConnection->serverAddrLen,
            .ulResourceId  = pConnection->resource.ulResourceId
    };
    SRV_STAT_SMB_VERSION protocolStatVer = SRV_STAT_SMB_VERSION_UNKNOWN;

    switch (protocolVersion)
    {
        case SMB_PROTOCOL_VERSION_1 :

            protocolStatVer = SRV_STAT_SMB_VERSION_1;

            break;

        case SMB_PROTOCOL_VERSION_2 :

            protocolStatVer = SRV_STAT_SMB_VERSION_2;

            break;

        case SMB_PROTOCOL_VERSION_UNKNOWN:

            protocolStatVer = SRV_STAT_SMB_VERSION_UNKNOWN;

            break;
    }

    ntStatus = SrvStatisticsCreateRequestContext(
                    &statConnInfo,
                    protocolStatVer,
                    ulRequestLength,
                    &pStatInfo);
    BAIL_ON_NT_STATUS(ntStatus);

    *ppStatInfo = pStatInfo;

cleanup:

    return ntStatus;

error:

    *ppStatInfo = NULL;

    if (pStatInfo)
    {
        SrvStatisticsCloseRequestContext(pStatInfo);
    }

    goto cleanup;
}

static
NTSTATUS
SrvProtocolTransportDriverCheckSignature(
    IN PSRV_CONNECTION pConnection,
    IN PSMB_PACKET pPacket
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    // Already in pConnection lock.

    if(pPacket->netbiosOpcode != SRV_NETBIOS_OPCODE_SESSION_MESSAGE)
    {
        // You can't sign a netbios packet.  Just say "yes"

       ntStatus = STATUS_SUCCESS;
        goto cleanup;
    }

    switch (pConnection->protocolVer)
    {
        case SMB_PROTOCOL_VERSION_1:
            // Update the sequence whether we end up signing or not
            pPacket->sequence = SrvProtocolTransportDriverGetNextSequence(
                                    pConnection,
                                    pPacket);
            break;

        default:
            break;
    }
    BAIL_ON_NT_STATUS(ntStatus);

    if (SrvConnectionIsSigningActive_inlock(pConnection))
    {
        switch (pConnection->protocolVer)
        {
            case SMB_PROTOCOL_VERSION_1:
                ntStatus = SMBPacketVerifySignature(
                                pPacket,
                                pPacket->sequence,
                                pConnection->pSessionKey,
                                pConnection->ulSessionKeyLength);

                break;

            case SMB_PROTOCOL_VERSION_2:
                // Allow CANCEL and ECHO requests to be unsigned
                if (SMB2PacketIsSigned(pPacket))
                {
                    ntStatus = SMB2PacketVerifySignature(
                        pPacket,
                        pConnection->pSessionKey,
                        pConnection->ulSessionKeyLength);
                }
                else if (pPacket->pSMB2Header->command != COM2_CANCEL &&
                         pPacket->pSMB2Header->command != COM2_ECHO)
                {
                    ntStatus = STATUS_INVALID_NETWORK_RESPONSE;
                    BAIL_ON_NT_STATUS(ntStatus);
                }

                break;

            default:
                ntStatus = STATUS_INTERNAL_ERROR;

                break;
        }
        BAIL_ON_NT_STATUS(ntStatus);
    }

cleanup:

    return ntStatus;

error:

    goto cleanup;
}

static
ULONG
SrvProtocolTransportDriverGetNextSequence(
    IN PSRV_CONNECTION pConnection,
    IN PSMB_PACKET pPacket
    )
{
    ULONG ulRequestSequence = 0;

    // Already in pConnection lock.

    switch (pPacket->pSMBHeader->command)
    {
        case COM_NEGOTIATE:

            break;

        case COM_NT_CANCEL:

            ulRequestSequence = pConnection->ulSequence++;

            break;

        case COM_SESSION_SETUP_ANDX:

            /* Sequence number increments don't start until the last leg
               of the first successful session setup */

            if (pConnection->state == LWIO_SRV_CONN_STATE_NEGOTIATE)
            {
                ulRequestSequence = 0;
                pConnection->ulSequence = 2;
            }
            else
            {
                ulRequestSequence = pConnection->ulSequence;
                pConnection->ulSequence += 2;
            }

            break;

        default:

            ulRequestSequence = pConnection->ulSequence;
            pConnection->ulSequence += 2;

            break;
    }

    return ulRequestSequence;
}

static
VOID
SrvProtocolTransportDriverFreeResources(
    IN PSRV_SEND_CONTEXT pSendContext
    )
{
    if (pSendContext->pStatInfo)
    {
        SrvStatisticsSetResponseInfo(
            pSendContext->pStatInfo,
            pSendContext->bIsZct ? LwZctGetLength(pSendContext->pZct) :
                    pSendContext->pPacket->bufferUsed);

        SrvStatisticsRelease(pSendContext->pStatInfo);
    }

    if (!pSendContext->bIsZct)
    {
        SMBPacketRelease(
            pSendContext->pConnection->hPacketAllocator,
            pSendContext->pPacket);
    }

    SrvConnectionRelease(pSendContext->pConnection);

    SrvFreeMemory(pSendContext);
}

static
PFN_SRV_CONNECTION_IO_COMPLETE
SrvProtocolTransportDriverGetZctCallback(
    IN PLWIO_SRV_CONNECTION pConnection,
    OUT PVOID* ppCallbackContext
    )
{
    PFN_SRV_CONNECTION_IO_COMPLETE pfnCallback = NULL;
    PVOID pCallbackContext = NULL;

    // pConnection lock must be held.

    if (pConnection->readerState.pfnZctCallback)
    {
        pfnCallback = pConnection->readerState.pfnZctCallback;
        pCallbackContext = pConnection->readerState.pZctCallbackContext;

        pConnection->readerState.pfnZctCallback = NULL;
        pConnection->readerState.pZctCallbackContext = NULL;
        pConnection->readerState.sNumBytesToRead = 0;
    }

    *ppCallbackContext = pCallbackContext;

    return pfnCallback;
}

NTSTATUS
SrvProtocolTransportSendResponse(
    IN PLWIO_SRV_CONNECTION pConnection,
    IN PSMB_PACKET pPacket,
    IN PSRV_STAT_INFO pStatInfo
    )
{
    NTSTATUS ntStatus = 0;
    PSRV_SEND_CONTEXT pSendContext = NULL;

    ntStatus = SrvAllocateMemory(sizeof(*pSendContext), OUT_PPVOID(&pSendContext));
    BAIL_ON_NT_STATUS(ntStatus);

    pSendContext->pConnection = pConnection;
    SrvConnectionAcquire(pConnection);

    if (pStatInfo)
    {
        pSendContext->pStatInfo = SrvStatisticsAcquire(pStatInfo);
    }

    // TODO-Should remove refcounting from pPacket altogether?
    pSendContext->pPacket = pPacket;
    InterlockedIncrement(&pPacket->refCount);

    ntStatus = SrvTransportSocketSendReply(
                    pConnection->pSocket,
                    pSendContext,
                    pSendContext->pPacket->pRawBuffer,
                    pSendContext->pPacket->bufferUsed);
    BAIL_ON_NT_STATUS(ntStatus);

    SrvProtocolTransportDriverFreeResources(pSendContext);

cleanup:

    // This function never returns pending because the caller
    // does not have a callback mechanism.
    if (ntStatus == STATUS_PENDING)
    {
        ntStatus = STATUS_SUCCESS;
    }

    return ntStatus;

error:

    if (ntStatus != STATUS_PENDING)
    {
        if (pSendContext)
        {
            SrvProtocolTransportDriverFreeResources(pSendContext);
        }

        // This will trigger rundown in protocol code.
        SrvConnectionSetInvalid(pConnection);
    }

    goto cleanup;
}

NTSTATUS
SrvProtocolTransportSendZctResponse(
    IN PLWIO_SRV_CONNECTION pConnection,
    IN PLW_ZCT_VECTOR pZct,
    IN PSRV_STAT_INFO pStatInfo,
    IN OPTIONAL PFN_SRV_PROTOCOL_SEND_COMPLETE pfnCallback,
    IN OPTIONAL PVOID pCallbackContext
    )
{
    NTSTATUS ntStatus = 0;
    PSRV_SEND_CONTEXT pSendContext = NULL;

    LWIO_ASSERT(!pCallbackContext || pfnCallback);

    ntStatus = SrvAllocateMemory(sizeof(*pSendContext), OUT_PPVOID(&pSendContext));
    BAIL_ON_NT_STATUS(ntStatus);

    pSendContext->pConnection = pConnection;
    SrvConnectionAcquire(pConnection);

    if (pStatInfo)
    {
        pSendContext->pStatInfo = SrvStatisticsAcquire(pStatInfo);
    }

    pSendContext->bIsZct = TRUE;

    pSendContext->pZct = pZct;
    pSendContext->pfnCallback = pfnCallback;
    pSendContext->pCallbackContext = pCallbackContext;

    ntStatus = SrvTransportSocketSendZctReply(
                    pConnection->pSocket,
                    pSendContext,
                    pSendContext->pZct);
    BAIL_ON_NT_STATUS(ntStatus);

    SrvProtocolTransportDriverFreeResources(pSendContext);

cleanup:

    return ntStatus;

error:

    if (ntStatus != STATUS_PENDING)
    {
        if (pSendContext)
        {
            SrvProtocolTransportDriverFreeResources(pSendContext);
        }

        // This will trigger rundown in protocol code.
        SrvConnectionSetInvalid(pConnection);
    }

    goto cleanup;
}

NTSTATUS
SrvProtocolTransportContinueAsNonZct(
    IN PLWIO_SRV_CONNECTION pConnection,
    IN PSRV_EXEC_CONTEXT pZctExecContext
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    BOOLEAN bInLock = FALSE;

    LWIO_LOCK_RWMUTEX_EXCLUSIVE(bInLock, &pConnection->mutex);
    pConnection->readerState.zctState = SRV_ZCT_STATE_NOT_ZCT;
    pConnection->readerState.pRequestPacket = pZctExecContext->pSmbRequest;
    SrvAcquireExecContext(pZctExecContext);
    pConnection->readerState.pContinueExecContext = pZctExecContext;

    // Update remaining buffer space
    ntStatus = SrvProtocolTransportDriverUpdateBuffer(pConnection);
    // should never fail as there are no invalid params.
    LWIO_ASSERT(STATUS_SUCCESS == ntStatus);

    LWIO_UNLOCK_RWMUTEX(bInLock, &pConnection->mutex);

    // TODO -- Add support for receiving sycnhronously?

    ntStatus = STATUS_PENDING;

    return ntStatus;
}

NTSTATUS
SrvProtocolTransportReceiveZct(
    IN PLWIO_SRV_CONNECTION pConnection,
    IN PLW_ZCT_VECTOR pZct,
    IN PFN_SRV_CONNECTION_IO_COMPLETE pfnCallback,
    IN PVOID pCallbackContext
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    BOOLEAN bInLock = FALSE;

    LWIO_ASSERT(pfnCallback);

    if (!pfnCallback)
    {
        ntStatus = STATUS_INVALID_PARAMETER;
        BAIL_ON_NT_STATUS(ntStatus);
    }

    // TODO-remove lock from here as there can be only one ZCT write
    // on a connection that is trying to read from the socket.
    LWIO_LOCK_RWMUTEX_EXCLUSIVE(bInLock, &pConnection->mutex);
    pConnection->readerState.pfnZctCallback = pfnCallback;
    pConnection->readerState.pZctCallbackContext = pCallbackContext;
    LWIO_ASSERT(LwZctGetRemaining(pZct) == pConnection->readerState.sNumBytesToRead);
    LWIO_UNLOCK_RWMUTEX(bInLock, &pConnection->mutex);

    ntStatus = SrvTransportSocketReceiveZct(pConnection->pSocket, pZct);
    BAIL_ON_NT_STATUS(ntStatus);

    LWIO_LOCK_RWMUTEX_EXCLUSIVE(bInLock, &pConnection->mutex);
    pConnection->readerState.pfnZctCallback = NULL;
    pConnection->readerState.pZctCallbackContext = NULL;
    pConnection->readerState.sNumBytesToRead = 0;
    LWIO_UNLOCK_RWMUTEX(bInLock, &pConnection->mutex);

cleanup:

    LWIO_UNLOCK_RWMUTEX(bInLock, &pConnection->mutex);

    return ntStatus;

error:

    if (ntStatus != STATUS_PENDING)
    {
        LWIO_LOCK_RWMUTEX_EXCLUSIVE(bInLock, &pConnection->mutex);
        pConnection->readerState.pfnZctCallback = NULL;
        pConnection->readerState.pZctCallbackContext = NULL;
        LWIO_UNLOCK_RWMUTEX(bInLock, &pConnection->mutex);

        // This will trigger rundown in protocol code.
        SrvConnectionSetInvalid(pConnection);
    }

    goto cleanup;
}

NTSTATUS
SrvProtocolTransportResumeFromZct(
    IN PLWIO_SRV_CONNECTION pConnection
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    BOOLEAN bInLock = FALSE;

    LWIO_LOCK_RWMUTEX_EXCLUSIVE(bInLock, &pConnection->mutex);

    LWIO_ASSERT(pConnection->readerState.zctState == SRV_ZCT_STATE_IS_ZCT);
    pConnection->readerState.zctState = SRV_ZCT_STATE_NOT_ZCT;

    // Handle draining if ZCT transfer was not done (as in case
    // of an error prior to transfer).
    pConnection->readerState.ulSkipBytes = pConnection->readerState.sNumBytesToRead;
    pConnection->readerState.sNumBytesToRead = 0;

    ntStatus = SrvProtocolTransportDriverAllocatePacket(pConnection);
    BAIL_ON_NT_STATUS(ntStatus);

    // Update remaining buffer space
    ntStatus = SrvProtocolTransportDriverUpdateBuffer(pConnection);
    // should never fail as there are no invalid params.
    LWIO_ASSERT(STATUS_SUCCESS == ntStatus);

cleanup:

    LWIO_UNLOCK_RWMUTEX(bInLock, &pConnection->mutex);

    return ntStatus;

error:

    LWIO_UNLOCK_RWMUTEX(bInLock, &pConnection->mutex);

    // This will trigger rundown in protocol code.
    SrvConnectionSetInvalid(pConnection);

    goto cleanup;
}

