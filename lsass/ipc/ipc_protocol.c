#include "ipc.h"

static LWMsgTypeSpec gLsaIPCErrorSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_IPC_ERROR),
    LWMSG_MEMBER_UINT32(LSA_IPC_ERROR, dwError),
    LWMSG_MEMBER_PSTR(LSA_IPC_ERROR, pszErrorMessage),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaGroupInfo0Spec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_GROUP_INFO_0),
    LWMSG_MEMBER_UINT32(LSA_GROUP_INFO_0, gid),
    LWMSG_MEMBER_PSTR(LSA_GROUP_INFO_0, pszName),
    LWMSG_MEMBER_PSTR(LSA_GROUP_INFO_0, pszSid),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaGroupInfo1Spec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_GROUP_INFO_1),
    LWMSG_MEMBER_UINT32(LSA_GROUP_INFO_1, gid),
    LWMSG_MEMBER_PSTR(LSA_GROUP_INFO_1, pszName),
    LWMSG_MEMBER_PSTR(LSA_GROUP_INFO_1, pszSid),
    LWMSG_MEMBER_PSTR(LSA_GROUP_INFO_1, pszPasswd),
    LWMSG_MEMBER_POINTER_BEGIN(LSA_GROUP_INFO_1, ppszMembers),
    LWMSG_PSTR,
    LWMSG_POINTER_END,
    LWMSG_ATTR_STRING,
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

#define GROUP_INFO_LEVEL_0 0
#define GROUP_INFO_LEVEL_1 1

static LWMsgTypeSpec gLsaGroupInfoListSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_GROUP_INFO_LIST),
    LWMSG_MEMBER_UINT32(LSA_GROUP_INFO_LIST, dwGroupInfoLevel),
    LWMSG_MEMBER_UINT32(LSA_GROUP_INFO_LIST, dwNumGroups),
    LWMSG_MEMBER_UNION_BEGIN(LSA_GROUP_INFO_LIST, ppGroupInfoList),
    LWMSG_MEMBER_POINTER_BEGIN(union _GROUP_INFO_LIST, ppInfoList0),
    LWMSG_POINTER_BEGIN,
    LWMSG_TYPESPEC(gLsaGroupInfo0Spec),
    LWMSG_POINTER_END,
    LWMSG_POINTER_END,
    LWMSG_ATTR_LENGTH_MEMBER(LSA_GROUP_INFO_LIST, dwNumGroups),
    LWMSG_ATTR_TAG(GROUP_INFO_LEVEL_0),
    LWMSG_MEMBER_POINTER_BEGIN(union _GROUP_INFO_LIST, ppInfoList1),
    LWMSG_POINTER_BEGIN,
    LWMSG_TYPESPEC(gLsaGroupInfo1Spec),
    LWMSG_POINTER_END,
    LWMSG_POINTER_END,
    LWMSG_ATTR_LENGTH_MEMBER(LSA_GROUP_INFO_LIST, dwNumGroups),
    LWMSG_ATTR_TAG(GROUP_INFO_LEVEL_1),
    LWMSG_UNION_END,
    LWMSG_ATTR_DISCRIM(LSA_GROUP_INFO_LIST, dwGroupInfoLevel),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaUserInfo0Spec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_USER_INFO_0),
    LWMSG_MEMBER_UINT32(LSA_USER_INFO_0, uid),
    LWMSG_MEMBER_UINT32(LSA_USER_INFO_0, gid),
    LWMSG_MEMBER_PSTR(LSA_USER_INFO_0, pszName),
    LWMSG_MEMBER_PSTR(LSA_USER_INFO_0, pszPasswd),
    LWMSG_MEMBER_PSTR(LSA_USER_INFO_0, pszGecos),
    LWMSG_MEMBER_PSTR(LSA_USER_INFO_0, pszShell),
    LWMSG_MEMBER_PSTR(LSA_USER_INFO_0, pszHomedir),
    LWMSG_MEMBER_PSTR(LSA_USER_INFO_0, pszSid),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaUserInfo1Spec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_USER_INFO_1),
    LWMSG_MEMBER_UINT32(LSA_USER_INFO_1, uid),
    LWMSG_MEMBER_UINT32(LSA_USER_INFO_1, gid),
    LWMSG_MEMBER_PSTR(LSA_USER_INFO_1, pszName),
    LWMSG_MEMBER_PSTR(LSA_USER_INFO_1, pszPasswd),
    LWMSG_MEMBER_PSTR(LSA_USER_INFO_1, pszGecos),
    LWMSG_MEMBER_PSTR(LSA_USER_INFO_1, pszShell),
    LWMSG_MEMBER_PSTR(LSA_USER_INFO_1, pszHomedir),
    LWMSG_MEMBER_PSTR(LSA_USER_INFO_1, pszSid),
    LWMSG_MEMBER_PSTR(LSA_USER_INFO_1, pszUPN),
    LWMSG_MEMBER_UINT32(LSA_USER_INFO_1, bIsGeneratedUPN),
    LWMSG_MEMBER_UINT32(LSA_USER_INFO_1, bIsLocalUser),
    LWMSG_MEMBER_UINT32(LSA_USER_INFO_1, dwLMHashLen),
    LWMSG_MEMBER_POINTER_BEGIN(LSA_USER_INFO_1, pLMHash),
    LWMSG_UINT8(BYTE),
    LWMSG_POINTER_END,
    LWMSG_ATTR_LENGTH_MEMBER(LSA_USER_INFO_1, dwLMHashLen),
    LWMSG_MEMBER_UINT32(LSA_USER_INFO_1, dwNTHashLen),
    LWMSG_MEMBER_POINTER_BEGIN(LSA_USER_INFO_1, pNTHash),
    LWMSG_UINT8(BYTE),
    LWMSG_POINTER_END,
    LWMSG_ATTR_LENGTH_MEMBER(LSA_USER_INFO_1, dwNTHashLen),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaUserInfo2Spec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_USER_INFO_2),
    LWMSG_MEMBER_UINT32(LSA_USER_INFO_2, uid),
    LWMSG_MEMBER_UINT32(LSA_USER_INFO_2, gid),
    LWMSG_MEMBER_PSTR(LSA_USER_INFO_2, pszName),
    LWMSG_MEMBER_PSTR(LSA_USER_INFO_2, pszPasswd),
    LWMSG_MEMBER_PSTR(LSA_USER_INFO_2, pszGecos),
    LWMSG_MEMBER_PSTR(LSA_USER_INFO_2, pszShell),
    LWMSG_MEMBER_PSTR(LSA_USER_INFO_2, pszHomedir),
    LWMSG_MEMBER_PSTR(LSA_USER_INFO_2, pszSid),
    LWMSG_MEMBER_PSTR(LSA_USER_INFO_2, pszUPN),
    LWMSG_MEMBER_UINT32(LSA_USER_INFO_2, bIsGeneratedUPN),
    LWMSG_MEMBER_UINT32(LSA_USER_INFO_2, bIsLocalUser),
    LWMSG_MEMBER_UINT32(LSA_USER_INFO_2, dwLMHashLen),
    LWMSG_MEMBER_POINTER_BEGIN(LSA_USER_INFO_2, pLMHash),
    LWMSG_UINT8(BYTE),
    LWMSG_POINTER_END,
    LWMSG_ATTR_LENGTH_MEMBER(LSA_USER_INFO_2, dwLMHashLen),
    LWMSG_MEMBER_UINT32(LSA_USER_INFO_2, dwNTHashLen),
    LWMSG_MEMBER_POINTER_BEGIN(LSA_USER_INFO_2, pNTHash),
    LWMSG_UINT8(BYTE),
    LWMSG_POINTER_END,
    LWMSG_ATTR_LENGTH_MEMBER(LSA_USER_INFO_2, dwNTHashLen),
    LWMSG_MEMBER_UINT32(LSA_USER_INFO_2, dwDaysToPasswordExpiry),
    LWMSG_MEMBER_INT8(LSA_USER_INFO_2, bPasswordExpired),
    LWMSG_MEMBER_INT8(LSA_USER_INFO_2, bPasswordNeverExpires),
    LWMSG_MEMBER_INT8(LSA_USER_INFO_2, bPromptPasswordChange),
    LWMSG_MEMBER_INT8(LSA_USER_INFO_2, bUserCanChangePassword),
    LWMSG_MEMBER_INT8(LSA_USER_INFO_2, bAccountDisabled),
    LWMSG_MEMBER_INT8(LSA_USER_INFO_2, bAccountExpired),
    LWMSG_MEMBER_INT8(LSA_USER_INFO_2, bAccountLocked),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

