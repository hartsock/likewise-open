/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright Likewise Software    2004-2009
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
 *  Copyright (C) Likewise Software. All rights reserved.
 *
 *  Module Name:
 *
 *     pstoresec.c
 *
 *  Abstract:
 *
 *        Likewise Password Storage (LWPS)
 *
 *        Security descriptor utility for registry pstore provider
 *
 *  Authors: Adam Bernstein (abernstein@likewise.com)
 *           Wei Fu (wfu@likewise.com)
 */
#include "includes.h"


static
NTSTATUS
RegDB_BuildRestrictedDaclForKey(
    OUT PACL *ppDacl
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    DWORD dwSizeDacl = 0;
    PSID pRootSid = NULL;
    DWORD dwSidCount = 0;
    PACL pDacl = NULL;

    status = LwMapSecurityGetSidFromId(gpRegLwMapSecurityCtx,
		                           &pRootSid,
		                           TRUE,
                                       0);
    BAIL_ON_LWPS_ERROR(status);
    dwSidCount++;

    dwSizeDacl = ACL_HEADER_SIZE +
        dwSidCount * sizeof(ACCESS_ALLOWED_ACE) +
        RtlLengthSid(pRootSid) +
        dwSidCount * sizeof(ULONG);

    status= LW_RTL_ALLOCATE(&pDacl, VOID, dwSizeDacl);
    BAIL_ON_LWPS_ERROR(status);

    status = RtlCreateAcl(pDacl, dwSizeDacl, ACL_REVISION);
    BAIL_ON_LWPS_ERROR(status);

    status = RtlAddAccessAllowedAceEx(pDacl,
                                      ACL_REVISION,
                                      0,
                                      KEY_ALL_ACCESS |
                                      DELETE |
                                      WRITE_OWNER |
                                      WRITE_DAC |
                                      READ_CONTROL,
                                      pRootSid);
    BAIL_ON_LWPS_ERROR(status);

    *ppDacl = pDacl;
    pDacl = NULL;

cleanup:
    LW_RTL_FREE(&pRootSid);
    LW_RTL_FREE(&pDacl);

    return status;

error:
    goto cleanup;
}


VOID
RegDB_FreeAbsoluteSecurityDescriptor(
    IN OUT PSECURITY_DESCRIPTOR_ABSOLUTE *ppSecDesc
    )
{
    PSID pOwner = NULL;
    PSID pGroup = NULL;
    PACL pDacl = NULL;
    PACL pSacl = NULL;
    BOOLEAN bDefaulted = FALSE;
    BOOLEAN bPresent = FALSE;
    PSECURITY_DESCRIPTOR_ABSOLUTE pSecDesc = NULL;

    if ((ppSecDesc == NULL) || (*ppSecDesc == NULL)) {
        return;
    }

    pSecDesc = *ppSecDesc;

    RtlGetOwnerSecurityDescriptor(pSecDesc, &pOwner, &bDefaulted);
    RtlGetGroupSecurityDescriptor(pSecDesc, &pGroup, &bDefaulted);
    RtlGetDaclSecurityDescriptor(pSecDesc, &bPresent, &pDacl, &bDefaulted);
    RtlGetSaclSecurityDescriptor(pSecDesc, &bPresent, &pSacl, &bDefaulted);

    LW_RTL_FREE(&pSecDesc);
    LW_RTL_FREE(&pOwner);
    LW_RTL_FREE(&pGroup);
    LW_RTL_FREE(&pDacl);
    LW_RTL_FREE(&pSacl);

    *ppSecDesc = NULL;

    return;
}


NTSTATUS
RegDB_CreateRestrictedSecDescAbs(
	IN OUT PSECURITY_DESCRIPTOR_ABSOLUTE *ppSecDescAbs
	)
{
    NTSTATUS status = STATUS_SUCCESS;
    PSECURITY_DESCRIPTOR_ABSOLUTE pSecDescAbs = NULL;
    PACL pDacl = NULL;
    PSID pSid = NULL;

    status = LW_RTL_ALLOCATE(&pSecDescAbs,
                            VOID,
                            SECURITY_DESCRIPTOR_ABSOLUTE_MIN_SIZE);
    BAIL_ON_LWPS_ERROR(status);

    status = RtlCreateSecurityDescriptorAbsolute(pSecDescAbs,
                                                 SECURITY_DESCRIPTOR_REVISION);
    BAIL_ON_LWPS_ERROR(status);

    // Owner: Root
    status = LwMapSecurityGetSidFromId(gpRegLwMapSecurityCtx,
		                           &pSid,
		                           TRUE,
                                       0);
    BAIL_ON_LWPS_ERROR(status);

    status = RtlSetOwnerSecurityDescriptor(pSecDescAbs,
                                            pSid,
                                            FALSE);
    BAIL_ON_LWPS_ERROR(status);
    pSid = NULL;

    // Group (no need to set group for registry key)

    // Do not set Sacl currently

    // DACL
    status = RegDB_BuildRestrictedDaclForKey(&pDacl);
    BAIL_ON_LWPS_ERROR(status);

    status = RtlSetDaclSecurityDescriptor(pSecDescAbs,
                                          TRUE,
                                          pDacl,
                                          FALSE);
    BAIL_ON_LWPS_ERROR(status);
    pDacl = NULL;

    if (!RtlValidSecurityDescriptor(pSecDescAbs))
    {
		status = STATUS_INVALID_SECURITY_DESCR;
		BAIL_ON_LWPS_ERROR(status);
    }
    *ppSecDescAbs = pSecDescAbs;

cleanup:
    LW_RTL_FREE(&pDacl);
    LW_RTL_FREE(&pSid);

    return status;

error:
    goto cleanup;
}
