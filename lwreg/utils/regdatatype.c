/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright Likewise Software    2004-2008
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
 *        regtime.c
 *
 * Abstract:
 *
 *        Likewise Registry
 *
 *        DataType Convertion Utilities
 *
 * Authors: Wei Fu (wfu@likewise.com)
 *
 *
 */

#include "includes.h"

DWORD
RegMultiStrsToByteArrayW(
    PWSTR*   ppwszInMultiSz,
    PBYTE*   ppOutBuf,
    SSIZE_T* pOutBufLen
    )
{
    return RegNtStatusToWin32Error(
		NtRegMultiStrsToByteArrayW(ppwszInMultiSz,
				                   ppOutBuf,
				                   pOutBufLen)
				                  );
}

NTSTATUS
NtRegMultiStrsToByteArrayW(
    PWSTR*   ppwszInMultiSz,
    PBYTE*   ppOutBuf,
    SSIZE_T* pOutBufLen
    )
{
    DWORD   dwError   = 0;
    SSIZE_T idx       = 0;
    SSIZE_T OutBufLen = 0;
    PBYTE   pOutBuf   = NULL;
    PBYTE   pCursor   = NULL;

    BAIL_ON_INVALID_POINTER(ppwszInMultiSz);
    BAIL_ON_INVALID_POINTER(ppOutBuf);
    BAIL_ON_INVALID_POINTER(pOutBufLen);

    // Determine total length of all strings in bytes
    for (; ppwszInMultiSz[idx]; idx++)
    {
        size_t len = 0;

        if (ppwszInMultiSz[idx])
        {
		len = RtlWC16StringNumChars(ppwszInMultiSz[idx]);
        }

        OutBufLen +=  (len + 1) * sizeof(WCHAR);
    }

    OutBufLen += sizeof(WCHAR); // double null at end

    dwError = LW_RTL_ALLOCATE((PVOID*)&pOutBuf, BYTE,
		                  sizeof(*pOutBuf) * OutBufLen);
    BAIL_ON_REG_ERROR(dwError);

    for (idx=0, pCursor = pOutBuf; ppwszInMultiSz[idx]; idx++)
    {
        size_t len = 0;

        if (ppwszInMultiSz[idx])
        {
		len = RtlWC16StringNumChars(ppwszInMultiSz[idx]);
        }

        len++; // accommodate null

        memcpy(pCursor, (PBYTE)ppwszInMultiSz[idx], len * sizeof(WCHAR));

        pCursor += len * sizeof(WCHAR);
    }

    *((PWSTR)(pCursor)) = 0;

   *ppOutBuf   = pOutBuf;
   *pOutBufLen = OutBufLen;

cleanup:

    return dwError;

error:

    if (pOutBuf)
    {
        LwRtlMemoryFree(pOutBuf);
    }

    if (ppOutBuf)
    {
        *ppOutBuf = NULL;
    }
    if (pOutBufLen)
    {
        *pOutBufLen = 0;
    }

    goto cleanup;
}

DWORD
RegMultiStrsToByteArrayA(
    PSTR*    ppszInMultiSz,
    PBYTE*   ppOutBuf,
    SSIZE_T* pOutBufLen
    )
{
    return RegNtStatusToWin32Error(
		NtRegMultiStrsToByteArrayA(ppszInMultiSz,
				                   ppOutBuf,
				                   pOutBufLen)
				                  );
}

NTSTATUS
NtRegMultiStrsToByteArrayA(
    PSTR*    ppszInMultiSz,
    PBYTE*   ppOutBuf,
    SSIZE_T* pOutBufLen
    )
{
    DWORD   dwError   = 0;
    SSIZE_T idx       = 0;
    SSIZE_T OutBufLen = 0;
    PBYTE   pOutBuf   = NULL;
    PBYTE   pCursor   = NULL;

    BAIL_ON_INVALID_POINTER(ppszInMultiSz);
    BAIL_ON_INVALID_POINTER(ppOutBuf);
    BAIL_ON_INVALID_POINTER(pOutBufLen);

    // Determine total length of all strings in bytes
    for (; ppszInMultiSz[idx]; idx++)
    {
        OutBufLen += strlen(ppszInMultiSz[idx]) + 1;
    }

    OutBufLen++; // double null at end

    dwError = LW_RTL_ALLOCATE((PVOID*)&pOutBuf, BYTE,
		                  sizeof(*pOutBuf) * OutBufLen);
    BAIL_ON_REG_ERROR(dwError);

    for (idx=0, pCursor = pOutBuf; ppszInMultiSz[idx]; idx++)
    {
        size_t len = strlen(ppszInMultiSz[idx]) + 1;

        memcpy(pCursor, ppszInMultiSz[idx], len);

        pCursor += len;
    }

    *pCursor = '\0';

   *ppOutBuf   = pOutBuf;
   *pOutBufLen = OutBufLen;

cleanup:

    return dwError;

error:

    if (pOutBuf)
    {
        LwRtlMemoryFree(pOutBuf);
    }

    if (ppOutBuf)
    {
        *ppOutBuf = NULL;
    }
    if (pOutBufLen)
    {
        *pOutBufLen = 0;
    }

    goto cleanup;
}