#define USER_INFO_LEVEL_0 0
#define USER_INFO_LEVEL_1 1
#define USER_INFO_LEVEL_2 2

static LWMsgTypeSpec gLsaUserInfoListSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_USER_INFO_LIST),
    LWMSG_MEMBER_UINT32(LSA_USER_INFO_LIST, dwUserInfoLevel),
    LWMSG_MEMBER_UINT32(LSA_USER_INFO_LIST, dwNumUsers),
    LWMSG_MEMBER_UNION_BEGIN(LSA_USER_INFO_LIST, ppUserInfoList),
    LWMSG_MEMBER_POINTER_BEGIN(union _USER_INFO_LIST, ppInfoList0),
    LWMSG_POINTER_BEGIN,
    LWMSG_TYPESPEC(gLsaUserInfo0Spec),
    LWMSG_POINTER_END,
    LWMSG_POINTER_END,
    LWMSG_ATTR_LENGTH_MEMBER(LSA_USER_INFO_LIST, dwNumUsers),
    LWMSG_ATTR_TAG(USER_INFO_LEVEL_0),
    LWMSG_MEMBER_POINTER_BEGIN(union _USER_INFO_LIST, ppInfoList1),
    LWMSG_POINTER_BEGIN,
    LWMSG_TYPESPEC(gLsaUserInfo1Spec),
    LWMSG_POINTER_END,
    LWMSG_POINTER_END,
    LWMSG_ATTR_LENGTH_MEMBER(LSA_USER_INFO_LIST, dwNumUsers),
    LWMSG_ATTR_TAG(GROUP_INFO_LEVEL_1),
    LWMSG_MEMBER_POINTER_BEGIN(union _USER_INFO_LIST, ppInfoList2),
    LWMSG_POINTER_BEGIN,
    LWMSG_TYPESPEC(gLsaUserInfo2Spec),
    LWMSG_POINTER_END,
    LWMSG_POINTER_END,
    LWMSG_ATTR_LENGTH_MEMBER(LSA_USER_INFO_LIST, dwNumUsers),
    LWMSG_ATTR_TAG(USER_INFO_LEVEL_2),
    LWMSG_UNION_END,
    LWMSG_ATTR_DISCRIM(LSA_USER_INFO_LIST, dwUserInfoLevel),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaBeginObjectEnumSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_ENUM_OBJECTS_INFO),
    LWMSG_MEMBER_UINT32(LSA_ENUM_OBJECTS_INFO, dwObjectInfoLevel),
    LWMSG_MEMBER_UINT32(LSA_ENUM_OBJECTS_INFO, dwNumMaxObjects),
    LWMSG_MEMBER_PSTR(LSA_ENUM_OBJECTS_INFO, pszGUID),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaNssArtefactInfo0Spec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_NSS_ARTEFACT_INFO_0),
    LWMSG_MEMBER_PSTR(LSA_NSS_ARTEFACT_INFO_0, pszName),
    LWMSG_MEMBER_PSTR(LSA_NSS_ARTEFACT_INFO_0, pszValue),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

#define NSS_ARTEFACT_INFO_LEVEL_0 0

