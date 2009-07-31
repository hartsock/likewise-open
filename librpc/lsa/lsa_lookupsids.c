/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 */

/*
 * Copyright Likewise Software    2004-2008
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the license, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.  You should have received a copy
 * of the GNU Lesser General Public License along with this program.  If
 * not, see <http://www.gnu.org/licenses/>.
 *
 * LIKEWISE SOFTWARE MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING
 * TERMS AS WELL.  IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT
 * WITH LIKEWISE SOFTWARE, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE
 * TERMS OF THAT SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE GNU
 * LESSER GENERAL PUBLIC LICENSE, NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU
 * HAVE QUESTIONS, OR WISH TO REQUEST A COPY OF THE ALTERNATE LICENSING
 * TERMS OFFERED BY LIKEWISE SOFTWARE, PLEASE CONTACT LIKEWISE SOFTWARE AT
 * license@likewisesoftware.com
 */

/*
 * Copyright (C) Likewise Software. All rights reserved.
 *
 * Module Name:
 *
 *        lsa_lookupsids.c
 *
 * Abstract:
 *
 *        Remote Procedure Call (RPC) Client Interface
 *
 *        LsaLookupSids function
 *
 * Authors: Rafal Szczesniak (rafal@likewise.com)
 */

#include "includes.h"


NTSTATUS
LsaLookupSids(
    IN  handle_t hBinding,
    IN  PolicyHandle *phPolicy,
    IN  SidArray *pSids,
    OUT RefDomainList **ppRefDomList,
    OUT TranslatedName **ppTransNames,
    IN  UINT16 Level,
    IN OUT UINT32 *Count
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    NTSTATUS ntRetStatus = STATUS_SUCCESS;
    TranslatedNameArray NameArray = {0};
    RefDomainList *pRefDomains = NULL;
    TranslatedName *pOutNames = NULL;
    RefDomainList *pOutDomains = NULL;

    BAIL_ON_INVALID_PTR(hBinding, ntStatus);
    BAIL_ON_INVALID_PTR(phPolicy, ntStatus);
    BAIL_ON_INVALID_PTR(pSids, ntStatus);
    BAIL_ON_INVALID_PTR(ppRefDomList, ntStatus);
    BAIL_ON_INVALID_PTR(ppTransNames, ntStatus);
    BAIL_ON_INVALID_PTR(Count, ntStatus);

    /* windows allows Level to be in range 1-6 */

    *Count = 0;

    DCERPC_CALL(ntStatus, _LsaLookupSids(
                              hBinding,
                              phPolicy,
                              pSids,
                              &pRefDomains,
                              &NameArray,
                              Level,
                              Count));
    ntRetStatus = ntStatus;

    /* Status other than success doesn't have to mean failure here */
    if (ntRetStatus != STATUS_SUCCESS &&
        ntRetStatus != LW_STATUS_SOME_NOT_MAPPED) goto error;

    ntStatus = LsaAllocateTranslatedNames(&pOutNames, &NameArray);
    BAIL_ON_NT_STATUS(ntStatus);

    ntStatus = LsaAllocateRefDomainList(&pOutDomains, pRefDomains);
    BAIL_ON_NT_STATUS(ntStatus);

    *ppTransNames = pOutNames;
    *ppRefDomList = pOutDomains;

cleanup:

    /* Free pointers returned from stub */
    if (pRefDomains) {
        LsaFreeStubRefDomainList(pRefDomains);
    }

    LsaCleanStubTranslatedNameArray(&NameArray);

    if (ntStatus == STATUS_SUCCESS &&
        (ntRetStatus == STATUS_SUCCESS ||
         ntRetStatus == LW_STATUS_SOME_NOT_MAPPED)) {
        ntStatus = ntRetStatus;
    }

    return ntStatus;

error:
    LsaRpcFreeMemory((PVOID)pOutNames);
    LsaRpcFreeMemory((PVOID)pOutDomains);

    *ppTransNames = NULL;
    *ppRefDomList = NULL;

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
