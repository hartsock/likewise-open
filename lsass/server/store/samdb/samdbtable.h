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
 *        samdbtable.h
 *
 * Abstract:
 *
 *
 *      Likewise SAM Database Provider
 *
 *      Database Schema
 *
 * Authors: Krishna Ganugapati (krishnag@likewisesoftware.com)
 *
 */

#ifndef __SAM_DB_TABLE_H__
#define __SAM_DB_TABLE_H__

#define SAM_DB_SCHEMA_VERSION 1
#define SAM_DB_MIN_UID        1000
#define SAM_DB_MIN_GID        1000
#define SAM_DB_MIN_RID        1000

#define SAM_DB_CONFIG_TABLE  "samdbconfig"
#define SAM_DB_OBJECTS_TABLE "samdbobjects"
#define SAM_DB_MEMBERS_TABLE "samdbmembers"

#define SAM_DB_COL_RECORD_ID            "ObjectRecordId"
#define SAM_DB_COL_OBJECT_SID           "ObjectSID"
#define SAM_DB_COL_DISTINGUISHED_NAME   "DistinguishedName"
#define SAM_DB_COL_PARENT_DN            "ParentDN"
#define SAM_DB_COL_OBJECT_CLASS         "ObjectClass"
#define SAM_DB_COL_DOMAIN               "Domain"
#define SAM_DB_COL_NETBIOS_NAME         "NetBIOSName"
#define SAM_DB_COL_COMMON_NAME          "CommonName"
#define SAM_DB_COL_SAM_ACCOUNT_NAME     "SamAccountName"
#define SAM_DB_COL_USER_PRINCIPAL_NAME  "UserPrincipalName"
#define SAM_DB_COL_DESCRIPTION          "Description"
#define SAM_DB_COL_UID                  "UID"
#define SAM_DB_COL_PASSWORD             "Password"
#define SAM_DB_COL_ACCOUNT_FLAGS        "AccountFlags"
#define SAM_DB_COL_GECOS                "Gecos"
#define SAM_DB_COL_HOME_DIR             "Homedir"
#define SAM_DB_COL_SHELL                "LoginShell"
#define SAM_DB_COL_PASSWORD_CHANGE_TIME "PasswdChangeTime"
#define SAM_DB_COL_FULL_NAME            "FullName"
#define SAM_DB_COL_ACCOUNT_EXPIRY       "AccountExpiry"
#define SAM_DB_COL_LM_HASH              "LMHash"
#define SAM_DB_COL_NT_HASH              "NTHash"
#define SAM_DB_COL_PRIMARY_GROUP        "PrimaryGroup"
#define SAM_DB_COL_GID                  "GID"
#define SAM_DB_COL_CREATED_TIME         "CreatedTime"