static LWMsgTypeSpec gLsaNssArtefactInfoListSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_NSS_ARTEFACT_INFO_LIST),
    LWMSG_MEMBER_UINT32(LSA_NSS_ARTEFACT_INFO_LIST, dwNssArtefactInfoLevel),
    LWMSG_MEMBER_UINT32(LSA_NSS_ARTEFACT_INFO_LIST, dwNumNssArtefacts),
    LWMSG_MEMBER_UNION_BEGIN(LSA_NSS_ARTEFACT_INFO_LIST, ppNssArtefactInfoList),
    LWMSG_MEMBER_POINTER_BEGIN(union _NSS_ARTEFACT_INFO_LIST, ppInfoList0),
    LWMSG_POINTER_BEGIN,
    LWMSG_TYPESPEC(gLsaNssArtefactInfo0Spec),
    LWMSG_POINTER_END,
    LWMSG_POINTER_END,
    LWMSG_ATTR_LENGTH_MEMBER(LSA_NSS_ARTEFACT_INFO_LIST, dwNumNssArtefacts),
    LWMSG_ATTR_TAG(NSS_ARTEFACT_INFO_LEVEL_0),
    LWMSG_UNION_END,
    LWMSG_ATTR_DISCRIM(LSA_NSS_ARTEFACT_INFO_LIST, dwNssArtefactInfoLevel),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCUserModInfoSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_USER_MOD_INFO),
    LWMSG_MEMBER_UINT32(LSA_USER_MOD_INFO, uid),
    LWMSG_MEMBER_STRUCT_BEGIN(LSA_USER_MOD_INFO, actions),
    LWMSG_MEMBER_INT8(struct _actions, bEnableUser),
    LWMSG_MEMBER_INT8(struct _actions, bDisableUser),
    LWMSG_MEMBER_INT8(struct _actions, bUnlockUser),
    LWMSG_MEMBER_INT8(struct _actions, bSetChangePasswordOnNextLogon),
    LWMSG_MEMBER_INT8(struct _actions, bSetPasswordNeverExpires),
    LWMSG_MEMBER_INT8(struct _actions, bSetPasswordMustExpire),
    LWMSG_MEMBER_INT8(struct _actions, bAddToGroups),
    LWMSG_MEMBER_INT8(struct _actions, bRemoveFromGroups),
    LWMSG_MEMBER_INT8(struct _actions, bSetAccountExpiryDate),
    LWMSG_STRUCT_END,
    LWMSG_MEMBER_PSTR(LSA_USER_MOD_INFO, pszAddToGroups),
    LWMSG_MEMBER_PSTR(LSA_USER_MOD_INFO, pszRemoveFromGroups),
    LWMSG_MEMBER_PSTR(LSA_USER_MOD_INFO, pszExpiryDate),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaSidInfoSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_SID_INFO),
    LWMSG_MEMBER_UINT8(LSA_SID_INFO, accountType),
    LWMSG_MEMBER_PSTR(LSA_SID_INFO, pszSamAccountName),
    LWMSG_MEMBER_PSTR(LSA_SID_INFO, pszDomainName),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaNamesBySidsSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_FIND_NAMES_BY_SIDS),
    LWMSG_MEMBER_UINT32(LSA_FIND_NAMES_BY_SIDS, sCount),
    LWMSG_MEMBER_POINTER_BEGIN(LSA_FIND_NAMES_BY_SIDS, pSIDInfoList),
    LWMSG_TYPESPEC(gLsaSidInfoSpec),
    LWMSG_POINTER_END,
    LWMSG_ATTR_LENGTH_MEMBER(LSA_FIND_NAMES_BY_SIDS, sCount),
    LWMSG_MEMBER_INT8(LSA_FIND_NAMES_BY_SIDS, chDomainSeparator),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaSecBufferSpec[] =
{
    LWMSG_STRUCT_BEGIN(SEC_BUFFER),
    LWMSG_MEMBER_UINT8(SEC_BUFFER, length),
    LWMSG_MEMBER_UINT8(SEC_BUFFER, maxLength),
    LWMSG_MEMBER_POINTER_BEGIN(SEC_BUFFER, buffer),
    LWMSG_UINT8(BYTE),
    LWMSG_POINTER_END,
    LWMSG_ATTR_LENGTH_MEMBER(SEC_BUFFER, maxLength),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaSecBufferSSpec[] =
{
    LWMSG_STRUCT_BEGIN(SEC_BUFFER_S),
    LWMSG_MEMBER_UINT8(SEC_BUFFER_S, length),
    LWMSG_MEMBER_UINT8(SEC_BUFFER_S, maxLength),
    LWMSG_MEMBER_ARRAY_BEGIN(SEC_BUFFER_S, buffer),
    LWMSG_UINT8(BYTE),
    LWMSG_ARRAY_END,
    LWMSG_ATTR_LENGTH_STATIC(S_BUFLEN),
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCMakeAuthMsgReplySpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_GSS_R_MAKE_AUTH_MSG),
    LWMSG_MEMBER_UINT32(LSA_GSS_R_MAKE_AUTH_MSG, msgError),
    LWMSG_MEMBER_TYPESPEC(LSA_GSS_R_MAKE_AUTH_MSG, authenticateMessage, gLsaSecBufferSpec),
    LWMSG_MEMBER_TYPESPEC(LSA_GSS_R_MAKE_AUTH_MSG, baseSessionKey, gLsaSecBufferSSpec),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCCheckAuthMsgReplySpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_GSS_R_CHECK_AUTH_MSG),
    LWMSG_MEMBER_UINT32(LSA_GSS_R_CHECK_AUTH_MSG, msgError),
    LWMSG_MEMBER_TYPESPEC(LSA_GSS_R_CHECK_AUTH_MSG, baseSessionKey, gLsaSecBufferSSpec),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaMetricPack0Spec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_METRIC_PACK_0),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_0, failedAuthentications),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_0, failedUserLookupsByName),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_0, failedUserLookupsById),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_0, failedGroupLookupsByName),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_0, failedGroupLookupsById),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_0, failedOpenSession),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_0, failedCloseSession),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_0, failedChangePassword),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_0, unauthorizedAccesses),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaMetricPack1Spec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_METRIC_PACK_1),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_1, successfulAuthentications),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_1, failedAuthentications),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_1, rootUserAuthentications),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_1, successfulUserLookupsByName),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_1, failedUserLookupsByName),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_1, successfulUserLookupsById),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_1, failedUserLookupsById),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_1, successfulGroupLookupsByName),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_1, failedGroupLookupsByName),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_1, successfulGroupLookupsById),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_1, failedGroupLookupsById),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_1, successfulOpenSession),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_1, failedOpenSession),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_1, successfulCloseSession),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_1, failedCloseSession),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_1, successfulChangePassword),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_1, failedChangePassword),
    LWMSG_MEMBER_UINT64(LSA_METRIC_PACK_1, unauthorizedAccesses),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

#define METRIC_INFO_LEVEL_0 0
#define METRIC_INFO_LEVEL_1 1