DWORD
RegByteArrayToMultiStrsW(
    PBYTE   pInBuf,
    SSIZE_T bufLen,
    PWSTR** pppwszStrings
    )
{
    return RegNtStatusToWin32Error(
		NtRegByteArrayToMultiStrsW(pInBuf,
				                   bufLen,
				                   pppwszStrings)
				                  );

}

NTSTATUS
NtRegByteArrayToMultiStrsW(
    PBYTE   pInBuf,
    SSIZE_T bufLen,
    PWSTR** pppwszStrings
    )
{
    DWORD   dwError      = 0;
    DWORD   dwNumStrings = 0;
    PWSTR*  ppwszStrings = NULL;
    PWSTR   pwszCursor   = NULL;
    size_t  len          = 0;
    SSIZE_T iStr         = 0;

    BAIL_ON_INVALID_POINTER(pInBuf);
    BAIL_ON_INVALID_POINTER(pppwszStrings);

    if (!bufLen || (bufLen % sizeof(WCHAR)))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_REG_ERROR(dwError);
    }

    // determine number of strings
    pwszCursor = (PWSTR)pInBuf;
	do
    {
        if (pwszCursor)
        {
		len = RtlWC16StringNumChars(pwszCursor);
        }

        if (len)
        {
            pwszCursor += len + 1;
            dwNumStrings++;
        }
    } while (len);

    dwError = LW_RTL_ALLOCATE((PVOID*)&ppwszStrings, PWSTR,
		                  sizeof(*ppwszStrings) * (dwNumStrings + 1));
    BAIL_ON_REG_ERROR(dwError);

    pwszCursor = (PWSTR)pInBuf;
    for (iStr = 0; iStr < dwNumStrings; iStr++)
    {
        PWSTR   pwszStrBegin = pwszCursor;

	    len = 0;
        while (!IsNullOrEmptyString(pwszCursor))
        {
            len++;
            pwszCursor++;
        }

        dwError = LW_RTL_ALLOCATE((PVOID*)&ppwszStrings[iStr], WCHAR,
			                  sizeof(*ppwszStrings[iStr]) * (len + 1));
        BAIL_ON_REG_ERROR(dwError);

        memcpy( (PBYTE)ppwszStrings[iStr],
                (PBYTE)pwszStrBegin,
                len * sizeof(WCHAR));

        pwszCursor++;
    }

    *pppwszStrings = ppwszStrings;

cleanup:

    return dwError;

error:

    *pppwszStrings = NULL;

    if (ppwszStrings)
    {
        RegFreeMultiStrsW(ppwszStrings);
    }

    goto cleanup;
}

DWORD
RegByteArrayToMultiStrsA(
    PBYTE   pInBuf,
    SSIZE_T bufLen,
    PSTR**  pppszStrings
    )
{
    return RegNtStatusToWin32Error(
		NtRegByteArrayToMultiStrsA(pInBuf,
				                   bufLen,
				                   pppszStrings)
				                  );
}