#define SAM_DB_QUERY_CREATE_TABLES  \
    "CREATE TABLE " SAM_DB_CONFIG_TABLE " (\n"                                 \
                 "UIDCounter INTEGER,\n"                                       \
                 "GIDCounter INTEGER,\n"                                       \
                 "RIDCounter INTEGER,\n"                                       \
                 "Version    INTEGER\n"                                        \
                 ");\n"                                                        \
    "CREATE TABLE " SAM_DB_OBJECTS_TABLE " (\n"                                \
                 SAM_DB_COL_RECORD_ID " INTEGER PRIMARY KEY AUTOINCREMENT,\n"  \
                 SAM_DB_COL_OBJECT_SID           " TEXT COLLATE NOCASE,\n"     \
                 SAM_DB_COL_DISTINGUISHED_NAME   " TEXT COLLATE NOCASE,\n"     \
                 SAM_DB_COL_PARENT_DN            " TEXT,\n"                    \
                 SAM_DB_COL_OBJECT_CLASS         " INTEGER,\n"                 \
                 SAM_DB_COL_DOMAIN               " TEXT COLLATE NOCASE,\n"     \
                 SAM_DB_COL_NETBIOS_NAME         " TEXT COLLATE NOCASE,\n"     \
                 SAM_DB_COL_COMMON_NAME          " TEXT,\n"                    \
                 SAM_DB_COL_SAM_ACCOUNT_NAME     " TEXT COLLATE NOCASE,\n"     \
                 SAM_DB_COL_USER_PRINCIPAL_NAME  " TEXT COLLATE NOCASE,\n"     \
                 SAM_DB_COL_DESCRIPTION          " TEXT,\n"                    \
                 SAM_DB_COL_UID                  " INTEGER,\n"                 \
                 SAM_DB_COL_PASSWORD             " TEXT,\n"                    \
                 SAM_DB_COL_ACCOUNT_FLAGS        " INTEGER,\n"                 \
                 SAM_DB_COL_GECOS                " TEXT,\n"                    \
                 SAM_DB_COL_HOME_DIR             " TEXT,\n"                    \
                 SAM_DB_COL_SHELL                " TEXT,\n"                    \
                 SAM_DB_COL_PASSWORD_CHANGE_TIME " INTEGER,\n"                 \
                 SAM_DB_COL_FULL_NAME            " TEXT,\n"                    \
                 SAM_DB_COL_ACCOUNT_EXPIRY       " INTEGER,\n"                 \
                 SAM_DB_COL_LM_HASH              " BLOB,\n"                    \
                 SAM_DB_COL_NT_HASH              " BLOB,\n"                    \
                 SAM_DB_COL_PRIMARY_GROUP        " INTEGER,\n"                 \
                 SAM_DB_COL_GID                  " INTEGER,\n"                 \
                 SAM_DB_COL_CREATED_TIME " DATE DEFAULT (DATETIME('now')),\n"  \
     "UNIQUE(" SAM_DB_COL_OBJECT_SID ", " SAM_DB_COL_DISTINGUISHED_NAME "),\n" \
     "UNIQUE(" SAM_DB_COL_DISTINGUISHED_NAME ", " SAM_DB_COL_PARENT_DN "),\n"  \
     "CHECK(" SAM_DB_COL_OBJECT_CLASS      " == 1 \n"                          \
            " OR " SAM_DB_COL_OBJECT_CLASS " == 2 \n"                          \
            " OR " SAM_DB_COL_OBJECT_CLASS " == 3 \n"                          \
            " OR " SAM_DB_COL_OBJECT_CLASS " == 4 \n"                          \
            " OR " SAM_DB_COL_OBJECT_CLASS " == 5)\n"                          \
                 ");\n"                                                        \
    "CREATE TABLE " SAM_DB_MEMBERS_TABLE " (\n"                                \
                 "GroupRecordId  INTEGER,\n"                                   \
                 "MemberRecordId INTEGER,\n"                                   \
                 "CreatedTime    DATE NOT NULL DEFAULT (DATETIME('now')),\n"   \
                 "UNIQUE(GroupRecordId, MemberRecordId),\n"                    \
                 "FOREIGN KEY (GroupRecordId) \n"                              \
                 "    REFERENCES " SAM_DB_OBJECTS_TABLE " (ObjectRecordId),\n" \
                 "FOREIGN KEY (MemberRecordId) \n"                             \
                 "    REFERENCES " SAM_DB_OBJECTS_TABLE " (ObjectRecordId)\n"  \
                 ");\n"                                                        \
    "CREATE TRIGGER samdbobjects_delete_object \n"                             \
    "AFTER  DELETE on " SAM_DB_OBJECTS_TABLE "\n"                              \
    "BEGIN\n"                                                                  \
    "  DELETE FROM " SAM_DB_MEMBERS_TABLE "\n"                                 \
    "   WHERE GroupRecordId = old.ObjectRecordId;\n"                           \
    "END;\n"