static LWMsgTypeSpec gLsaMetricPackSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_METRIC_PACK),
    LWMSG_MEMBER_UINT32(LSA_METRIC_PACK, dwInfoLevel),
    LWMSG_MEMBER_UNION_BEGIN(LSA_METRIC_PACK, pMetricPack),
    LWMSG_MEMBER_POINTER_BEGIN(union _METRIC_PACK, pMetricPack0),
    LWMSG_TYPESPEC(gLsaMetricPack0Spec),
    LWMSG_POINTER_END,
    LWMSG_ATTR_TAG(METRIC_INFO_LEVEL_0),
    LWMSG_MEMBER_POINTER_BEGIN(union _METRIC_PACK, pMetricPack1),
    LWMSG_TYPESPEC(gLsaMetricPack1Spec),
    LWMSG_POINTER_END,
    LWMSG_ATTR_TAG(METRIC_INFO_LEVEL_1),
    LWMSG_UNION_END,
    LWMSG_ATTR_DISCRIM(LSA_METRIC_PACK, dwInfoLevel),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaDcInfoSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_DC_INFO),
    LWMSG_MEMBER_PSTR(LSA_DC_INFO, pszName),
    LWMSG_MEMBER_PSTR(LSA_DC_INFO, pszAddress),
    LWMSG_MEMBER_PSTR(LSA_DC_INFO, pszSiteName),
    LWMSG_MEMBER_UINT32(LSA_DC_INFO, dwFlags),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaTrustedDomainInfoSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_TRUSTED_DOMAIN_INFO),
    LWMSG_MEMBER_PSTR(LSA_TRUSTED_DOMAIN_INFO, pszDnsDomain),
    LWMSG_MEMBER_PSTR(LSA_TRUSTED_DOMAIN_INFO, pszNetbiosDomain),
    LWMSG_MEMBER_PSTR(LSA_TRUSTED_DOMAIN_INFO, pszTrusteeDnsDomain),
    LWMSG_MEMBER_PSTR(LSA_TRUSTED_DOMAIN_INFO, pszDomainSID),
    LWMSG_MEMBER_PSTR(LSA_TRUSTED_DOMAIN_INFO, pszDomainGUID),
    LWMSG_MEMBER_PSTR(LSA_TRUSTED_DOMAIN_INFO, pszForestName),
    LWMSG_MEMBER_PSTR(LSA_TRUSTED_DOMAIN_INFO, pszClientSiteName),
    LWMSG_MEMBER_UINT32(LSA_TRUSTED_DOMAIN_INFO, dwTrustFlags),
    LWMSG_MEMBER_UINT32(LSA_TRUSTED_DOMAIN_INFO, dwTrustType),
    LWMSG_MEMBER_UINT32(LSA_TRUSTED_DOMAIN_INFO, dwTrustAttributes),
    LWMSG_MEMBER_UINT32(LSA_TRUSTED_DOMAIN_INFO, dwTrustDirection),
    LWMSG_MEMBER_UINT32(LSA_TRUSTED_DOMAIN_INFO, dwTrustMode),
    LWMSG_MEMBER_UINT32(LSA_TRUSTED_DOMAIN_INFO, dwDomainFlags),
    LWMSG_MEMBER_POINTER_BEGIN(LSA_TRUSTED_DOMAIN_INFO, pDCInfo),
    LWMSG_TYPESPEC(gLsaDcInfoSpec),
    LWMSG_POINTER_END,
    LWMSG_MEMBER_POINTER_BEGIN(LSA_TRUSTED_DOMAIN_INFO, pGCInfo),
    LWMSG_TYPESPEC(gLsaDcInfoSpec),
    LWMSG_POINTER_END,
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaAuthProviderStatusSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_AUTH_PROVIDER_STATUS),
    LWMSG_MEMBER_PSTR(LSA_AUTH_PROVIDER_STATUS, pszId),
    LWMSG_MEMBER_UINT8(LSA_AUTH_PROVIDER_STATUS, mode),
    LWMSG_MEMBER_UINT8(LSA_AUTH_PROVIDER_STATUS, subMode),
    LWMSG_MEMBER_UINT8(LSA_AUTH_PROVIDER_STATUS, status),
    LWMSG_MEMBER_PSTR(LSA_AUTH_PROVIDER_STATUS, pszDomain),
    LWMSG_MEMBER_PSTR(LSA_AUTH_PROVIDER_STATUS, pszForest),
    LWMSG_MEMBER_PSTR(LSA_AUTH_PROVIDER_STATUS, pszSite),
    LWMSG_MEMBER_PSTR(LSA_AUTH_PROVIDER_STATUS, pszCell),
    LWMSG_MEMBER_UINT32(LSA_AUTH_PROVIDER_STATUS, dwNetworkCheckInterval),
    LWMSG_MEMBER_UINT32(LSA_AUTH_PROVIDER_STATUS, dwNumTrustedDomains),
    LWMSG_MEMBER_POINTER_BEGIN(LSA_AUTH_PROVIDER_STATUS, pTrustedDomainInfoArray),
    LWMSG_TYPESPEC(gLsaTrustedDomainInfoSpec),
    LWMSG_POINTER_END,
    LWMSG_ATTR_LENGTH_MEMBER(LSA_AUTH_PROVIDER_STATUS, dwNumTrustedDomains),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaStatusSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSASTATUS),
    LWMSG_MEMBER_UINT32(LSASTATUS, dwUptime),
    LWMSG_MEMBER_STRUCT_BEGIN(LSASTATUS, version),
    LWMSG_MEMBER_UINT32(LSA_VERSION, dwMajor),
    LWMSG_MEMBER_UINT32(LSA_VERSION, dwMinor),
    LWMSG_MEMBER_UINT32(LSA_VERSION, dwBuild),
    LWMSG_STRUCT_END,
    LWMSG_MEMBER_UINT32(LSASTATUS, dwCount),
    LWMSG_MEMBER_POINTER_BEGIN(LSASTATUS, pAuthProviderStatusList),
    LWMSG_TYPESPEC(gLsaAuthProviderStatusSpec),
    LWMSG_POINTER_END,
    LWMSG_ATTR_LENGTH_MEMBER(LSASTATUS, dwCount),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaStatusPtrSpec[] =
{
    LWMSG_POINTER_BEGIN,
    LWMSG_TYPESPEC(gLsaStatusSpec),
    LWMSG_POINTER_END,
    LWMSG_TYPE_END
};

/* LsaIpcServerHandle (opaque) */
static LWMsgTypeSpec gLsaIpcEnumServerHandleSpec[] =
{
    /* Identify type name of handle */
    LWMSG_HANDLE(LsaIpcEnumServerHandle),
    /* End specification */
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCFindObjectByNameReqSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_IPC_FIND_OBJECT_BY_NAME_REQ),
    LWMSG_MEMBER_UINT32(LSA_IPC_FIND_OBJECT_BY_NAME_REQ, FindFlags),
    LWMSG_MEMBER_UINT32(LSA_IPC_FIND_OBJECT_BY_NAME_REQ, dwInfoLevel),
    LWMSG_MEMBER_PSTR(LSA_IPC_FIND_OBJECT_BY_NAME_REQ, pszName),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCFindNssArtefactByKeyReqSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_IPC_FIND_NSSARTEFACT_BY_KEY_REQ),
    LWMSG_MEMBER_UINT32(LSA_IPC_FIND_NSSARTEFACT_BY_KEY_REQ, dwFlags),
    LWMSG_MEMBER_UINT32(LSA_IPC_FIND_NSSARTEFACT_BY_KEY_REQ, dwInfoLevel),
    LWMSG_MEMBER_PSTR(LSA_IPC_FIND_NSSARTEFACT_BY_KEY_REQ, pszKeyName),
    LWMSG_MEMBER_PSTR(LSA_IPC_FIND_NSSARTEFACT_BY_KEY_REQ, pszMapName),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCFindObjectByIdReqSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_IPC_FIND_OBJECT_BY_ID_REQ),
    LWMSG_MEMBER_UINT32(LSA_IPC_FIND_OBJECT_BY_ID_REQ, FindFlags),
    LWMSG_MEMBER_UINT32(LSA_IPC_FIND_OBJECT_BY_ID_REQ, dwInfoLevel),
    LWMSG_MEMBER_UINT32(LSA_IPC_FIND_OBJECT_BY_ID_REQ, id),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCBeginObjectEnumReqSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_IPC_BEGIN_ENUM_RECORDS_REQ),
    /* handle - marshal as LsaIpcEnumServerHandleSpec (references existing spec) */
    LWMSG_MEMBER_TYPESPEC(LSA_IPC_BEGIN_ENUM_RECORDS_REQ, Handle, gLsaIpcEnumServerHandleSpec),
    LWMSG_ATTR_HANDLE_LOCAL_FOR_RECEIVER,
    LWMSG_MEMBER_UINT32(LSA_IPC_BEGIN_ENUM_RECORDS_REQ, dwInfoLevel),
    LWMSG_MEMBER_UINT32(LSA_IPC_BEGIN_ENUM_RECORDS_REQ, dwNumMaxRecords),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCBeginNssArtefactEnumReqSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_IPC_BEGIN_ENUM_NSSARTEFACT_REQ),
    /* handle - marshal as LsaIpcEnumServerHandleSpec (references existing spec) */
    LWMSG_MEMBER_TYPESPEC(LSA_IPC_BEGIN_ENUM_NSSARTEFACT_REQ, Handle, gLsaIpcEnumServerHandleSpec),
    LWMSG_ATTR_HANDLE_LOCAL_FOR_RECEIVER,
    LWMSG_MEMBER_UINT32(LSA_IPC_BEGIN_ENUM_NSSARTEFACT_REQ, dwInfoLevel),
    LWMSG_MEMBER_UINT32(LSA_IPC_BEGIN_ENUM_NSSARTEFACT_REQ, dwMaxNumNSSArtefacts),
    LWMSG_MEMBER_UINT32(LSA_IPC_BEGIN_ENUM_NSSARTEFACT_REQ, dwFlags),
    LWMSG_MEMBER_PSTR(LSA_IPC_BEGIN_ENUM_NSSARTEFACT_REQ, pszMapName),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};