NTSTATUS
NtRegByteArrayToMultiStrsA(
    PBYTE   pInBuf,
    SSIZE_T bufLen,
    PSTR**  pppszStrings
    )
{
    DWORD   dwError      = 0;
    DWORD   dwNumStrings = 0;
    PSTR*   ppszStrings  = NULL;
    PSTR    pszCursor    = NULL;
    SSIZE_T sLen         = 0;
    SSIZE_T iStr         = 0;

    BAIL_ON_INVALID_POINTER(pInBuf);
    BAIL_ON_INVALID_POINTER(pppszStrings);

    if (!bufLen)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_REG_ERROR(dwError);
    }

    // determine number of strings
    pszCursor = (PSTR)pInBuf;
    do
    {
        sLen = strlen(pszCursor);

        if (sLen)
        {
            pszCursor += sLen + 1;
            dwNumStrings++;
        }
    } while (sLen);

    dwError = LW_RTL_ALLOCATE((PVOID*)&ppszStrings, PSTR,
		                  sizeof(*ppszStrings) * (dwNumStrings + 1));
    BAIL_ON_REG_ERROR(dwError);

    pszCursor = (PSTR)pInBuf;
    for (iStr = 0; iStr < dwNumStrings; iStr++)
    {
        SSIZE_T len = 0;
        PSTR    pszStrBegin = pszCursor;

        while (!IsNullOrEmptyString(pszCursor))
        {
            len++;
            pszCursor++;
        }

        dwError = LW_RTL_ALLOCATE((PVOID*)&ppszStrings[iStr], CHAR,
			                  sizeof(*ppszStrings[iStr]) * (len + 1));
        BAIL_ON_REG_ERROR(dwError);

        memcpy( (PBYTE)ppszStrings[iStr],
                (PBYTE)pszStrBegin,
                len * sizeof(CHAR));

        pszCursor++;
    }

    *pppszStrings = ppszStrings;

cleanup:

    return dwError;

error:

    *pppszStrings = NULL;

    if (ppszStrings)
    {
        RegFreeMultiStrsA(ppszStrings);
    }

    goto cleanup;
}

DWORD
RegConvertByteStreamA2W(
    const PBYTE pData,
    DWORD       cbData,
    PBYTE*      ppOutData,
    PDWORD      pcbOutDataLen
    )
{
    return RegNtStatusToWin32Error(
		NtRegConvertByteStreamA2W(pData,
				                  cbData,
				                  ppOutData,
				                  pcbOutDataLen)
				                  );
}

NTSTATUS
NtRegConvertByteStreamA2W(
    const PBYTE pData,
    DWORD       cbData,
    PBYTE*      ppOutData,
    PDWORD      pcbOutDataLen
    )
{
    DWORD dwError      = 0;
    PBYTE pOutData     = NULL;
    DWORD cbOutDataLen = 0;
    PCSTR pszCursor    = NULL;
    PWSTR pwszCursor   = NULL;
    PWSTR pwszValue    = NULL;
    DWORD cbWcLen      = 0;

    cbOutDataLen = cbData * sizeof(WCHAR);

    dwError = LW_RTL_ALLOCATE((PVOID*)&pOutData, BYTE,
		                  sizeof(*pOutData) * cbOutDataLen);
    BAIL_ON_REG_ERROR(dwError);

    pszCursor = (PCSTR)pData;
    pwszCursor = (PWSTR)pOutData;

    while (pszCursor && *pszCursor)
    {
        DWORD dwLength = strlen(pszCursor);

        if (pwszValue)
        {
            LwRtlMemoryFree(pwszValue);
            pwszValue = NULL;
        }

	dwError = LwRtlWC16StringAllocateFromCString(&pwszValue, pszCursor);
	BAIL_ON_REG_ERROR(dwError);

        cbWcLen = wc16slen(pwszValue);
        memcpy((PBYTE)pwszCursor,
               (PBYTE)pwszValue,
               (cbWcLen + 1) * sizeof(*pwszValue));

        pszCursor  += dwLength + 1;
        pwszCursor += cbWcLen + 1;
    }
    *pwszCursor++ = '\0';

    *ppOutData     = pOutData;
    *pcbOutDataLen = ((PBYTE) pwszCursor) - pOutData;

cleanup:

    if (pwszValue)
    {
        LwRtlMemoryFree(pwszValue);
    }

    return dwError;

error:

    *ppOutData     = NULL;
    *pcbOutDataLen = 0;

    if (pOutData)
    {
        LwRtlMemoryFree(pOutData);
    }

    goto cleanup;
}

DWORD
RegConvertByteStreamW2A(
    const PBYTE pData,
    DWORD       cbData,
    PBYTE*      ppOutData,
    PDWORD      pcbOutDataLen
    )
{
    return RegNtStatusToWin32Error(
		NtRegConvertByteStreamW2A(pData,
				                  cbData,
				                  ppOutData,
				                  pcbOutDataLen)
				                  );
}