typedef enum
{
    SAMDB_OBJECT_CLASS_UNKNOWN        = 0,
    SAMDB_OBJECT_CLASS_DOMAIN         = 1,
    SAMDB_OBJECT_CLASS_BUILTIN_DOMAIN = 2,
    SAMDB_OBJECT_CLASS_CONTAINER      = 3,
    SAMDB_OBJECT_CLASS_GROUP          = 4,
    SAMDB_OBJECT_CLASS_USER           = 5,
    SAMDB_OBJECT_CLASS_SENTINEL

} SAMDB_OBJECT_CLASS;

typedef enum
{
    SAMDB_ATTR_TYPE_UNKNOWN = 0,
    SAMDB_ATTR_TYPE_TEXT,
    SAMDB_ATTR_TYPE_INT32,
    SAMDB_ATTR_TYPE_INT64,
    SAMDB_ATTR_TYPE_BOOLEAN,
    SAMDB_ATTR_TYPE_BLOB,
    SAMDB_ATTR_TYPE_DATETIME
} SAMDB_ATTR_TYPE;

#define SAM_DB_DIR_ATTR_NAME_MAX_LEN 32
#define SAM_DB_COL_NAME_MAX_LEN      32

/* These are the strings exchanged through the Directory API */
#define SAM_DB_DIR_ATTR_RECORD_ID \
    {'O','b','j','e','c','t','R','e','c','o','r','d','I','d',0}
#define SAM_DB_DIR_ATTR_OBJECT_SID \
    {'O','b','j','e','c','t','S','I','D',0}
#define SAM_DB_DIR_ATTR_DISTINGUISHED_NAME  \
    {'D','i','s','t','i','n','g','u','i','s','h','e','d','N','a','m','e',0}
#define SAM_DB_DIR_ATTR_PARENT_DN \
    {'P','a','r','e','n','t','D','N',0}
#define SAM_DB_DIR_ATTR_OBJECT_CLASS \
    {'O','b','j','e','c','t','C','l','a','s','s',0}
#define SAM_DB_DIR_ATTR_DOMAIN \
    {'D','o','m','a','i','n',0}
#define SAM_DB_DIR_ATTR_NETBIOS_NAME \
    {'N','e','t','B','I','O','S','N','a','m','e',0}
#define SAM_DB_DIR_ATTR_COMMON_NAME \
    {'C','o','m','m','o','n','N','a','m','e',0}
#define SAM_DB_DIR_ATTR_SAM_ACCOUNT_NAME \
    {'S','a','m','A','c','c','o','u','n','t','N','a','m','e',0}
#define SAM_DB_DIR_ATTR_USER_PRINCIPAL_NAME \
    {'U','s','e','r','P','r','i','n','c','i','p','a','l','N','a','m','e',0}
#define SAM_DB_DIR_ATTR_DESCRIPTION \
    {'D','e','s','c','r','i','p','t','i','o','n',0}
#define SAM_DB_DIR_ATTR_UID \
    {'U','I','D',0}
#define SAM_DB_DIR_ATTR_PASSWORD \
    {'P','a','s','s','w','o','r','d',0}
#define SAM_DB_DIR_ATTR_ACCOUNT_FLAGS \
    {'A','c','c','o','u','n','t','F','l','a','g','s',0}
#define SAM_DB_DIR_ATTR_GECOS \
    {'G','e','c','o','s',0}
#define SAM_DB_DIR_ATTR_HOME_DIR \
    {'H','o','m','e','d','i','r',0}
#define SAM_DB_DIR_ATTR_SHELL \
    {'L','o','g','i','n','S','h','e','l','l',0}
#define SAM_DB_DIR_ATTR_PASSWORD_CHANGE_TIME \
    {'P','a','s','s','w','d','C','h','a','n','g','e','T','i','m','e',0}
#define SAM_DB_DIR_ATTR_FULL_NAME \
    {'F','u','l','l','N','a','m','e',0}
#define SAM_DB_DIR_ATTR_ACCOUNT_EXPIRY \
    {'A','c','c','o','u','n','t','E','x','p','i','r','y',0}
#define SAM_DB_DIR_ATTR_LM_HASH \
    {'L','M','H','a','s','h',0}