static LWMsgTypeSpec gLsaIPCEnumObjectReqSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_IPC_ENUM_RECORDS_REQ),
    /* handle - marshal as LsaIpcEnumServerHandleSpec (references existing spec) */
    LWMSG_MEMBER_TYPESPEC(LSA_IPC_ENUM_RECORDS_REQ, Handle, gLsaIpcEnumServerHandleSpec),
    LWMSG_ATTR_HANDLE_LOCAL_FOR_RECEIVER,
    LWMSG_MEMBER_PSTR(LSA_IPC_ENUM_RECORDS_REQ, pszToken),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCAddGroupInfoReqSpec[] =
{
    LWMSG_POINTER_BEGIN,
    LWMSG_TYPESPEC(gLsaGroupInfoListSpec),
    LWMSG_POINTER_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCAddUserInfoReqSpec[] =
{
    LWMSG_POINTER_BEGIN,
    LWMSG_TYPESPEC(gLsaUserInfoListSpec),
    LWMSG_POINTER_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCDelObjectInfoReqSpec[] =
{
    LWMSG_POINTER_BEGIN,
    LWMSG_UINT32(DWORD),
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCAuthUserReqSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_IPC_AUTH_USER_REQ),
    LWMSG_MEMBER_PSTR(LSA_IPC_AUTH_USER_REQ, pszLoginName),
    LWMSG_MEMBER_PSTR(LSA_IPC_AUTH_USER_REQ, pszPassword),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCChangePasswordReqSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_IPC_CHANGE_PASSWORD_REQ),
    LWMSG_MEMBER_PSTR(LSA_IPC_CHANGE_PASSWORD_REQ, pszLoginName),
    LWMSG_MEMBER_PSTR(LSA_IPC_CHANGE_PASSWORD_REQ, pszNewPassword),
    LWMSG_MEMBER_PSTR(LSA_IPC_CHANGE_PASSWORD_REQ, pszOldPassword),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCOpenOrCloseSessionReqSpec[] =
{
    LWMSG_PSTR,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCModUserInfoReqSpec[] =
{
    LWMSG_POINTER_BEGIN,
    LWMSG_TYPESPEC(gLsaIPCUserModInfoSpec),
    LWMSG_POINTER_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCFindNamesBySidListReqSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_IPC_NAMES_BY_SIDS_REQ),
    LWMSG_MEMBER_UINT32(LSA_IPC_NAMES_BY_SIDS_REQ, sCount),
    LWMSG_MEMBER_POINTER_BEGIN(LSA_IPC_NAMES_BY_SIDS_REQ, ppszSidList),
    LWMSG_PSTR,
    LWMSG_POINTER_END,
    LWMSG_ATTR_STRING,
    LWMSG_ATTR_LENGTH_MEMBER(LSA_IPC_NAMES_BY_SIDS_REQ, sCount),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCMakeAuthMsgReqSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_IPC_MAKE_AUTH_MSG_REQ),
    LWMSG_MEMBER_UINT32(LSA_IPC_MAKE_AUTH_MSG_REQ, negotiateFlags),
    LWMSG_MEMBER_TYPESPEC(LSA_IPC_MAKE_AUTH_MSG_REQ, credentials, gLsaSecBufferSpec),
    LWMSG_MEMBER_TYPESPEC(LSA_IPC_MAKE_AUTH_MSG_REQ, serverChallenge, gLsaSecBufferSSpec),
    LWMSG_MEMBER_TYPESPEC(LSA_IPC_MAKE_AUTH_MSG_REQ, targetInfo, gLsaSecBufferSpec),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCCheckAuthMsgReqSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_IPC_CHECK_AUTH_MSG_REQ),
    LWMSG_MEMBER_UINT32(LSA_IPC_CHECK_AUTH_MSG_REQ, negotiateFlags),
    LWMSG_MEMBER_TYPESPEC(LSA_IPC_CHECK_AUTH_MSG_REQ, serverChallenge, gLsaSecBufferSSpec),
    LWMSG_MEMBER_TYPESPEC(LSA_IPC_CHECK_AUTH_MSG_REQ, targetInfo, gLsaSecBufferSpec),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCLoginfoSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_LOG_INFO),
    LWMSG_MEMBER_UINT8(LSA_LOG_INFO, maxAllowedLogLevel),
    LWMSG_MEMBER_UINT8(LSA_LOG_INFO, logTarget),
    LWMSG_MEMBER_PSTR(LSA_LOG_INFO, pszPath),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCSetLoginfoReqSpec[] =
{
    LWMSG_POINTER_BEGIN,
    LWMSG_TYPESPEC(gLsaIPCLoginfoSpec),
    LWMSG_POINTER_END,
    LWMSG_TYPE_END
};


