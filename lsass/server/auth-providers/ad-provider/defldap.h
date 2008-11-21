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
 *        provider-main.h
 *
 * Abstract:
 *
 *        Likewise Security and Authentication Subsystem (LSASS)
 *
 *        Active Directory Authentication Provider
 *
 * Authors: Krishna Ganugapati (krishnag@likewisesoftware.com)
 *          Sriram Nambakam (snambakam@likewisesoftware.com)
 *          Wei Fu (wfu@likewisesoftware.com)
 *          Brian Dunstan (bdunstan@likewisesoftware.com)
 *          Kyle Stemen (kstemen@likewisesoftware.com)
 */

#ifndef DEFLDAP_H_
#define DEFLDAP_H_

DWORD
DefaultModeEnumUsers(
    HANDLE hProvider,
    PCSTR pszDomainDnsName,
    PCSTR pszCellDN,
    PAD_ENUM_STATE pEnumState,
    DWORD dwMaxNumUsers,
    PDWORD pdwUsersFound,
    PVOID** pppUserInfoList
    );

DWORD
DefaultModeEnumGroups(
    HANDLE hProvider,
    PCSTR pszCellDN,
    PCSTR pszDomainDnsName,
    PAD_ENUM_STATE pEnumState,
    DWORD dwMaxNumGroups,
    PDWORD pdwGroupsFound,
    PVOID** pppGroupInfoList
    );

DWORD
DefaultModeFindNSSArtefactByKey(
    HANDLE         hDirectory,
    PCSTR          pszCellDN,
    PCSTR          pszNetBIOSDomainName,
    PCSTR          pszKeyName,
    PCSTR          pszMapName,
    DWORD          dwInfoLevel,
    LSA_NIS_MAP_QUERY_FLAGS dwFlags,
    PVOID*         ppNSSArtefactInfo
    );

DWORD
DefaultModeSchemaFindNSSArtefactByKey(
    HANDLE         hDirectory,
    PCSTR          pszCellDN,
    PCSTR          pszNetBIOSDomainName,
    PCSTR          pszKeyName,
    PCSTR          pszMapName,
    DWORD          dwInfoLevel,
    LSA_NIS_MAP_QUERY_FLAGS dwFlags,
    PVOID*         ppNSSArtefactInfo
    );

DWORD
DefaultModeNonSchemaFindNSSArtefactByKey(
    HANDLE         hDirectory,
    PCSTR          pszCellDN,
    PCSTR          pszNetBIOSDomainName,
    PCSTR          pszKeyName,
    PCSTR          pszMapName,
    DWORD          dwInfoLevel,
    LSA_NIS_MAP_QUERY_FLAGS dwFlags,
    PVOID*         ppNSSArtefactInfo
    );

DWORD
DefaultModeEnumNSSArtefacts(
    HANDLE         hDirectory,
    PCSTR          pszCellDN,
    PCSTR          pszNetBIOSDomainName,
    PAD_ENUM_STATE pEnumState,
    DWORD          dwMaxNumNSSArtefacts,
    PDWORD         pdwNumNSSArtefactsFound,
    PVOID**        pppNSSArtefactInfoList
    );

DWORD
DefaultModeSchemaEnumNSSArtefacts(
    HANDLE         hDirectory,
    PCSTR          pszCellDN,
    PCSTR          pszNetBIOSDomainName,
    PAD_ENUM_STATE pEnumState,
    DWORD          dwMaxNumNSSArtefacts,
    PDWORD         pdwNumNSSArtefactsFound,
    PVOID**        pppNSSArtefactInfoList
    );

DWORD
DefaultModeNonSchemaEnumNSSArtefacts(
    HANDLE         hDirectory,
    PCSTR          pszCellDN,
    PCSTR          pszNetBIOSDomainName,
    PAD_ENUM_STATE pEnumState,
    DWORD          dwMaxNumNSSArtefacts,
    PDWORD         pdwNumNSSArtefactsFound,
    PVOID**        pppNSSArtefactInfoList
    );

#endif /*DEFLDAP_H_*/