#define SAM_DB_DIR_ATTR_NT_HASH \
    {'N','T','H','a','s','h',0}
#define SAM_DB_DIR_ATTR_PRIMARY_GROUP \
    {'P','r','i','m','a','r','y','G','r','o','u','p',0}
#define SAM_DB_DIR_ATTR_GID \
    {'G','I','D',0}
#define SAM_DB_DIR_ATTR_MEMBERS \
    {'M','e','m','b','e','r','s',0};
#define SAM_DB_DIR_ATTR_CREATED_TIME \
    {'C','r','e','a','t','e','d','T','i','m','e',0}

typedef DWORD SAM_DB_ATTR_FLAGS;

#define SAM_DB_ATTR_FLAGS_NONE                       0x00000000
#define SAM_DB_ATTR_FLAGS_MANDATORY                  0x00000001
#define SAM_DB_ATTR_FLAGS_READONLY                   0x00000002
#define SAM_DB_ATTR_FLAGS_GENERATE_IF_NOT_SPECIFIED  0x00000004
#define SAM_DB_ATTR_FLAGS_GENERATE_ALWAYS            0x00000008
#define SAM_DB_ATTR_FLAGS_GENERATED_BY_DB            0x00000010
#define SAM_DB_ATTR_FLAGS_DERIVATIVE                 0x00000020

#define SAM_DB_IS_A_ROW_ID         TRUE
#define SAM_DB_IS_NOT_A_ROW_ID     FALSE
#define SAM_DB_IS_MULTI_VALUED     TRUE
#define SAM_DB_IS_NOT_MULTI_VALUED FALSE
#define SAM_DB_IS_QUERYABLE        TRUE
#define SAM_DB_IS_NOT_QUERYABLE    FALSE

typedef struct _SAM_DB_ATTRIBUTE_MAP
{
    wchar16_t       wszDirectoryAttribute[SAM_DB_DIR_ATTR_NAME_MAX_LEN];
    CHAR            szDbColumnName[SAM_DB_COL_NAME_MAX_LEN];
    SAMDB_ATTR_TYPE attributeType;
    BOOLEAN         bIsRowId;
    BOOLEAN         bIsMultiValued;
    BOOLEAN         bIsQueryable;

} SAM_DB_ATTRIBUTE_MAP, *PSAM_DB_ATTRIBUTE_MAP;