NTSTATUS
NtRegConvertByteStreamW2A(
    const PBYTE pData,
    DWORD       cbData,
    PBYTE*      ppOutData,
    PDWORD      pcbOutDataLen
    )
{
    DWORD dwError      = 0;
    PBYTE pOutData     = NULL;
    DWORD cbOutDataLen = 0;
    PSTR  pszCursor    = NULL;
    PWSTR pwszCursor   = NULL;
    PSTR  pszValue     = NULL;
    DWORD cbMbsLen     = 0;

    /* High bound on bytes for given number of wide characters passed in */
    cbOutDataLen = (cbData / sizeof(WCHAR)) * 4;

    dwError = LW_RTL_ALLOCATE((PVOID*)&pOutData, BYTE,
		                  sizeof(*pOutData) * cbOutDataLen);
    BAIL_ON_REG_ERROR(dwError);

    pwszCursor = (PWSTR)pData;
    pszCursor = (PSTR)pOutData;

    while (pwszCursor && *pwszCursor)
    {
        size_t len = 0;

        if (pwszCursor)
        {
		len = RtlWC16StringNumChars(pwszCursor);
        }

        if (pszValue)
        {
            LwRtlMemoryFree(pszValue);
            pszValue = NULL;
        }

	dwError = LwRtlCStringAllocateFromWC16String(&pszValue, pwszCursor);
        BAIL_ON_REG_ERROR(dwError);

        cbMbsLen = strlen(pszValue);
        memcpy(pszCursor, pszValue, cbMbsLen + 1);

        pszCursor  += cbMbsLen + 1;
        pwszCursor += len + 1;
    }
    *pszCursor++ = '\0';

    *ppOutData     = pOutData;
    *pcbOutDataLen = ((PBYTE) pszCursor) - pOutData;

cleanup:

    if (pszValue)
    {
        LwRtlMemoryFree(pszValue);
    }

    return dwError;

error:

    *ppOutData     = NULL;
    *pcbOutDataLen = 0;

    if (pOutData)
    {
        LwRtlMemoryFree(pOutData);
    }

    goto cleanup;
}

NTSTATUS
RegCopyValueBytes(
    IN PBYTE pValue,
    IN DWORD dwValueLen,
    OUT OPTIONAL PBYTE pData,
    IN OUT OPTIONAL PDWORD pcbData
    )
{
    NTSTATUS status = 0;

    if (pData && !pcbData)
    {
	status = STATUS_INVALID_PARAMETER;
	BAIL_ON_NT_STATUS(status);
    }

    if(pData && dwValueLen > *pcbData)
    {
        status = STATUS_BUFFER_TOO_SMALL;
        BAIL_ON_NT_STATUS(status);
    }

    if (dwValueLen && pData)
    {
        memcpy(pData, pValue, dwValueLen);
    }

    if (pcbData)
    {
        *pcbData = dwValueLen;
    }

error:
    return status;

    /*switch (type)
    {
        case REG_BINARY:
        case REG_MULTI_SZ:
        case REG_SZ:
            status = RegHexStrToByteArray(
                           pszValue,
                           NULL,
                           &pTempData,
                           &cbData);
            BAIL_ON_NT_STATUS(status);

            if(pData && cbData > *pcbData)
            {
                status = STATUS_BUFFER_TOO_SMALL;
                BAIL_ON_NT_STATUS(status);
            }

            if (pData)
            {
                memcpy(pData, pTempData, cbData);
            }

            break;

        case REG_DWORD:

            if (pData &&  sizeof(dwValue) > *pcbData)
            {
                status = STATUS_BUFFER_TOO_SMALL;
                BAIL_ON_NT_STATUS(status);
            }

            dwValue = (DWORD)atoi(pszValue);

            cbData = sizeof(dwValue);

            if (pData)
            {
                memcpy(pData, &dwValue, cbData);
            }

            break;

        default:
		status = STATUS_NOT_SUPPORTED;
            BAIL_ON_NT_STATUS(status);
    }*/
}

void
RegFreeMultiStrsA(
    PSTR* ppszStrings
    )
{
    SSIZE_T idx = 0;

    if (!ppszStrings)
    {
        return;
    }
    while (ppszStrings[idx])
    {
        LwRtlMemoryFree(ppszStrings[idx++]);
    }

    LwRtlMemoryFree(ppszStrings);
}

void
RegFreeMultiStrsW(
    PWSTR* ppwszStrings
    )
{
    SSIZE_T idx = 0;

    while (ppwszStrings[idx])
    {
        LwRtlMemoryFree(ppwszStrings[idx++]);
    }

    LwRtlMemoryFree(ppwszStrings);
}
