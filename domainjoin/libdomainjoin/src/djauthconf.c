/* Editor Settings: expandtabs and use 4 spaces for indentation
* ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
* -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright (C) Centeris Corporation 2004-2007
 * Copyright (C) Likewise Software    2007-2008
 * All rights reserved.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as 
 * published by the Free Software Foundation; either version 2.1 of 
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this program.  If not, see 
 * <http://www.gnu.org/licenses/>.
 */

#include "domainjoin.h"
#include "djpamconf.h"

static const int MAX_LINE_LENGTH = 1024;

CENTERROR
DJInitSmbConfig(PCSTR rootPrefix)
{
    CENTERROR ceError = CENTERROR_SUCCESS;
    CHAR  szFilePath[PATH_MAX+1];
    PCFGSECTION pSectionList = NULL;
    PCSTR pszSectionName = "global";
    PCFGSECTION pSection = NULL;
    
    if(rootPrefix == NULL)
        rootPrefix = "";

    sprintf(szFilePath, "%s%s/lwiauthd.conf", rootPrefix, SAMBACONFDIR);

    ceError = CTParseConfigFile(szFilePath, &pSectionList, 0);
    BAIL_ON_CENTERIS_ERROR(ceError);

    ceError = CTCreateConfigSection(&pSectionList,
                                    &pSection,
                                    pszSectionName);
    BAIL_ON_CENTERIS_ERROR(ceError);

    ceError = CTDeleteNameValuePairBySection(pSection, "realm");
    BAIL_ON_CENTERIS_ERROR(ceError);

    ceError = CTSetConfigValueBySection(pSection,
                                        "workgroup",
                                        "WORKGROUP");
    BAIL_ON_CENTERIS_ERROR(ceError);

    ceError = CTSetConfigValueBySection(pSection,
                                        "security",
                                        "user");
    BAIL_ON_CENTERIS_ERROR(ceError);

    ceError = CTSaveConfigSectionList(szFilePath,
                                      pSectionList);
    BAIL_ON_CENTERIS_ERROR(ceError);

error:

    if (pSectionList)
        CTFreeConfigSectionList(pSectionList);

    return ceError;
}

CENTERROR
SetDescription(
    PSTR pszDescription
    )
{
    return DJSetSambaValue(NULL,"server string", pszDescription);
}

static
CENTERROR
DeleteRealm()
{
    CENTERROR ceError = CENTERROR_SUCCESS;
    CHAR  szFilePath[PATH_MAX+1];
    PCFGSECTION pSectionList = NULL;
    PCSTR pszSectionName = "global";
    PCFGSECTION pSection = NULL;

    sprintf(szFilePath, "%s/lwiauthd.conf", SAMBACONFDIR);

    ceError = CTParseConfigFile(szFilePath, &pSectionList, 0);
    BAIL_ON_CENTERIS_ERROR(ceError);

    ceError = CTCreateConfigSection(&pSectionList,
                                    &pSection,
                                    pszSectionName);
    BAIL_ON_CENTERIS_ERROR(ceError);

    ceError = CTDeleteNameValuePairBySection(pSection, "realm");
    BAIL_ON_CENTERIS_ERROR(ceError);

    ceError = CTSaveConfigSectionList(szFilePath,
                                      pSectionList);
    BAIL_ON_CENTERIS_ERROR(ceError);

error:

    if (pSectionList)
        CTFreeConfigSectionList(pSectionList);

    return ceError;
}

// the name-value pair - "realm" should be set to the domain name
CENTERROR
SetRealm(PCSTR rootPrefix, PCSTR psz_realm)
{
    CENTERROR ceError = CENTERROR_SUCCESS;
    PSTR pszUpperCaseRealm = NULL;

    ceError = CTAllocateString(psz_realm, &pszUpperCaseRealm);
    BAIL_ON_CENTERIS_ERROR(ceError);

    CTStrToUpper(pszUpperCaseRealm);

    ceError = DJSetSambaValue(rootPrefix, "realm", pszUpperCaseRealm);
    BAIL_ON_CENTERIS_ERROR(ceError);

error:

    if (pszUpperCaseRealm)
        CTFreeString(pszUpperCaseRealm);

    return ceError;
}

// the name-value pair - "workgroup" should be set to the short domain name
CENTERROR
SetWorkgroup(
        const char *rootPrefix,
    char *psz_workgroup
    )
{
    CENTERROR ceError = CENTERROR_SUCCESS;

    ceError = DJSetSambaValue(rootPrefix, "workgroup", psz_workgroup);
    BAIL_ON_CENTERIS_ERROR(ceError);

error:

    return ceError;
}