#define SAMDB_OBJECT_ATTRIBUTE_MAP            \
    {                                         \
        SAM_DB_DIR_ATTR_RECORD_ID,            \
        SAM_DB_COL_RECORD_ID,                 \
        SAMDB_ATTR_TYPE_INT64,                \
        SAM_DB_IS_A_ROW_ID,                   \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_QUERYABLE                   \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_OBJECT_SID,           \
        SAM_DB_COL_OBJECT_SID,                \
        SAMDB_ATTR_TYPE_TEXT,                 \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_QUERYABLE                   \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_DISTINGUISHED_NAME,   \
        SAM_DB_COL_DISTINGUISHED_NAME,        \
        SAMDB_ATTR_TYPE_TEXT,                 \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_QUERYABLE                   \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_PARENT_DN,            \
        SAM_DB_COL_PARENT_DN,                 \
        SAMDB_ATTR_TYPE_TEXT,                 \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_QUERYABLE                   \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_OBJECT_CLASS,         \
        SAM_DB_COL_OBJECT_CLASS,              \
        SAMDB_ATTR_TYPE_INT32,                \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_QUERYABLE                   \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_DOMAIN,               \
        SAM_DB_COL_DOMAIN,                    \
        SAMDB_ATTR_TYPE_TEXT,                 \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_QUERYABLE                   \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_NETBIOS_NAME,         \
        SAM_DB_COL_NETBIOS_NAME,              \
        SAMDB_ATTR_TYPE_TEXT,                 \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_QUERYABLE                   \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_COMMON_NAME,          \
        SAM_DB_COL_COMMON_NAME,               \
        SAMDB_ATTR_TYPE_TEXT,                 \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_QUERYABLE                   \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_SAM_ACCOUNT_NAME,     \
        SAM_DB_COL_SAM_ACCOUNT_NAME,          \
        SAMDB_ATTR_TYPE_TEXT,                 \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_QUERYABLE                   \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_USER_PRINCIPAL_NAME,  \
        SAM_DB_COL_USER_PRINCIPAL_NAME,       \
        SAMDB_ATTR_TYPE_TEXT,                 \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_QUERYABLE                   \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_DESCRIPTION,          \
        SAM_DB_COL_DESCRIPTION,               \
        SAMDB_ATTR_TYPE_TEXT,                 \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_QUERYABLE                   \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_UID,                  \
        SAM_DB_COL_UID,                       \
        SAMDB_ATTR_TYPE_INT32,                \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_QUERYABLE                   \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_PASSWORD,             \
        SAM_DB_COL_PASSWORD,                  \
        SAMDB_ATTR_TYPE_TEXT,                 \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_NOT_QUERYABLE               \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_ACCOUNT_FLAGS,      \
        SAM_DB_COL_ACCOUNT_FLAGS  ,           \
        SAMDB_ATTR_TYPE_INT32,                \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_QUERYABLE                   \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_GECOS,                \
        SAM_DB_COL_GECOS,                     \
        SAMDB_ATTR_TYPE_TEXT,                 \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_QUERYABLE                   \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_HOME_DIR,             \
        SAM_DB_COL_HOME_DIR,                  \
        SAMDB_ATTR_TYPE_TEXT,                 \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_QUERYABLE                   \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_SHELL,                \
        SAM_DB_COL_SHELL,                     \
        SAMDB_ATTR_TYPE_TEXT,                 \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_QUERYABLE                   \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_PASSWORD_CHANGE_TIME, \
        SAM_DB_COL_PASSWORD_CHANGE_TIME,      \
        SAMDB_ATTR_TYPE_INT32,                \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_QUERYABLE                   \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_FULL_NAME,            \
        SAM_DB_COL_FULL_NAME,                 \
        SAMDB_ATTR_TYPE_TEXT,                 \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_QUERYABLE                   \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_ACCOUNT_EXPIRY,       \
        SAM_DB_COL_ACCOUNT_EXPIRY,            \
        SAMDB_ATTR_TYPE_INT64,                \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_QUERYABLE                   \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_LM_HASH,              \
        SAM_DB_COL_LM_HASH,                   \
        SAMDB_ATTR_TYPE_BLOB,                 \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_NOT_QUERYABLE               \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_NT_HASH,              \
        SAM_DB_COL_NT_HASH,                   \
        SAMDB_ATTR_TYPE_BLOB,                 \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_NOT_QUERYABLE               \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_PRIMARY_GROUP,        \
        SAM_DB_COL_PRIMARY_GROUP,             \
        SAMDB_ATTR_TYPE_INT32,                \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_QUERYABLE                   \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_GID,                  \
        SAM_DB_COL_GID,                       \
        SAMDB_ATTR_TYPE_INT32,                \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_QUERYABLE                   \
    },                                        \
    {                                         \
        SAM_DB_DIR_ATTR_CREATED_TIME,         \
        SAM_DB_COL_CREATED_TIME,              \
        SAMDB_ATTR_TYPE_DATETIME,             \
        SAM_DB_IS_NOT_A_ROW_ID,               \
        SAM_DB_IS_NOT_MULTI_VALUED,           \
        SAM_DB_IS_QUERYABLE                   \
    }

typedef struct _SAMDB_ATTRIBUTE_MAP_INFO
{
    wchar16_t wszAttributeName[SAM_DB_DIR_ATTR_NAME_MAX_LEN];
    DWORD     dwAttributeFlags;

} SAMDB_ATTRIBUTE_MAP_INFO, *PSAMDB_ATTRIBUTE_MAP_INFO;