static LWMsgTypeSpec gLsaIPCGetMetricsReqSpec[] =
{
    LWMSG_POINTER_BEGIN,
    LWMSG_UINT32(DWORD),
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCCheckUserInListReqSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_IPC_CHECK_USER_IN_LIST_REQ),
    LWMSG_MEMBER_PSTR(LSA_IPC_CHECK_USER_IN_LIST_REQ, pszLoginName),
    LWMSG_MEMBER_PSTR(LSA_IPC_CHECK_USER_IN_LIST_REQ, pszListName),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaTraceinfoSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_TRACE_INFO),
    LWMSG_MEMBER_UINT32(LSA_TRACE_INFO, dwTraceFlag),
    LWMSG_MEMBER_INT8(LSA_TRACE_INFO, bStatus),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaTraceinfoArraySpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_TRACE_INFO_LIST),
    LWMSG_MEMBER_UINT32(LSA_TRACE_INFO_LIST, dwNumFlags),
    LWMSG_MEMBER_POINTER_BEGIN(LSA_TRACE_INFO_LIST, pTraceInfoArray),
    LWMSG_TYPESPEC(gLsaTraceinfoSpec),
    LWMSG_POINTER_END,
    LWMSG_ATTR_LENGTH_MEMBER(LSA_TRACE_INFO_LIST, dwNumFlags),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCSetTraceinfoReqSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_IPC_SET_TRACE_INFO_REQ),
    LWMSG_MEMBER_UINT32(LSA_IPC_SET_TRACE_INFO_REQ, dwNumFlags),
    LWMSG_MEMBER_POINTER_BEGIN(LSA_IPC_SET_TRACE_INFO_REQ, pTraceFlagArray),
    LWMSG_TYPESPEC(gLsaTraceinfoSpec),
    LWMSG_POINTER_END,
    LWMSG_ATTR_LENGTH_MEMBER(LSA_IPC_SET_TRACE_INFO_REQ, dwNumFlags),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCDataBlobSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_DATA_BLOB),
    LWMSG_MEMBER_UINT32(LSA_DATA_BLOB, dwLen),
    LWMSG_MEMBER_POINTER_BEGIN(LSA_DATA_BLOB, pData),
    LWMSG_UINT8(BYTE),
    LWMSG_POINTER_END,
    LWMSG_ATTR_LENGTH_MEMBER(LSA_DATA_BLOB, dwLen),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCAuthClearTextParamSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_AUTH_CLEARTEXT_PARAM),
    LWMSG_MEMBER_PSTR(LSA_AUTH_CLEARTEXT_PARAM, pszPassword),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCAuthChapParamSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_AUTH_CHAP_PARAM),
    LWMSG_MEMBER_POINTER_BEGIN(LSA_AUTH_CHAP_PARAM, pChallenge),
    LWMSG_TYPESPEC(gLsaIPCDataBlobSpec),
    LWMSG_POINTER_END,
    LWMSG_MEMBER_POINTER_BEGIN(LSA_AUTH_CHAP_PARAM, pLM_resp),
    LWMSG_TYPESPEC(gLsaIPCDataBlobSpec),
    LWMSG_POINTER_END,
    LWMSG_MEMBER_POINTER_BEGIN(LSA_AUTH_CHAP_PARAM, pNT_resp),
    LWMSG_TYPESPEC(gLsaIPCDataBlobSpec),
    LWMSG_POINTER_END,
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCAuthUserParamSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_AUTH_USER_PARAMS),
    LWMSG_MEMBER_UINT8(LSA_AUTH_USER_PARAMS, AuthType),
    LWMSG_MEMBER_PSTR(LSA_AUTH_USER_PARAMS, pszAccountName),
    LWMSG_MEMBER_PSTR(LSA_AUTH_USER_PARAMS, pszDomain),
    LWMSG_MEMBER_PSTR(LSA_AUTH_USER_PARAMS, pszWorkstation),
    LWMSG_MEMBER_UNION_BEGIN(LSA_AUTH_USER_PARAMS, pass),
    LWMSG_MEMBER_TYPESPEC(union _PASS, clear, gLsaIPCAuthClearTextParamSpec),
    LWMSG_ATTR_TAG(LSA_AUTH_PLAINTEXT),
    LWMSG_MEMBER_TYPESPEC(union _PASS, chap, gLsaIPCAuthChapParamSpec),
    LWMSG_ATTR_TAG(LSA_AUTH_CHAP),
    LWMSG_UNION_END,
    LWMSG_ATTR_DISCRIM(LSA_AUTH_USER_PARAMS, AuthType),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCAuthUserExReqSpec[] =
{
    LWMSG_POINTER_BEGIN,
    LWMSG_TYPESPEC(gLsaIPCAuthUserParamSpec),
    LWMSG_POINTER_END,
    LWMSG_TYPE_END
};


