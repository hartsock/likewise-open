/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

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

#include "client.h"

OM_uint32
ntlm_gss_acquire_supplied_cred(
    OM_uint32 *minorStatus,
    gss_name_t desiredName,
    gss_buffer_t credBuffer,
    OM_uint32 reqTime,
    gss_OID_set desiredMechs,
    gss_cred_usage_t credUsage,
    gss_cred_id_t *credHandle,
    gss_OID_set *actualMechs,
    OM_uint32 *retTime
)
{

    DWORD dwError;
    DWORD majorStatus = GSS_S_COMPLETE;
    SEC_BUFFER suppliedCreds;
    uid_t uid = geteuid();

    if (credBuffer)
        MAKE_SECBUFFER(&suppliedCreds, credBuffer);
    else
        ZERO_STRUCT(suppliedCreds);

    /* @todo - OID support */

    dwError = NTLMGssAcquireSuppliedCred(
                    minorStatus,
                    &suppliedCreds,
                    uid,
                    (DWORD) reqTime,
                    (DWORD) credUsage,
                    (PVOID*) credHandle,
                    (DWORD*) retTime
                    );

    BAIL_ON_LSA_ERROR(dwError);

error:

    if (dwError)
        majorStatus = NTLMTranslateMajorStatus(dwError);

    return majorStatus;
}