#define SAMDB_TOP_ATTRIBUTE_MAP                                   \
    {                                                             \
        SAM_DB_DIR_ATTR_RECORD_ID,                                \
        (SAM_DB_ATTR_FLAGS_MANDATORY |                            \
         SAM_DB_ATTR_FLAGS_READONLY  |                            \
         SAM_DB_ATTR_FLAGS_GENERATED_BY_DB)                       \
    },                                                            \
    {                                                             \
        SAM_DB_DIR_ATTR_OBJECT_SID,                               \
        (SAM_DB_ATTR_FLAGS_MANDATORY |                            \
         SAM_DB_ATTR_FLAGS_READONLY  |                            \
         SAM_DB_ATTR_FLAGS_GENERATE_IF_NOT_SPECIFIED)             \
    },                                                            \
    {                                                             \
        SAM_DB_DIR_ATTR_DISTINGUISHED_NAME,                       \
        (SAM_DB_ATTR_FLAGS_MANDATORY |                            \
         SAM_DB_ATTR_FLAGS_READONLY  |                            \
         SAM_DB_ATTR_FLAGS_GENERATE_ALWAYS |                      \
         SAM_DB_ATTR_FLAGS_DERIVATIVE)                            \
    },                                                            \
    {                                                             \
        SAM_DB_DIR_ATTR_PARENT_DN,                                \
        (SAM_DB_ATTR_FLAGS_MANDATORY |                            \
         SAM_DB_ATTR_FLAGS_READONLY  |                            \
         SAM_DB_ATTR_FLAGS_GENERATE_ALWAYS |                      \
         SAM_DB_ATTR_FLAGS_DERIVATIVE)                            \
    },                                                            \
    {                                                             \
        SAM_DB_DIR_ATTR_OBJECT_CLASS,                             \
        (SAM_DB_ATTR_FLAGS_MANDATORY | SAM_DB_ATTR_FLAGS_READONLY)\
    },                                                            \
    {                                                             \
        SAM_DB_DIR_ATTR_DOMAIN,                                   \
        (SAM_DB_ATTR_FLAGS_MANDATORY |                            \
         SAM_DB_ATTR_FLAGS_READONLY  |                            \
         SAM_DB_ATTR_FLAGS_GENERATE_ALWAYS |                      \
         SAM_DB_ATTR_FLAGS_DERIVATIVE)                            \
    },                                                            \
    {                                                             \
        SAM_DB_DIR_ATTR_CREATED_TIME,                             \
        (SAM_DB_ATTR_FLAGS_MANDATORY |                            \
         SAM_DB_ATTR_FLAGS_READONLY  |                            \
         SAM_DB_ATTR_FLAGS_GENERATED_BY_DB)                       \
    }

#define SAMDB_USER_ATTRIBUTE_MAP                                 \
    SAMDB_TOP_ATTRIBUTE_MAP,                                     \
    {                                                            \
        SAM_DB_DIR_ATTR_SAM_ACCOUNT_NAME,                        \
        SAM_DB_ATTR_FLAGS_MANDATORY | SAM_DB_ATTR_FLAGS_READONLY \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_USER_PRINCIPAL_NAME,                     \
        SAM_DB_ATTR_FLAGS_NONE                                   \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_COMMON_NAME,                             \
        SAM_DB_ATTR_FLAGS_MANDATORY | SAM_DB_ATTR_FLAGS_READONLY \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_UID,                                     \
        (SAM_DB_ATTR_FLAGS_MANDATORY |                           \
         SAM_DB_ATTR_FLAGS_READONLY  |                           \
         SAM_DB_ATTR_FLAGS_GENERATE_IF_NOT_SPECIFIED)            \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_DESCRIPTION,                             \
        SAM_DB_ATTR_FLAGS_NONE                                   \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_PASSWORD,                                \
        SAM_DB_ATTR_FLAGS_NONE                                   \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_ACCOUNT_FLAGS,                           \
        SAM_DB_ATTR_FLAGS_NONE                                   \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_GECOS,                                   \
        SAM_DB_ATTR_FLAGS_NONE                                   \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_HOME_DIR,                                \
        SAM_DB_ATTR_FLAGS_NONE                                   \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_SHELL,                                   \
        SAM_DB_ATTR_FLAGS_NONE                                   \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_PASSWORD_CHANGE_TIME,                    \
        SAM_DB_ATTR_FLAGS_NONE                                   \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_FULL_NAME,                               \
        SAM_DB_ATTR_FLAGS_NONE                                   \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_ACCOUNT_EXPIRY,                          \
        SAM_DB_ATTR_FLAGS_NONE                                   \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_LM_HASH,                                 \
        SAM_DB_ATTR_FLAGS_NONE                                   \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_NT_HASH,                                 \
        SAM_DB_ATTR_FLAGS_NONE                                   \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_PRIMARY_GROUP,                           \
        (SAM_DB_ATTR_FLAGS_MANDATORY |                           \
         SAM_DB_ATTR_FLAGS_GENERATE_IF_NOT_SPECIFIED)            \
    }