static LWMsgTypeSpec gLsaIPCLsaSidSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_SID),

    LWMSG_MEMBER_UINT8(LSA_SID, Revision),
    LWMSG_MEMBER_UINT8(LSA_SID, NumSubAuths),

    LWMSG_MEMBER_ARRAY_BEGIN(LSA_SID, AuthId),
    LWMSG_UINT8(BYTE),
    LWMSG_ARRAY_END,
    LWMSG_ATTR_LENGTH_STATIC(6),

    LWMSG_MEMBER_ARRAY_BEGIN(LSA_SID, SubAuths),
    LWMSG_UINT32(BYTE),
    LWMSG_ARRAY_END,
    LWMSG_ATTR_LENGTH_MEMBER(LSA_SID, NumSubAuths),

    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCLsaSidPtrSpec[] =
{
    LWMSG_POINTER_BEGIN,
    LWMSG_TYPESPEC(gLsaIPCLsaSidSpec),
    LWMSG_POINTER_END,
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCLsaSidAttribSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_SID_ATTRIB),
    LWMSG_MEMBER_TYPESPEC(LSA_SID_ATTRIB, Sid, gLsaIPCLsaSidSpec),
    LWMSG_MEMBER_UINT32(LSA_SID_ATTRIB, dwAttrib),
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCLsaSidAttribPtrSpec[] =
{
    LWMSG_POINTER_BEGIN,
    LWMSG_TYPESPEC(gLsaIPCLsaSidAttribSpec),
    LWMSG_POINTER_END,
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaAuthUserInfoSpec[] =
{
    LWMSG_STRUCT_BEGIN(LSA_AUTH_USER_INFO),

    LWMSG_MEMBER_UINT32(LSA_AUTH_USER_INFO, dwUserFlags),

    LWMSG_MEMBER_PSTR(LSA_AUTH_USER_INFO, pszAccount),
    LWMSG_MEMBER_PSTR(LSA_AUTH_USER_INFO, pszUserPrincipalName),
    LWMSG_MEMBER_PSTR(LSA_AUTH_USER_INFO, pszFullName),
    LWMSG_MEMBER_PSTR(LSA_AUTH_USER_INFO, pszDomain),
    LWMSG_MEMBER_PSTR(LSA_AUTH_USER_INFO, pszDnsDomain),

    LWMSG_MEMBER_UINT32(LSA_AUTH_USER_INFO, dwAcctFlags),

    LWMSG_MEMBER_POINTER_BEGIN(LSA_AUTH_USER_INFO, pSessionKey),
    LWMSG_TYPESPEC(gLsaIPCDataBlobSpec),
    LWMSG_POINTER_END,

    LWMSG_MEMBER_POINTER_BEGIN(LSA_AUTH_USER_INFO, pLmSessionKey),
    LWMSG_TYPESPEC(gLsaIPCDataBlobSpec),
    LWMSG_POINTER_END,

    LWMSG_MEMBER_UINT16(LSA_AUTH_USER_INFO, LogonCount),
    LWMSG_MEMBER_UINT16(LSA_AUTH_USER_INFO, BadPasswordCount),

    LWMSG_MEMBER_INT64(LSA_AUTH_USER_INFO, LogonTime),
    LWMSG_MEMBER_INT64(LSA_AUTH_USER_INFO, LogoffTime),
    LWMSG_MEMBER_INT64(LSA_AUTH_USER_INFO, KickoffTime),
    LWMSG_MEMBER_INT64(LSA_AUTH_USER_INFO, LastPasswordChange),
    LWMSG_MEMBER_INT64(LSA_AUTH_USER_INFO, CanChangePassword),
    LWMSG_MEMBER_INT64(LSA_AUTH_USER_INFO, MustChangePassword),

    LWMSG_MEMBER_PSTR(LSA_AUTH_USER_INFO, pszLogonServer),
    LWMSG_MEMBER_PSTR(LSA_AUTH_USER_INFO, pszLogonScript),
    LWMSG_MEMBER_PSTR(LSA_AUTH_USER_INFO, pszProfilePath),
    LWMSG_MEMBER_PSTR(LSA_AUTH_USER_INFO, pszHomeDirectory),
    LWMSG_MEMBER_PSTR(LSA_AUTH_USER_INFO, pszHomeDrive),

    LWMSG_MEMBER_TYPESPEC(LSA_AUTH_USER_INFO, DomainSid, gLsaIPCLsaSidSpec),
    LWMSG_MEMBER_UINT32(LSA_AUTH_USER_INFO, dwUserRid),
    LWMSG_MEMBER_UINT32(LSA_AUTH_USER_INFO, dwPrimaryGroupRid),

    LWMSG_MEMBER_UINT32(LSA_AUTH_USER_INFO, dwNumSids),
    LWMSG_MEMBER_POINTER_BEGIN(LSA_AUTH_USER_INFO, pSidAttribList),
    LWMSG_TYPESPEC(gLsaIPCLsaSidAttribPtrSpec),
    LWMSG_POINTER_END,
    LWMSG_ATTR_LENGTH_MEMBER(LSA_AUTH_USER_INFO, dwNumSids),

    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaAuthUserInfoPtrSpec[] =
{
    LWMSG_POINTER_BEGIN,
    LWMSG_TYPESPEC(gLsaAuthUserInfoSpec),
    LWMSG_POINTER_END,
    LWMSG_STRUCT_END,
    LWMSG_TYPE_END
};

static LWMsgTypeSpec gLsaIPCGetTraceinfoReqSpec[] =
{
    LWMSG_POINTER_BEGIN,
    LWMSG_UINT32(DWORD),
    LWMSG_TYPE_END
};

static LWMsgProtocolSpec gLsaIPCSpec[] =
{
    LWMSG_MESSAGE(LSA_Q_OPEN_SERVER, NULL),
    LWMSG_MESSAGE(LSA_R_OPEN_SERVER_SUCCESS, gLsaIpcEnumServerHandleSpec),
    LWMSG_MESSAGE(LSA_R_OPEN_SERVER_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_GROUP_BY_NAME, gLsaIPCFindObjectByNameReqSpec),
    LWMSG_MESSAGE(LSA_R_GROUP_BY_NAME_SUCCESS, gLsaGroupInfoListSpec),
    LWMSG_MESSAGE(LSA_R_GROUP_BY_NAME_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_GROUP_BY_ID, gLsaIPCFindObjectByIdReqSpec),
    LWMSG_MESSAGE(LSA_R_GROUP_BY_ID_SUCCESS, gLsaGroupInfoListSpec),
    LWMSG_MESSAGE(LSA_R_GROUP_BY_ID_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_BEGIN_ENUM_GROUPS, gLsaIPCBeginObjectEnumReqSpec),
    LWMSG_MESSAGE(LSA_R_BEGIN_ENUM_GROUPS_SUCCESS, gLsaBeginObjectEnumSpec),
    LWMSG_MESSAGE(LSA_R_BEGIN_ENUM_GROUPS_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_ENUM_GROUPS, gLsaIPCEnumObjectReqSpec),
    LWMSG_MESSAGE(LSA_R_ENUM_GROUPS_SUCCESS, gLsaGroupInfoListSpec),
    LWMSG_MESSAGE(LSA_R_ENUM_GROUPS_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_END_ENUM_GROUPS, gLsaIPCEnumObjectReqSpec),
    LWMSG_MESSAGE(LSA_R_END_ENUM_GROUPS_SUCCESS, NULL),
    LWMSG_MESSAGE(LSA_R_END_ENUM_GROUPS_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_USER_BY_NAME, gLsaIPCFindObjectByNameReqSpec),
    LWMSG_MESSAGE(LSA_R_USER_BY_NAME_SUCCESS, gLsaUserInfoListSpec),
    LWMSG_MESSAGE(LSA_R_USER_BY_NAME_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_USER_BY_ID, gLsaIPCFindObjectByIdReqSpec),
    LWMSG_MESSAGE(LSA_R_USER_BY_ID_SUCCESS, gLsaUserInfoListSpec),
    LWMSG_MESSAGE(LSA_R_USER_BY_ID_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_BEGIN_ENUM_USERS, gLsaIPCBeginObjectEnumReqSpec),
    LWMSG_MESSAGE(LSA_R_BEGIN_ENUM_USERS_SUCCESS, gLsaBeginObjectEnumSpec),
    LWMSG_MESSAGE(LSA_R_BEGIN_ENUM_USERS_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_ENUM_USERS, gLsaIPCEnumObjectReqSpec),
    LWMSG_MESSAGE(LSA_R_ENUM_USERS_SUCCESS, gLsaUserInfoListSpec),
    LWMSG_MESSAGE(LSA_R_ENUM_USERS_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_END_ENUM_USERS, gLsaIPCEnumObjectReqSpec),
    LWMSG_MESSAGE(LSA_R_END_ENUM_USERS_SUCCESS, NULL),
    LWMSG_MESSAGE(LSA_R_END_ENUM_USERS_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_AUTH_USER, gLsaIPCAuthUserReqSpec),
    LWMSG_MESSAGE(LSA_R_AUTH_USER_SUCCESS, NULL),
    LWMSG_MESSAGE(LSA_R_AUTH_USER_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_AUTH_USER_EX, gLsaIPCAuthUserExReqSpec),
    LWMSG_MESSAGE(LSA_R_AUTH_USER_EX_SUCCESS, gLsaAuthUserInfoPtrSpec),
    LWMSG_MESSAGE(LSA_R_AUTH_USER_EX_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_VALIDATE_USER, gLsaIPCAuthUserReqSpec),
    LWMSG_MESSAGE(LSA_R_VALIDATE_USER_SUCCESS, NULL),
    LWMSG_MESSAGE(LSA_R_VALIDATE_USER_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_ADD_GROUP, gLsaIPCAddGroupInfoReqSpec),
    LWMSG_MESSAGE(LSA_R_ADD_GROUP_SUCCESS, NULL),
    LWMSG_MESSAGE(LSA_R_ADD_GROUP_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_DELETE_GROUP, gLsaIPCDelObjectInfoReqSpec),
    LWMSG_MESSAGE(LSA_R_DELETE_GROUP_SUCCESS, NULL),
    LWMSG_MESSAGE(LSA_R_DELETE_GROUP_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_ADD_USER, gLsaIPCAddUserInfoReqSpec),
    LWMSG_MESSAGE(LSA_R_ADD_USER_SUCCESS, NULL),
    LWMSG_MESSAGE(LSA_R_ADD_USER_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_DELETE_USER, gLsaIPCDelObjectInfoReqSpec),
    LWMSG_MESSAGE(LSA_R_DELETE_USER_SUCCESS, NULL),
    LWMSG_MESSAGE(LSA_R_DELETE_USER_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_CHANGE_PASSWORD, gLsaIPCChangePasswordReqSpec),
    LWMSG_MESSAGE(LSA_R_CHANGE_PASSWORD_SUCCESS, NULL),
    LWMSG_MESSAGE(LSA_R_CHANGE_PASSWORD_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_OPEN_SESSION, gLsaIPCOpenOrCloseSessionReqSpec),
    LWMSG_MESSAGE(LSA_R_OPEN_SESSION_SUCCESS, NULL),
    LWMSG_MESSAGE(LSA_R_OPEN_SESSION_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_CLOSE_SESSION, gLsaIPCOpenOrCloseSessionReqSpec),
    LWMSG_MESSAGE(LSA_R_CLOSE_SESSION_SUCCESS, NULL),
    LWMSG_MESSAGE(LSA_R_CLOSE_SESSION_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_MODIFY_USER, gLsaIPCModUserInfoReqSpec),
    LWMSG_MESSAGE(LSA_R_MODIFY_USER_SUCCESS, NULL),
    LWMSG_MESSAGE(LSA_R_MODIFY_USER_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_NAMES_BY_SID_LIST, gLsaIPCFindNamesBySidListReqSpec),
    LWMSG_MESSAGE(LSA_R_NAMES_BY_SID_LIST_SUCCESS, gLsaNamesBySidsSpec),
    LWMSG_MESSAGE(LSA_R_NAMES_BY_SID_LIST_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_CHECK_USER_IN_LIST, gLsaIPCCheckUserInListReqSpec),
    LWMSG_MESSAGE(LSA_R_CHECK_USER_IN_LIST_SUCCESS, NULL),
    LWMSG_MESSAGE(LSA_R_CHECK_USER_IN_LIST_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_GSS_MAKE_AUTH_MSG, gLsaIPCMakeAuthMsgReqSpec),
    LWMSG_MESSAGE(LSA_R_GSS_MAKE_AUTH_MSG_SUCCESS, gLsaIPCMakeAuthMsgReplySpec),
    LWMSG_MESSAGE(LSA_R_GSS_MAKE_AUTH_MSG_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_GSS_CHECK_AUTH_MSG, gLsaIPCCheckAuthMsgReqSpec),
    LWMSG_MESSAGE(LSA_R_GSS_CHECK_AUTH_MSG_SUCCESS, gLsaIPCCheckAuthMsgReplySpec),
    LWMSG_MESSAGE(LSA_R_GSS_CHECK_AUTH_MSG_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_GROUPS_FOR_USER, gLsaIPCFindObjectByIdReqSpec),
    LWMSG_MESSAGE(LSA_R_GROUPS_FOR_USER_SUCCESS, gLsaGroupInfoListSpec),
    LWMSG_MESSAGE(LSA_R_GROUPS_FOR_USER_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_SET_LOGINFO, gLsaIPCSetLoginfoReqSpec),
    LWMSG_MESSAGE(LSA_R_SET_LOGINFO_SUCCESS, NULL),
    LWMSG_MESSAGE(LSA_R_SET_LOGINFO_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_GET_LOGINFO, NULL),
    LWMSG_MESSAGE(LSA_R_GET_LOGINFO_SUCCESS, gLsaIPCLoginfoSpec),
    LWMSG_MESSAGE(LSA_R_GET_LOGINFO_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_GET_METRICS, gLsaIPCGetMetricsReqSpec),
    LWMSG_MESSAGE(LSA_R_GET_METRICS_SUCCESS, gLsaMetricPackSpec),
    LWMSG_MESSAGE(LSA_R_GET_METRICS_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_GET_STATUS, NULL),
    LWMSG_MESSAGE(LSA_R_GET_STATUS_SUCCESS, gLsaStatusPtrSpec),
    LWMSG_MESSAGE(LSA_R_GET_STATUS_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_REFRESH_CONFIGURATION, NULL),
    LWMSG_MESSAGE(LSA_R_REFRESH_CONFIGURATION_SUCCESS, NULL),
    LWMSG_MESSAGE(LSA_R_REFRESH_CONFIGURATION_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_BEGIN_ENUM_NSS_ARTEFACTS, gLsaIPCBeginNssArtefactEnumReqSpec),
    LWMSG_MESSAGE(LSA_R_BEGIN_ENUM_NSS_ARTEFACTS_SUCCESS, gLsaBeginObjectEnumSpec),
    LWMSG_MESSAGE(LSA_R_BEGIN_ENUM_NSS_ARTEFACTS_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_ENUM_NSS_ARTEFACTS, gLsaIPCEnumObjectReqSpec),
    LWMSG_MESSAGE(LSA_R_ENUM_NSS_ARTEFACTS_SUCCESS, gLsaNssArtefactInfoListSpec),
    LWMSG_MESSAGE(LSA_R_ENUM_NSS_ARTEFACTS_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_END_ENUM_NSS_ARTEFACTS, gLsaIPCEnumObjectReqSpec),
    LWMSG_MESSAGE(LSA_R_END_ENUM_NSS_ARTEFACTS_SUCCESS, NULL),
    LWMSG_MESSAGE(LSA_R_END_ENUM_NSS_ARTEFACTS_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_FIND_NSS_ARTEFACT_BY_KEY, gLsaIPCFindNssArtefactByKeyReqSpec),
    LWMSG_MESSAGE(LSA_R_FIND_NSS_ARTEFACT_BY_KEY_SUCCESS, gLsaNssArtefactInfoListSpec),
    LWMSG_MESSAGE(LSA_R_FIND_NSS_ARTEFACT_BY_KEY_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_SET_TRACE_INFO, gLsaIPCSetTraceinfoReqSpec),
    LWMSG_MESSAGE(LSA_R_SET_TRACE_INFO_SUCCESS, NULL),
    LWMSG_MESSAGE(LSA_R_SET_TRACE_INFO_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_GET_TRACE_INFO, gLsaIPCGetTraceinfoReqSpec),
    LWMSG_MESSAGE(LSA_R_GET_TRACE_INFO_SUCCESS, gLsaTraceinfoArraySpec),
    LWMSG_MESSAGE(LSA_R_GET_TRACE_INFO_FAILURE, gLsaIPCErrorSpec),
    LWMSG_MESSAGE(LSA_Q_ENUM_TRACE_INFO, NULL),
    LWMSG_MESSAGE(LSA_R_ENUM_TRACE_INFO_SUCCESS, gLsaTraceinfoArraySpec),
    LWMSG_MESSAGE(LSA_R_ENUM_TRACE_INFO_FAILURE, gLsaIPCErrorSpec),
    LWMSG_PROTOCOL_END
};

LWMsgProtocolSpec*
LsaIPCGetProtocolSpec(
    void
    )
{
    return gLsaIPCSpec;
}