CENTERROR
ConfigureSambaEx(
    PCSTR pszDomainName,
    PCSTR pszShortDomainName
    )
{
    CENTERROR ceError = CENTERROR_SUCCESS;

#if defined(_AIX)
    ceError = DJFixMethodsConfigFile();
    BAIL_ON_CENTERIS_ERROR(ceError);
#endif

    // TODO: Cleanup interface for join and leave
    //       based on user action
    if (pszDomainName == NULL &&
        pszShortDomainName == NULL) {
        /* Indicates leaving the domain */
        ceError = UnConfigureNameServiceSwitch();
        BAIL_ON_CENTERIS_ERROR(ceError);
    } else {
        ceError = ConfigureNameServiceSwitch();
        BAIL_ON_CENTERIS_ERROR(ceError);
    }

    ceError = DJModifyKrb5Conf("",
		    !IsNullOrEmptyString(pszDomainName),
		    pszDomainName, pszShortDomainName, NULL);
    BAIL_ON_CENTERIS_ERROR(ceError);

    ceError = ConfigurePamForADLogin(pszShortDomainName);
    BAIL_ON_CENTERIS_ERROR(ceError);

#if defined(_AIX)
    // Configure AIX to use PAM only after we have done the PAM re-configuration,
    // which fixes up errors in the default PAM configuration on AIX.

    if (pszDomainName == NULL &&
        pszShortDomainName == NULL) {

        ceError = UnconfigureUserSecurity(NULL);
        BAIL_ON_CENTERIS_ERROR(ceError);

    } else {

        ceError = ConfigureUserSecurity(NULL);
        BAIL_ON_CENTERIS_ERROR(ceError);

    }

    ceError = DJFixLoginConfigFile(NULL);
    BAIL_ON_CENTERIS_ERROR(ceError);
#endif

error:
    return ceError;
}

CENTERROR
DJRevertToOriginalWorkgroup(
    PSTR pszOrigWorkgroupName
    )
{
    CENTERROR ceError = CENTERROR_SUCCESS;

    ceError = DeleteRealm();
    BAIL_ON_CENTERIS_ERROR(ceError);

    ceError = SetWorkgroup(NULL, (!IsNullOrEmptyString(pszOrigWorkgroupName) ? pszOrigWorkgroupName : "WORKGROUP"));
    BAIL_ON_CENTERIS_ERROR(ceError);

    ceError = DJSetSambaValue(NULL,"security", "user");
    BAIL_ON_CENTERIS_ERROR(ceError);

    // change krb5 to not refer to a domain
    ceError = DJModifyKrb5Conf("", FALSE, "", "", NULL);
    BAIL_ON_CENTERIS_ERROR(ceError);

error:

    return ceError;
}

CENTERROR
DJSetSambaValue(
    PCSTR rootPrefix,
    PCSTR pszName,
    PCSTR pszValue
    )
{
    CENTERROR ceError = CENTERROR_SUCCESS;
    CHAR  szFilePath[PATH_MAX+1];
    PCFGSECTION pSectionList = NULL;

    if (IsNullOrEmptyString(pszName) ||
        IsNullOrEmptyString(pszValue)) {
        ceError = CENTERROR_INVALID_PARAMETER;
        BAIL_ON_CENTERIS_ERROR(ceError);
    }

    if(rootPrefix == NULL)
        rootPrefix = "";

    sprintf(szFilePath, "%s%s/lwiauthd.conf", rootPrefix, SAMBACONFDIR);

    ceError = CTParseConfigFile(szFilePath, &pSectionList, 0);
    BAIL_ON_CENTERIS_ERROR(ceError);

    ceError = CTSetConfigValueBySectionName(
        pSectionList,
        "global",
        pszName,
        pszValue);
    BAIL_ON_CENTERIS_ERROR(ceError);

    ceError = CTSaveConfigSectionList(szFilePath, pSectionList);
    BAIL_ON_CENTERIS_ERROR(ceError);

error:

    if (pSectionList)
        CTFreeConfigSectionList(pSectionList);

    return ceError;
}

CENTERROR
DJGetSambaValue(
    PSTR pszName,
    PSTR* ppszValue
    )
{
    CENTERROR ceError = CENTERROR_SUCCESS;
    CHAR  szBuf[MAX_LINE_LENGTH+1];
    PSTR pszValue = NULL;
    PCFGSECTION pSectionList = NULL;

    sprintf(szBuf, "%s/lwiauthd.conf", SAMBACONFDIR);

    ceError = CTParseConfigFile(szBuf, &pSectionList, 0);
    BAIL_ON_CENTERIS_ERROR(ceError);

    ceError = CTGetConfigValueBySectionName(
        pSectionList,
        "global",
        pszName,
        &pszValue);
    if (CENTERROR_IS_OK(ceError)) {
	*ppszValue = pszValue;
        pszValue = NULL;
    } else if (ceError != CENTERROR_CFG_VALUE_NOT_FOUND) {
        BAIL_ON_CENTERIS_ERROR(ceError);
    } else {
        ceError = CENTERROR_DOMAINJOIN_SMB_VALUE_NOT_FOUND;
        *ppszValue = NULL;
    }

    if (pSectionList)
        CTFreeConfigSectionList(pSectionList);

    if (pszValue)
        CTFreeString(pszValue);

    return ceError;

error:

    if (pszValue)
        CTFreeString(pszValue);

    if (pSectionList)
        CTFreeConfigSectionList(pSectionList);

    *ppszValue = NULL;

    return ceError;
}