#define SAMDB_CONTAINER_ATTRIBUTE_MAP                            \
    SAMDB_TOP_ATTRIBUTE_MAP,                                     \
    {                                                            \
        SAM_DB_DIR_ATTR_DOMAIN,                                  \
        (SAM_DB_ATTR_FLAGS_MANDATORY |                           \
         SAM_DB_ATTR_FLAGS_READONLY |                            \
         SAM_DB_ATTR_FLAGS_GENERATE_IF_NOT_SPECIFIED)            \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_COMMON_NAME,                             \
        SAM_DB_ATTR_FLAGS_MANDATORY | SAM_DB_ATTR_FLAGS_READONLY \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_SAM_ACCOUNT_NAME,                        \
        SAM_DB_ATTR_FLAGS_MANDATORY | SAM_DB_ATTR_FLAGS_READONLY \
    }

#define SAMDB_GROUP_ATTRIBUTE_MAP                                \
    SAMDB_TOP_ATTRIBUTE_MAP,                                     \
    {                                                            \
        SAM_DB_DIR_ATTR_SAM_ACCOUNT_NAME,                        \
        SAM_DB_ATTR_FLAGS_MANDATORY | SAM_DB_ATTR_FLAGS_READONLY \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_COMMON_NAME,                             \
        SAM_DB_ATTR_FLAGS_MANDATORY | SAM_DB_ATTR_FLAGS_READONLY \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_DESCRIPTION,                             \
        SAM_DB_ATTR_FLAGS_NONE                                   \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_PASSWORD,                                \
        SAM_DB_ATTR_FLAGS_NONE                                   \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_GID,                                     \
        (SAM_DB_ATTR_FLAGS_MANDATORY |                           \
         SAM_DB_ATTR_FLAGS_READONLY  |                           \
         SAM_DB_ATTR_FLAGS_GENERATE_IF_NOT_SPECIFIED)            \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_FULL_NAME,                               \
        SAM_DB_ATTR_FLAGS_NONE                                   \
    }

#define SAMDB_DOMAIN_ATTRIBUTE_MAP                               \
    SAMDB_TOP_ATTRIBUTE_MAP,                                     \
    {                                                            \
        SAM_DB_DIR_ATTR_NETBIOS_NAME,                            \
        SAM_DB_ATTR_FLAGS_MANDATORY | SAM_DB_ATTR_FLAGS_READONLY \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_COMMON_NAME,                             \
        SAM_DB_ATTR_FLAGS_MANDATORY | SAM_DB_ATTR_FLAGS_READONLY \
    },                                                           \
    {                                                            \
        SAM_DB_DIR_ATTR_SAM_ACCOUNT_NAME,                        \
        SAM_DB_ATTR_FLAGS_MANDATORY | SAM_DB_ATTR_FLAGS_READONLY \
    }


#endif /* __SAM_DB_TABLE_H__ */


/*
local variables:
mode: c
c-basic-offset: 4
indent-tabs-mode: nil
tab-width: 4
end:
*/
