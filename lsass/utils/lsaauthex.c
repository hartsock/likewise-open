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

/*
 * Copyright (C) Likewise Software. All rights reserved.
 *
 * Module Name:
 *
 *        lsaauthex.c
 *
 * Abstract:
 *
 *        Likewise Security and Authentication Subsystem (LSASS)
 *
 * Authors: Gerald Carter <gcarter@likewisesoftware.com>
 *
 */

#include "includes.h"

DWORD
LsaFreeAuthUserInfo(
	PLSA_AUTH_USER_INFO *ppAuthUserInfo
	)
{
	PLSA_AUTH_USER_INFO p = NULL;

	if (!ppAuthUserInfo || !*ppAuthUserInfo)
	{
		return LSA_ERROR_SUCCESS;
	}

	p = *ppAuthUserInfo;

	LSA_SAFE_FREE_MEMORY(p->pszAccount);
	LSA_SAFE_FREE_MEMORY(p->pszUserPrincipalName);
	LSA_SAFE_FREE_MEMORY(p->pszFullName);
	LSA_SAFE_FREE_MEMORY(p->pszDomain);
	LSA_SAFE_FREE_MEMORY(p->pszDnsDomain);

	LsaDataBlobFree(&p->pSessionKey);
	LsaDataBlobFree(&p->pLmSessionKey);

	LSA_SAFE_FREE_MEMORY(p->pszLogonServer);
	LSA_SAFE_FREE_MEMORY(p->pszLogonScript);
	LSA_SAFE_FREE_MEMORY(p->pszProfilePath);
	LSA_SAFE_FREE_MEMORY(p->pszHomeDirectory);
	LSA_SAFE_FREE_MEMORY(p->pszHomeDrive);

	LSA_SAFE_FREE_MEMORY(p->pRidAttribList);
	LSA_SAFE_FREE_MEMORY(p->pSidAttribList);


	LSA_SAFE_FREE_MEMORY(p);

	*ppAuthUserInfo = NULL;

	return LSA_ERROR_SUCCESS;
}


DWORD
LsaFreeAuthUserParams(
	PLSA_AUTH_USER_PARAMS *ppAuthUserParams
	)
{
	PLSA_AUTH_USER_PARAMS p = NULL;

	if (!ppAuthUserParams || !*ppAuthUserParams)
	{
		return LSA_ERROR_SUCCESS;
	}

	p = *ppAuthUserParams;

	LSA_SAFE_FREE_MEMORY(p->pszAccountName);
	LSA_SAFE_FREE_MEMORY(p->pszDomain);
	LSA_SAFE_FREE_MEMORY(p->pszWorkstation);

	switch (p->AuthType)
	{
	case LSA_AUTH_PLAINTEXT:
		LSA_SAFE_FREE_MEMORY(p->pass.clear.pszPassword);
		break;
	case LSA_AUTH_CHAP:
		LsaDataBlobFree(&p->pass.chap.pChallenge);
		LsaDataBlobFree(&p->pass.chap.pNT_resp);
		LsaDataBlobFree(&p->pass.chap.pLM_resp);
		break;
	}

	LSA_SAFE_FREE_MEMORY(p);

	*ppAuthUserParams = NULL;

	return LSA_ERROR_SUCCESS;
}


