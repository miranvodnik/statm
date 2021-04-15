/************************************************************************
**  source       * solidodbc3.h
**  directory    *
**  description  * Solid ODBC driver include
**               *
**               * Copyright UNICOM Systems, Inc. 1993, 2015.
\*************************************************************************/

#ifndef SOLIDODBC3_H
#define SOLIDODBC3_H

/* The standard trick for C++ */
#if defined(__cplusplus)
extern "C" {
#endif

/* Newer compilers might not define WIN32 but SQL headers on Windows need it. */
#if defined(_WIN32) && !defined(WIN32)
# define WIN32
#endif

/***************************************************************
 *
 * Solid extension defines
 *
 */
#define SQL_ATTR_SET_CONNECTION_DEAD    0x04BA  /* Solid extension attribute */
#define SQL_ATTR_TF_LEVEL               0x0525  /* Solid extension attribute */
#define SQL_ATTR_TC_PRIMARY             0x0526  /* Solid extension attribute */
#define SQL_ATTR_TF_PRIMARY             SQL_ATTR_TC_PRIMARY
#define SQL_ATTR_TC_SECONDARY           0x0527  /* Solid extension attribute */
#define SQL_ATTR_TF_SECONDARY           SQL_ATTR_TC_SECONDARY
#define SQL_ATTR_TF_WAITING             0x0528  /* Solid extension attribute */
#define SQL_ATTR_PA_LEVEL               0x0529  /* Solid extension attribute */
#define SQL_ATTR_TC_WORKLOAD_CONNECTION 0x052A  /* Solid extension attribute */

/* Extension attributes for millisecond-resolution timeouts. */
#define SQL_ATTR_LOGIN_TIMEOUT_MS       0x052B  /* Solid extension attribute */
#define SQL_ATTR_CONNECTION_TIMEOUT_MS  0x052C  /* Solid extension attribute */
#define SQL_ATTR_QUERY_TIMEOUT_MS       0x052D  /* Solid extension attribute */

#define SQL_ATTR_HANDLE_VALIDATION      0x052E

#define SQL_ATTR_IDLE_TIMEOUT           0x052F

#define SQL_ATTR_PASSTHROUGH_READ       0x0530
#define SQL_ATTR_PASSTHROUGH_WRITE      0x0531

#define SQL_ATTR_LC_CTYPE               0x0532

#define SQL_ATTR_TF_RECONNECT_TIMEOUT   0x0533
#define SQL_ATTR_TF_WAIT_TIMEOUT        0x0534

#define SQL_C_UTF8                      0x07CF


/* if ODBCVER is not defined, assume version 3.52 */
#if !defined(ODBCVER)
# define ODBCVER  0x0352
#endif /* ODBCVER */

/* Environment specific definitions */
#if !defined(EXPORT)
# define EXPORT
#endif

#if defined(WIN32)
# define SQL_API  __stdcall
#else
# define SQL_API
#endif


#if defined(SS_CRX)
# include <runetype_f.h>
#elif SS_FBX==5
# include <wchar.h>
# define _WCHAR_T_DEFINED 1
# include <runetype.h>
#elif defined(SS_VXW)
# define wchar_t unsigned short
#elif defined(SS_OSE)
# include <stddef.h>
# define _WCHAR_T_DEFINED 1
/* # define wchar_t int */
#elif defined(SS_BSI)
# include <rune.h>
#elif defined(SS_E32)
#else
/* The most common UNIX-like systems are covered here */
# include <wchar.h>
# if !defined(_WCHAR_T_DEFINED)
#  define _WCHAR_T_DEFINED 1
# endif
#endif

#ifdef _WCHAR_T_DEFINED
#define SQLWCHAR wchar_t
#else
#define SQLWCHAR unsigned short
#endif

#if defined(_WIN64)
#define SQLLEN          INT64
#define SQLULEN         UINT64
#else
/* for others, long & unsigned long are correct types to represent length */
#define SQLLEN          long
#define SQLULEN         unsigned long
#endif

#define SQLROWCOUNT     SQLULEN
#define SQLROWSETSIZE   SQLULEN
#define SQLTRANSID      SQLULEN
#define SQLROWOFFSET    SQLLEN
#define SQLINTEGER      int
#define SQLUINTEGER     unsigned int

#define SQLSMALLINT     short
#define SQLUSMALLINT    unsigned short
#define SQLRETURN       short
#define RETCODE         signed short
#define SQLCHAR         unsigned char
#define SQLSCHAR        signed char
#define SQLNUMERIC      unsigned char
#define SQLREAL         float
#define SQLDOUBLE       double
#define SQLDECIMAL      unsigned char
#define SQLFLOAT        double
#define SQLTIME         unsigned char
#define SQLDATE         unsigned char
#define SQLTIMESTAMP    unsigned char
#define SQLVARCHAR      char

#if !defined(WIN32) && !defined(_WIN64)

#define SQLHWND         void*

#ifndef WCHAR
#define WCHAR wchar_t
#endif

#ifndef LPWSTR
#define LPWSTR WCHAR *
#endif

#ifndef VOID
#define VOID void
#endif

#ifndef BOOL
#define BOOL unsigned char
#endif

#ifndef WORD
#define WORD short int
#endif

#ifndef DWORD
#define DWORD int
#endif

#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef CHAR
#define CHAR unsigned char
#endif

#ifndef LPSTR
#define LPSTR char*
#endif

#ifndef UCHAR
#define UCHAR unsigned char
#endif

#ifndef GUID
#define GUID unsigned long
#endif

#ifndef SQLGUID
#define SQLGUID unsigned long
#endif

#define SDWORD          int
#define SLONG           signed int
#define ULONG           unsigned int
#define SWORD           short int
#define SSHORT          signed short
#define UDWORD          unsigned int
#define UWORD           unsigned short int

#define SFLOAT          float
#define SDOUBLE         double
#define LDOUBLE         double
#else /* then WIN32 || _WIN64 */
typedef unsigned char       UCHAR;
typedef signed char         SCHAR;
typedef long int            SDWORD;
typedef short int           SWORD;
typedef unsigned int        UDWORD;
typedef unsigned short int  UWORD;
typedef float               SFLOAT;
typedef double              SDOUBLE;
typedef double              LDOUBLE;

typedef HWND                SQLHWND;
#if defined(GUID_DEFINED)
typedef GUID SQLGUID;
#else
typedef struct {
        SDWORD  Data1;
        SWORD   Data2;
        SWORD   Data3;
        UCHAR   Data4[ 8 ];
} SQLGUID;
#endif
#endif /* WIN32 || _WIN64 */

#if defined(_WIN64) || defined(WIN32)
# define ODBCINT64      INT64
# define UODBCINT64     UINT64
#elif defined(UNIX_64BIT)
# define ODBCINT64      long
# define UODBCINT64     unsigned long
#else
# if (SIZEOF_LONG == 8)
#  define ODBCINT64     long
#  define UODBCINT64    unsigned long
# else
#  ifdef HAVE_LONG_LONG
#   define ODBCINT64    long long
#   define UODBCINT64   unsigned long long
#  else
#   ifdef BIGENDIAN
struct solid_odbc_bigint_struct
{
        int             hiword;
        unsigned int    loword;
};
struct solid_odbc_bigint_struct_u
{
        unsigned int    hiword;
        unsigned int    loword;
};
#   else /* then little-endian */
struct solid_odbc_bigint_struct
{
        unsigned int    loword;
        int             hiword;
};
struct solid_odbc_bigint_struct_u
{
        unsigned int    loword;
        unsigned int    hiword;
};
#   endif /* endianness */
#   define ODBCINT64    struct solid_odbc_bigint_struct
#   define UODBCINT64   struct solid_odbc_bigint_struct_u
#  endif  /* HAVE_LONG_LONG */
# endif   /* SIZEOF_LONG */
#endif    /* _WIN64 || WIN32 */

#ifdef ODBCINT64
typedef ODBCINT64       SQLBIGINT;
#endif
#ifdef UODBCINT64
typedef UODBCINT64      SQLUBIGINT;
#endif

#if defined(UNIX_64BIT)
#define SQLSETPOSIROW   unsigned long
#elif defined(_WIN64)
#define SQLSETPOSIROW   UINT64
#else
#define SQLSETPOSIROW   unsigned short
#endif
typedef void*           SQLHANDLE;
typedef void*           SQLHENV;
typedef void*           SQLHDBC;
typedef void*           SQLHSTMT;

typedef void*           HENV;
typedef void*           HDBC;
typedef void*           HSTMT;
typedef void*           SQLPOINTER;
typedef void*           SQLHDESC;
typedef void*           PTR;

typedef struct date_st
{
        SQLSMALLINT   year;
        SQLUSMALLINT  month;
        SQLUSMALLINT  day;
} DATE_STRUCT;

typedef struct time_st
{
        SQLUSMALLINT  hour;
        SQLUSMALLINT  minute;
        SQLUSMALLINT  second;
} TIME_STRUCT;

typedef struct timestamp_st
{
        SQLSMALLINT   year;
        SQLUSMALLINT  month;
        SQLUSMALLINT  day;
        SQLUSMALLINT  hour;
        SQLUSMALLINT  minute;
        SQLUSMALLINT  second;
        SQLUINTEGER   fraction;
} TIMESTAMP_STRUCT;

typedef TIME_STRUCT      SQL_TIME_STRUCT;
typedef DATE_STRUCT      SQL_DATE_STRUCT;
typedef TIMESTAMP_STRUCT SQL_TIMESTAMP_STRUCT;

/* SQL-92 enum
 */
typedef enum {
        SQL_IS_YEAR             = 0x0001,
        SQL_IS_MONTH            = 0x0002,
        SQL_IS_DAY              = 0x0003,
        SQL_IS_HOUR             = 0x0004,
        SQL_IS_MINUTE           = 0x0005,
        SQL_IS_SECOND           = 0x0006,
        SQL_IS_YEAR_TO_MONTH    = 0x0007,
        SQL_IS_DAY_TO_HOUR      = 0x0008,
        SQL_IS_DAY_TO_MINUTE    = 0x0009,
        SQL_IS_DAY_TO_SECOND    = 0x000A,
        SQL_IS_HOUR_TO_MINUTE   = 0x000B,
        SQL_IS_HOUR_TO_SECOND   = 0x000C,
        SQL_IS_MINUTE_TO_SECOND = 0x000D
} SQLINTERVAL;

typedef struct {
        SQLUINTEGER   day;
        SQLUINTEGER   hour;
        SQLUINTEGER   minute;
        SQLUINTEGER   second;
        SQLUINTEGER   fraction;
} SQL_DAY_SECOND_STRUCT;


typedef struct {
        SQLUINTEGER   year;
        SQLUINTEGER   month;
} SQL_YEAR_MONTH_STRUCT;


typedef struct {
        SQLINTERVAL   interval_type;
        SQLSMALLINT   interval_sign;
        union {
                SQL_YEAR_MONTH_STRUCT  year_month;
                SQL_DAY_SECOND_STRUCT  day_second;
        } intval;
} SQL_INTERVAL_STRUCT;

#define SQL_MAX_NUMERIC_LEN     0x10
typedef struct {
        SQLCHAR       precision;
        SQLSCHAR      scale;
        SQLCHAR       sign;
        SQLCHAR       val[SQL_MAX_NUMERIC_LEN];
} SQL_NUMERIC_STRUCT;

#define DATE_STRUCT         SQL_DATE_STRUCT
#define TIME_STRUCT         SQL_TIME_STRUCT
#define TIMESTAMP_STRUCT    SQL_TIMESTAMP_STRUCT


#if defined(UNICODE)
typedef SQLWCHAR  SQLTCHAR;
#else
typedef SQLCHAR   SQLTCHAR;
#endif /* UNICODE */

/* -------- */

/* length indicator specials */
#define SQL_NULL_DATA           (-0x1)
#define SQL_DATA_AT_EXEC        (-0x2)


/* ODBC function return values */
#define SQL_INVALID_HANDLE      (-0x2)
#define SQL_ERROR               (-0x1)
#define SQL_SUCCESS             0x0
#define SQL_SUCCESS_WITH_INFO   0x1
#define SQL_STILL_EXECUTING     0x2
#define SQL_NEED_DATA           0x63
#define SQL_NO_DATA             0x64
#define SQL_NO_DATA_FOUND       SQL_NO_DATA

/* Easily test whether function return was
   SQL_SUCCESS or SQL_SUCCESS_WITH_INFO
*/
#define SQL_SUCCEEDED(rc) \
    ((unsigned short)(rc) <= (unsigned short)SQL_SUCCESS_WITH_INFO)

/* length indicator special value: null-terminated string */
#define SQL_NTS                 (-0x3)

#define SQL_MAX_MESSAGE_LENGTH  0x200

#define SQL_DATE_LEN            0xA

/* if precision is 0, otherwise add precision+1 */
#define SQL_TIME_LEN            0x8
/* same note about precision as for SQL_TIME_LEN */
#define SQL_TIMESTAMP_LEN       0x13

/* handle types */
#define SQL_HANDLE_ENV          0x1
#define SQL_HANDLE_DBC          0x2
#define SQL_HANDLE_STMT         0x3
#define SQL_HANDLE_DESC         0x4
#define SQL_HANDLE_SENV         0x5

/* environment  */
#define SQL_ATTR_OUTPUT_NTS         0x2711
#define SQL_ATTR_ODBC_VERSION       0x00C8
#define SQL_ATTR_CONNECTION_POOLING 0x00C9
#define SQL_ATTR_CP_MATCH           0x00CA

/* connection */
#define SQL_ATTR_AUTO_IPD       0x2711
#define SQL_ATTR_METADATA_ID    0x271E
#define SQL_ATTR_ANSI_APP       0x0073

#define SQL_ACCESS_MODE         0x0065
#define SQL_AUTOCOMMIT          0x0066
#define SQL_LOGIN_TIMEOUT       0x0067
#define SQL_OPT_TRACE           0x0068
#define SQL_OPT_TRACEFILE       0x0069
#define SQL_TRANSLATE_DLL       0x006A
#define SQL_TRANSLATE_OPTION    0x006B
#define SQL_TXN_ISOLATION       0x006C
#define SQL_CURRENT_QUALIFIER   0x006D
#define SQL_ODBC_CURSORS        0x006E
#define SQL_QUIET_MODE          0x006F
#define SQL_PACKET_SIZE         0x0070

#define SQL_DATABASE_NAME       0x10 /* use SQL_CURRENT_QUALIFIER */
#define SQL_FD_FETCH_PREV       SQL_FD_FETCH_PRIOR
#define SQL_FETCH_PREV          SQL_FETCH_PRIOR
#define SQL_CONCUR_TIMESTAMP    SQL_CONCUR_ROWVER
#define SQL_SCCO_OPT_TIMESTAMP  SQL_SCCO_OPT_ROWVER
#define SQL_CC_DELETE           SQL_CB_DELETE
#define SQL_CR_DELETE           SQL_CB_DELETE
#define SQL_CC_CLOSE            SQL_CB_CLOSE
#define SQL_CR_CLOSE            SQL_CB_CLOSE
#define SQL_CC_PRESERVE         SQL_CB_PRESERVE
#define SQL_CR_PRESERVE         SQL_CB_PRESERVE

#define SQL_ATTR_ACCESS_MODE            SQL_ACCESS_MODE
#define SQL_ATTR_AUTOCOMMIT             SQL_AUTOCOMMIT
#define SQL_ATTR_CONNECTION_TIMEOUT     0x0071
#define SQL_ATTR_CURRENT_CATALOG        SQL_CURRENT_QUALIFIER
#define SQL_ATTR_DISCONNECT_BEHAVIOR    0x0072
#define SQL_ATTR_ENLIST_IN_DTC          0x04B7
#define SQL_ATTR_ENLIST_IN_XA           0x04B8
#define SQL_ATTR_CONNECTION_DEAD        0x04B9
#define SQL_ATTR_LOGIN_TIMEOUT          SQL_LOGIN_TIMEOUT
#define SQL_ATTR_ODBC_CURSORS           SQL_ODBC_CURSORS
#define SQL_ATTR_PACKET_SIZE            SQL_PACKET_SIZE
#define SQL_ATTR_QUIET_MODE             SQL_QUIET_MODE
#define SQL_ATTR_TRACE                  SQL_OPT_TRACE
#define SQL_ATTR_TRACEFILE              SQL_OPT_TRACEFILE
#define SQL_ATTR_TRANSLATE_LIB          SQL_TRANSLATE_DLL
#define SQL_ATTR_TRANSLATE_OPTION       SQL_TRANSLATE_OPTION
#define SQL_ATTR_TXN_ISOLATION          SQL_TXN_ISOLATION

/* statement */
#define SQL_ATTR_APP_ROW_DESC               0x271A
#define SQL_ATTR_APP_PARAM_DESC             0x271B
#define SQL_ATTR_IMP_ROW_DESC               0x271C
#define SQL_ATTR_IMP_PARAM_DESC             0x271D
#define SQL_ATTR_CURSOR_SCROLLABLE          (-0x1)
#define SQL_ATTR_CURSOR_SENSITIVITY         (-0x2)
#define SQL_ATTR_ASYNC_ENABLE               0x4
#define SQL_ATTR_CONCURRENCY                SQL_CONCURRENCY
#define SQL_ATTR_CURSOR_TYPE                SQL_CURSOR_TYPE
#define SQL_ATTR_ENABLE_AUTO_IPD            0xF
#define SQL_ATTR_FETCH_BOOKMARK_PTR         0x10
#define SQL_ATTR_KEYSET_SIZE                SQL_KEYSET_SIZE
#define SQL_ATTR_MAX_LENGTH                 SQL_MAX_LENGTH
#define SQL_ATTR_MAX_ROWS                   SQL_MAX_ROWS
#define SQL_ATTR_NOSCAN                     SQL_NOSCAN
#define SQL_ATTR_PARAM_BIND_OFFSET_PTR      0x11
#define SQL_ATTR_PARAM_BIND_TYPE            0x12
#define SQL_ATTR_PARAM_OPERATION_PTR        0x13
#define SQL_ATTR_PARAM_STATUS_PTR           0x14
#define SQL_ATTR_PARAMS_PROCESSED_PTR       0x15
#define SQL_ATTR_PARAMSET_SIZE              0x16
#define SQL_ATTR_QUERY_TIMEOUT              SQL_QUERY_TIMEOUT
#define SQL_ATTR_RETRIEVE_DATA              SQL_RETRIEVE_DATA
#define SQL_ATTR_ROW_BIND_OFFSET_PTR        0x17
#define SQL_ATTR_ROW_BIND_TYPE              SQL_BIND_TYPE
#define SQL_ATTR_ROW_NUMBER                 SQL_ROW_NUMBER
#define SQL_ATTR_ROW_OPERATION_PTR          0x18
#define SQL_ATTR_ROW_STATUS_PTR             0x19
#define SQL_ATTR_ROWS_FETCHED_PTR           0x1A
#define SQL_ATTR_ROW_ARRAY_SIZE             0x1B
#define SQL_ATTR_SIMULATE_CURSOR            SQL_SIMULATE_CURSOR
#define SQL_ATTR_USE_BOOKMARKS              SQL_USE_BOOKMARKS

#define SQL_QUERY_TIMEOUT       0x0
#define SQL_MAX_ROWS            0x1
#define SQL_NOSCAN              0x2
#define SQL_MAX_LENGTH          0x3
#define SQL_ASYNC_ENABLE        0x4   /* SQL_ATTR_ASYNC_ENABLE */
#define SQL_BIND_TYPE           0x5
#define SQL_CURSOR_TYPE         0x6
#define SQL_CONCURRENCY         0x7
#define SQL_KEYSET_SIZE         0x8
#define SQL_ROWSET_SIZE         0x9
#define SQL_SIMULATE_CURSOR     0xA
#define SQL_RETRIEVE_DATA       0xB
#define SQL_USE_BOOKMARKS       0xC
#define SQL_GET_BOOKMARK        0xD
#define SQL_ROW_NUMBER          0xE

#define SQL_STMT_OPT_MAX        SQL_ROW_NUMBER
#define SQL_STMT_OPT_MIN        SQL_QUERY_TIMEOUT

/* SQL_ATTR_CURSOR_SCROLLABLE */
#define SQL_NONSCROLLABLE       0x0
#define SQL_SCROLLABLE          0x1

/* SQL descriptor fields */
#define SQL_DESC_COUNT                  0x03E9
#define SQL_DESC_TYPE                   0x03EA
#define SQL_DESC_LENGTH                 0x03EB
#define SQL_DESC_OCTET_LENGTH_PTR       0x03EC
#define SQL_DESC_PRECISION              0x03ED
#define SQL_DESC_SCALE                  0x03EE
#define SQL_DESC_DATETIME_INTERVAL_CODE 0x03EF
#define SQL_DESC_NULLABLE               0x03F0
#define SQL_DESC_INDICATOR_PTR          0x03F1
#define SQL_DESC_DATA_PTR               0x03F2
#define SQL_DESC_NAME                   0x03F3
#define SQL_DESC_UNNAMED                0x03F4
#define SQL_DESC_OCTET_LENGTH           0x03F5
#define SQL_DESC_ALLOC_TYPE             0x044B

/* descriptor field */
#define SQL_DESC_ARRAY_SIZE                     0x14
#define SQL_DESC_ARRAY_STATUS_PTR               0x15
#define SQL_DESC_AUTO_UNIQUE_VALUE              SQL_COLUMN_AUTO_INCREMENT
#define SQL_DESC_BASE_COLUMN_NAME               0x16
#define SQL_DESC_BASE_TABLE_NAME                0x17
#define SQL_DESC_BIND_OFFSET_PTR                0x18
#define SQL_DESC_BIND_TYPE                      0x19
#define SQL_DESC_CASE_SENSITIVE                 SQL_COLUMN_CASE_SENSITIVE
#define SQL_DESC_CATALOG_NAME                   SQL_COLUMN_QUALIFIER_NAME
#define SQL_DESC_CONCISE_TYPE                   SQL_COLUMN_TYPE
#define SQL_DESC_DATETIME_INTERVAL_PRECISION    0x1A
#define SQL_DESC_DISPLAY_SIZE                   SQL_COLUMN_DISPLAY_SIZE
#define SQL_DESC_FIXED_PREC_SCALE               SQL_COLUMN_MONEY
#define SQL_DESC_LABEL                          SQL_COLUMN_LABEL
#define SQL_DESC_LITERAL_PREFIX                 0x1B
#define SQL_DESC_LITERAL_SUFFIX                 0x1C
#define SQL_DESC_LOCAL_TYPE_NAME                0x1D
#define SQL_DESC_MAXIMUM_SCALE                  0x1E
#define SQL_DESC_MINIMUM_SCALE                  0x1F
#define SQL_DESC_NUM_PREC_RADIX                 0x20
#define SQL_DESC_PARAMETER_TYPE                 0x21
#define SQL_DESC_ROWS_PROCESSED_PTR             0x22
#define SQL_DESC_ROWVER                         0x23
#define SQL_DESC_SCHEMA_NAME                    SQL_COLUMN_OWNER_NAME
#define SQL_DESC_SEARCHABLE                     SQL_COLUMN_SEARCHABLE
#define SQL_DESC_TYPE_NAME                      SQL_COLUMN_TYPE_NAME
#define SQL_DESC_TABLE_NAME                     SQL_COLUMN_TABLE_NAME
#define SQL_DESC_UNSIGNED                       SQL_COLUMN_UNSIGNED
#define SQL_DESC_UPDATABLE                      SQL_COLUMN_UPDATABLE

/* diagnostics area fields */
#define SQL_DIAG_RETURNCODE             0x1
#define SQL_DIAG_NUMBER                 0x2
#define SQL_DIAG_ROW_COUNT              0x3
#define SQL_DIAG_SQLSTATE               0x4
#define SQL_DIAG_NATIVE                 0x5
#define SQL_DIAG_MESSAGE_TEXT           0x6
#define SQL_DIAG_DYNAMIC_FUNCTION       0x7
#define SQL_DIAG_CLASS_ORIGIN           0x8
#define SQL_DIAG_SUBCLASS_ORIGIN        0x9
#define SQL_DIAG_CONNECTION_NAME        0xA
#define SQL_DIAG_SERVER_NAME            0xB
#define SQL_DIAG_DYNAMIC_FUNCTION_CODE  0xC

#define SQL_DIAG_CURSOR_ROW_COUNT       (-0x4E1)
#define SQL_DIAG_ROW_NUMBER             (-0x4E0)
#define SQL_DIAG_COLUMN_NUMBER          (-0x4DF)

/* dynamic function codes */
#define SQL_DIAG_ALTER_DOMAIN           0x3
#define SQL_DIAG_ALTER_TABLE            0x4
#define SQL_DIAG_CALL                   0x7
#define SQL_DIAG_CREATE_ASSERTION       0x6
#define SQL_DIAG_CREATE_CHARACTER_SET   0x8
#define SQL_DIAG_CREATE_COLLATION       0xA
#define SQL_DIAG_CREATE_DOMAIN          0x17
#define SQL_DIAG_CREATE_INDEX           (-0x1)
#define SQL_DIAG_CREATE_SCHEMA          0x40
#define SQL_DIAG_CREATE_TABLE           0x4D
#define SQL_DIAG_CREATE_TRANSLATION     0x4F
#define SQL_DIAG_CREATE_VIEW            0x54
#define SQL_DIAG_DELETE_WHERE           0x13
#define SQL_DIAG_DROP_ASSERTION         0x18
#define SQL_DIAG_DROP_CHARACTER_SET     0x19
#define SQL_DIAG_DROP_COLLATION         0x1A
#define SQL_DIAG_DROP_DOMAIN            0x1B
#define SQL_DIAG_DROP_INDEX             (-0x2)
#define SQL_DIAG_DROP_SCHEMA            0x1F
#define SQL_DIAG_DROP_TABLE             0x20
#define SQL_DIAG_DROP_TRANSLATION       0x21
#define SQL_DIAG_DROP_VIEW              0x24
#define SQL_DIAG_DYNAMIC_DELETE_CURSOR  0x26
#define SQL_DIAG_DYNAMIC_UPDATE_CURSOR  0x51
#define SQL_DIAG_GRANT                  0x30
#define SQL_DIAG_INSERT                 0x32
#define SQL_DIAG_REVOKE                 0x3B
#define SQL_DIAG_SELECT_CURSOR          0x55
#define SQL_DIAG_UNKNOWN_STATEMENT      0x0
#define SQL_DIAG_UPDATE_WHERE           0x52

/* SQL data type */
#define SQL_UNKNOWN_TYPE        0x0
#define SQL_CHAR                0x1
#define SQL_NUMERIC             0x2
#define SQL_DECIMAL             0x3
#define SQL_INTEGER             0x4
#define SQL_SMALLINT            0x5
#define SQL_FLOAT               0x6
#define SQL_REAL                0x7
#define SQL_DOUBLE              0x8
#define SQL_DATETIME            0x9
#define SQL_VARCHAR             0xC
#define SQL_TYPE_DATE           0x5B
#define SQL_TYPE_TIME           0x5C
#define SQL_TYPE_TIMESTAMP      0x5D

/* cursor sensitivity */
#define SQL_UNSPECIFIED         0x0
#define SQL_INSENSITIVE         0x1
#define SQL_SENSITIVE           0x2

/* GetTypeInfo: all data types */
#define SQL_ALL_TYPES           0x0

#define SQL_DEFAULT             0x63

#define SQL_ARD_TYPE            (-0x63)

/* date/time subcodes */
#define SQL_CODE_DATE           0x1
#define SQL_CODE_TIME           0x2
#define SQL_CODE_TIMESTAMP      0x3

#define SQL_FALSE               0x0
#define SQL_TRUE                0x1

/* NULLABLE options */
#define SQL_NO_NULLS            0x0
#define SQL_NULLABLE            0x1
#define SQL_NULLABLE_UNKNOWN    0x2

/* SQLGetTypeInfo */
#define SQL_PRED_NONE           0x0
#define SQL_PRED_CHAR           0x1
#define SQL_PRED_BASIC          0x2
#define SQL_COL_PRED_CHAR       SQL_LIKE_ONLY
#define SQL_COL_PRED_BASIC      SQL_ALL_EXCEPT_LIKE
#define SQL_UNSEARCHABLE        0x0
#define SQL_LIKE_ONLY           0x1
#define SQL_ALL_EXCEPT_LIKE     0x2
#define SQL_SEARCHABLE          0x3
#define SQL_PRED_SEARCHABLE     SQL_SEARCHABLE

/* UNNAMED field values of descriptor */
#define SQL_NAMED               0x0
#define SQL_UNNAMED             0x1

/* ALLOC_TYPE field in descriptor */
#define SQL_DESC_ALLOC_AUTO     0x1
#define SQL_DESC_ALLOC_USER     0x2

/* SQLFreeStmt */
#define SQL_CLOSE               0x0
#define SQL_DROP                0x1
#define SQL_UNBIND              0x2
#define SQL_RESET_PARAMS        0x3

/* fetch orientation */
#define SQL_FETCH_NEXT          0x1
#define SQL_FETCH_FIRST         0x2
#define SQL_FETCH_LAST          0x3
#define SQL_FETCH_PRIOR         0x4
#define SQL_FETCH_ABSOLUTE      0x5
#define SQL_FETCH_RELATIVE      0x6

/* SQLEndTran */
#define SQL_COMMIT              0x0
#define SQL_ROLLBACK            0x1

/* null handles returned by SQLAllocHandle() */
#define SQL_NULL_HENV           0x0
#define SQL_NULL_HDBC           0x0
#define SQL_NULL_HSTMT          0x0
#define SQL_NULL_HDESC          0x0

#define SQL_NULL_HANDLE         0x0

/* SQLSpecialColumns */
#define SQL_SCOPE_CURROW        0x0
#define SQL_SCOPE_TRANSACTION   0x1
#define SQL_SCOPE_SESSION       0x2

#define SQL_PC_UNKNOWN          0x0
#define SQL_PC_NON_PSEUDO       0x1
#define SQL_PC_PSEUDO           0x2

/* SQLSpecialColumns */
#define SQL_ROW_IDENTIFIER      0x1

/* SQLStatistics */
#define SQL_INDEX_UNIQUE        0x0
#define SQL_INDEX_ALL           0x1

/* SQLStatistics */
#define SQL_INDEX_CLUSTERED     0x1
#define SQL_INDEX_HASHED        0x2
#define SQL_INDEX_OTHER         0x3

/* SQLGetFunctions */
#define SQL_API_SQLALLOCCONNECT         0x1
#define SQL_API_SQLALLOCENV             0x2
#define SQL_API_SQLALLOCHANDLE          0x3E9
#define SQL_API_SQLALLOCSTMT            0x3
#define SQL_API_SQLBINDCOL              0x4
#define SQL_API_SQLBINDPARAM            0x3EA
#define SQL_API_SQLCANCEL               0x5
#define SQL_API_SQLCLOSECURSOR          0x3EB
#define SQL_API_SQLCOLATTRIBUTE         0x6
#define SQL_API_SQLCOLUMNS              0x28
#define SQL_API_SQLCONNECT              0x7
#define SQL_API_SQLCOPYDESC             0x3EC
#define SQL_API_SQLDATASOURCES          0x39
#define SQL_API_SQLDESCRIBECOL          0x8
#define SQL_API_SQLDISCONNECT           0x9
#define SQL_API_SQLENDTRAN              0x3ED
#define SQL_API_SQLERROR                0xA
#define SQL_API_SQLEXECDIRECT           0xB
#define SQL_API_SQLEXECUTE              0xC
#define SQL_API_SQLFETCH                0xD
#define SQL_API_SQLFETCHSCROLL          0x3FD
#define SQL_API_SQLFREECONNECT          0xE
#define SQL_API_SQLFREEENV              0xF
#define SQL_API_SQLFREEHANDLE           0x3EE
#define SQL_API_SQLFREESTMT             0x10
#define SQL_API_SQLGETCONNECTATTR       0x3EF
#define SQL_API_SQLGETCONNECTOPTION     0x2A
#define SQL_API_SQLGETCURSORNAME        0x11
#define SQL_API_SQLGETDATA              0x2B
#define SQL_API_SQLGETDESCFIELD         0x3F0
#define SQL_API_SQLGETDESCREC           0x3F1
#define SQL_API_SQLGETDIAGFIELD         0x3F2
#define SQL_API_SQLGETDIAGREC           0x3F3
#define SQL_API_SQLGETENVATTR           0x3F4
#define SQL_API_SQLGETFUNCTIONS         0x2C
#define SQL_API_SQLGETINFO              0x2D
#define SQL_API_SQLGETSTMTATTR          0x3F6
#define SQL_API_SQLGETSTMTOPTION        0x2E
#define SQL_API_SQLGETTYPEINFO          0x2F
#define SQL_API_SQLNUMRESULTCOLS        0x12
#define SQL_API_SQLPARAMDATA            0x30
#define SQL_API_SQLPREPARE              0x13
#define SQL_API_SQLPUTDATA              0x31
#define SQL_API_SQLROWCOUNT             0x14
#define SQL_API_SQLSETCONNECTATTR       0x3F8
#define SQL_API_SQLSETCONNECTOPTION     0x32
#define SQL_API_SQLSETCURSORNAME        0x15
#define SQL_API_SQLSETDESCFIELD         0x3F9
#define SQL_API_SQLSETDESCREC           0x3FA
#define SQL_API_SQLSETENVATTR           0x3FB
#define SQL_API_SQLSETPARAM             0x16
#define SQL_API_SQLSETSTMTATTR          0x3FC
#define SQL_API_SQLSETSTMTOPTION        0x33
#define SQL_API_SQLSPECIALCOLUMNS       0x34
#define SQL_API_SQLSTATISTICS           0x35
#define SQL_API_SQLTABLES               0x36
#define SQL_API_SQLTRANSACT             0x17
#define SQL_API_SQLALLOCHANDLESTD       0x49
#define SQL_API_SQLBULKOPERATIONS       0x18
#define SQL_API_SQLBINDPARAMETER        0x48
#define SQL_API_SQLBROWSECONNECT        0x37
#define SQL_API_SQLCOLATTRIBUTES        0x6
#define SQL_API_SQLCOLUMNPRIVILEGES     0x38
#define SQL_API_SQLDESCRIBEPARAM        0x3A
#define SQL_API_SQLDRIVERCONNECT        0x29
#define SQL_API_SQLDRIVERS              0x47
#define SQL_API_SQLEXTENDEDFETCH        0x3B
#define SQL_API_SQLFOREIGNKEYS          0x3C
#define SQL_API_SQLMORERESULTS          0x3D
#define SQL_API_SQLNATIVESQL            0x3E
#define SQL_API_SQLNUMPARAMS            0x3F
#define SQL_API_SQLPARAMOPTIONS         0x40
#define SQL_API_SQLPRIMARYKEYS          0x41
#define SQL_API_SQLPROCEDURECOLUMNS     0x42
#define SQL_API_SQLPROCEDURES           0x43
#define SQL_API_SQLSETPOS               0x44
#define SQL_API_SQLSETSCROLLOPTIONS     0x45
#define SQL_API_SQLTABLEPRIVILEGES      0x46

/* SQLGetInfo */
#define SQL_MAX_DRIVER_CONNECTIONS          0x0
#define SQL_MAXIMUM_DRIVER_CONNECTIONS      SQL_MAX_DRIVER_CONNECTIONS
#define SQL_MAX_CONCURRENT_ACTIVITIES       0x1
#define SQL_MAXIMUM_CONCURRENT_ACTIVITIES   SQL_MAX_CONCURRENT_ACTIVITIES
#define SQL_DATA_SOURCE_NAME                0x2
#define SQL_FETCH_DIRECTION                 0x8
#define SQL_SERVER_NAME                     0xD
#define SQL_SEARCH_PATTERN_ESCAPE           0xE
#define SQL_DBMS_NAME                       0x11
#define SQL_DBMS_VER                        0x12
#define SQL_ACCESSIBLE_TABLES               0x13
#define SQL_ACCESSIBLE_PROCEDURES           0x14
#define SQL_CURSOR_COMMIT_BEHAVIOR          0x17
#define SQL_DATA_SOURCE_READ_ONLY           0x19
#define SQL_DEFAULT_TXN_ISOLATION           0x1A
#define SQL_IDENTIFIER_CASE                 0x1C
#define SQL_IDENTIFIER_QUOTE_CHAR           0x1D
#define SQL_MAX_COLUMN_NAME_LEN             0x1E
#define SQL_MAXIMUM_COLUMN_NAME_LENGTH      SQL_MAX_COLUMN_NAME_LEN
#define SQL_MAX_CURSOR_NAME_LEN             0x1F
#define SQL_MAXIMUM_CURSOR_NAME_LENGTH      SQL_MAX_CURSOR_NAME_LEN
#define SQL_MAX_SCHEMA_NAME_LEN             0x20
#define SQL_MAXIMUM_SCHEMA_NAME_LENGTH      SQL_MAX_SCHEMA_NAME_LEN
#define SQL_MAX_CATALOG_NAME_LEN            0x22
#define SQL_MAXIMUM_CATALOG_NAME_LENGTH     SQL_MAX_CATALOG_NAME_LEN
#define SQL_MAX_TABLE_NAME_LEN              0x23
#define SQL_SCROLL_CONCURRENCY              0x2B
#define SQL_TXN_CAPABLE                     0x2E
#define SQL_TRANSACTION_CAPABLE             SQL_TXN_CAPABLE
#define SQL_USER_NAME                       0x2F
#define SQL_TXN_ISOLATION_OPTION            0x48
#define SQL_TRANSACTION_ISOLATION_OPTION    SQL_TXN_ISOLATION_OPTION
#define SQL_INTEGRITY                       0x49
#define SQL_GETDATA_EXTENSIONS              0x51
#define SQL_NULL_COLLATION                  0x55
#define SQL_ALTER_TABLE                     0x56
#define SQL_ORDER_BY_COLUMNS_IN_SELECT      0x5A
#define SQL_SPECIAL_CHARACTERS              0x5E
#define SQL_MAX_COLUMNS_IN_GROUP_BY         0x61
#define SQL_MAXIMUM_COLUMNS_IN_GROUP_BY     SQL_MAX_COLUMNS_IN_GROUP_BY
#define SQL_MAX_COLUMNS_IN_INDEX            0x62
#define SQL_MAXIMUM_COLUMNS_IN_INDEX        SQL_MAX_COLUMNS_IN_INDEX
#define SQL_MAX_COLUMNS_IN_ORDER_BY         0x63
#define SQL_MAXIMUM_COLUMNS_IN_ORDER_BY     SQL_MAX_COLUMNS_IN_ORDER_BY
#define SQL_MAX_COLUMNS_IN_SELECT           0x64
#define SQL_MAXIMUM_COLUMNS_IN_SELECT       SQL_MAX_COLUMNS_IN_SELECT
#define SQL_MAX_COLUMNS_IN_TABLE            0x65
#define SQL_MAX_INDEX_SIZE                  0x66
#define SQL_MAXIMUM_INDEX_SIZE              SQL_MAX_INDEX_SIZE
#define SQL_MAX_ROW_SIZE                    0x68
#define SQL_MAXIMUM_ROW_SIZE                SQL_MAX_ROW_SIZE
#define SQL_MAX_STATEMENT_LEN               0x69
#define SQL_MAXIMUM_STATEMENT_LENGTH        SQL_MAX_STATEMENT_LEN
#define SQL_MAX_TABLES_IN_SELECT            0x6A
#define SQL_MAXIMUM_TABLES_IN_SELECT        SQL_MAX_TABLES_IN_SELECT
#define SQL_MAX_USER_NAME_LEN               0x6B
#define SQL_MAXIMUM_USER_NAME_LENGTH        SQL_MAX_USER_NAME_LEN
#define SQL_OJ_CAPABILITIES                 0x73
#define SQL_OUTER_JOIN_CAPABILITIES         SQL_OJ_CAPABILITIES

#define SQL_XOPEN_CLI_YEAR                  0x2710
#define SQL_CURSOR_SENSITIVITY              0x2711
#define SQL_DESCRIBE_PARAMETER              0x2712
#define SQL_CATALOG_NAME                    0x2713
#define SQL_COLLATION_SEQ                   0x2714
#define SQL_MAX_IDENTIFIER_LEN              0x2715
#define SQL_MAXIMUM_IDENTIFIER_LENGTH       SQL_MAX_IDENTIFIER_LEN

/* SQL_ALTER_TABLE bits */
#define SQL_AT_ADD_COLUMN                       0x00000001L
#define SQL_AT_DROP_COLUMN                      0x00000002L
#define SQL_AT_ADD_CONSTRAINT                   0x00000008L
#define SQL_AT_ADD_COLUMN_SINGLE                0x00000020L
#define SQL_AT_ADD_COLUMN_DEFAULT               0x00000040L
#define SQL_AT_ADD_COLUMN_COLLATION             0x00000080L
#define SQL_AT_SET_COLUMN_DEFAULT               0x00000100L
#define SQL_AT_DROP_COLUMN_DEFAULT              0x00000200L
#define SQL_AT_DROP_COLUMN_CASCADE              0x00000400L
#define SQL_AT_DROP_COLUMN_RESTRICT             0x00000800L
#define SQL_AT_ADD_TABLE_CONSTRAINT             0x00001000L
#define SQL_AT_DROP_TABLE_CONSTRAINT_CASCADE    0x00002000L
#define SQL_AT_DROP_TABLE_CONSTRAINT_RESTRICT   0x00004000L
#define SQL_AT_CONSTRAINT_NAME_DEFINITION       0x00008000L
#define SQL_AT_CONSTRAINT_INITIALLY_DEFERRED    0x00010000L
#define SQL_AT_CONSTRAINT_INITIALLY_IMMEDIATE   0x00020000L
#define SQL_AT_CONSTRAINT_DEFERRABLE            0x00040000L
#define SQL_AT_CONSTRAINT_NON_DEFERRABLE        0x00080000L

/* SQL_ASYNC_MODE */
#define SQL_AM_NONE                         0x0
#define SQL_AM_CONNECTION                   0x1
#define SQL_AM_STATEMENT                    0x2

/* SQL_CURSOR_COMMIT_BEHAVIOR */
#define SQL_CB_DELETE                       0x0
#define SQL_CB_CLOSE                        0x1
#define SQL_CB_PRESERVE                     0x2

/* SQL_FETCH_DIRECTION */
#define SQL_FD_FETCH_NEXT                   0x00000001L
#define SQL_FD_FETCH_FIRST                  0x00000002L
#define SQL_FD_FETCH_LAST                   0x00000004L
#define SQL_FD_FETCH_PRIOR                  0x00000008L
#define SQL_FD_FETCH_ABSOLUTE               0x00000010L
#define SQL_FD_FETCH_RELATIVE               0x00000020L

/* SQL_GETDATA_EXTENSIONS */
#define SQL_GD_ANY_COLUMN                   0x00000001L
#define SQL_GD_ANY_ORDER                    0x00000002L

/* SQL_IDENTIFIER_CASE */
#define SQL_IC_UPPER                        0x1
#define SQL_IC_LOWER                        0x2
#define SQL_IC_SENSITIVE                    0x3
#define SQL_IC_MIXED                        0x4

/* SQL_OJ_CAPABILITIES bits */
#define SQL_OJ_LEFT                         0x00000001L
#define SQL_OJ_RIGHT                        0x00000002L
#define SQL_OJ_FULL                         0x00000004L
#define SQL_OJ_NESTED                       0x00000008L
#define SQL_OJ_NOT_ORDERED                  0x00000010L
#define SQL_OJ_INNER                        0x00000020L
#define SQL_OJ_ALL_COMPARISON_OPS           0x00000040L

/* SQL_SCROLL_CONCURRENCY bits */
#define SQL_SCCO_READ_ONLY                  0x00000001L
#define SQL_SCCO_LOCK                       0x00000002L
#define SQL_SCCO_OPT_ROWVER                 0x00000004L
#define SQL_SCCO_OPT_VALUES                 0x00000008L

/* SQL_TXN_CAPABLE  */
#define SQL_TC_NONE                         0x0
#define SQL_TC_DML                          0x1
#define SQL_TC_ALL                          0x2
#define SQL_TC_DDL_COMMIT                   0x3
#define SQL_TC_DDL_IGNORE                   0x4

/* SQL_TXN_ISOLATION_OPTION bits */
#define SQL_TXN_READ_UNCOMMITTED            0x00000001L
#define SQL_TRANSACTION_READ_UNCOMMITTED    SQL_TXN_READ_UNCOMMITTED
#define SQL_TXN_READ_COMMITTED              0x00000002L
#define SQL_TRANSACTION_READ_COMMITTED      SQL_TXN_READ_COMMITTED
#define SQL_TXN_REPEATABLE_READ             0x00000004L
#define SQL_TRANSACTION_REPEATABLE_READ     SQL_TXN_REPEATABLE_READ
#define SQL_TXN_SERIALIZABLE                0x00000008L
#define SQL_TRANSACTION_SERIALIZABLE        SQL_TXN_SERIALIZABLE

/* SQL_NULL_COLLATION */
#define SQL_NC_HIGH                         0x0
#define SQL_NC_LOW                          0x1

/* constants */
#define SQL_SPEC_MAJOR      0x3        /* Major version of specification  */
#define SQL_SPEC_MINOR      0x34       /* Minor version of specification  */
#define SQL_SPEC_STRING     "03.52"    /* String constant for version */

/* size of SQLSTATE, excluding terminating null */
#define SQL_SQLSTATE_SIZE   0x5

/* max data source name size */
#define SQL_MAX_DSN_LENGTH  0x20

#define SQL_MAX_OPTION_STRING_LENGTH    0x100


/* SQL_ATTR_CONNECTION_POOLING */
#define SQL_CP_OFF                      0x0UL
#define SQL_CP_ONE_PER_DRIVER           0x1UL
#define SQL_CP_ONE_PER_HENV             0x2UL
#define SQL_CP_DEFAULT                  SQL_CP_OFF

/* SQL_ATTR_CP_MATCH */
#define SQL_CP_STRICT_MATCH             0x0UL
#define SQL_CP_RELAXED_MATCH            0x1UL
#define SQL_CP_MATCH_DEFAULT            SQL_CP_STRICT_MATCH

/* SQL_ATTR_ODBC_VERSION */
#define SQL_OV_ODBC2                    0x2UL
#define SQL_OV_ODBC3                    0x3UL

/* SQL_ACCESS_MODE */
#define SQL_MODE_READ_WRITE             0x0
#define SQL_MODE_READ_ONLY              0x1
#define SQL_MODE_DEFAULT                SQL_MODE_READ_WRITE

/* SQL_AUTOCOMMIT */
#define SQL_AUTOCOMMIT_OFF              0x0
#define SQL_AUTOCOMMIT_ON               0x1
#define SQL_AUTOCOMMIT_DEFAULT          SQL_AUTOCOMMIT_ON

/* SQL_LOGIN_TIMEOUT options */
#define SQL_LOGIN_TIMEOUT_DEFAULT       0xF

/* SQL_OPT_TRACE options */
#define SQL_OPT_TRACE_OFF               0x0
#define SQL_OPT_TRACE_ON                0x1
#define SQL_OPT_TRACE_DEFAULT           SQL_OPT_TRACE_OFF
#if defined(WIN32) || defined(_WIN64)
#define SQL_OPT_TRACE_FILE_DEFAULT      "\\SQL.LOG"
#else /* for others we won't assume root dir */
#define SQL_OPT_TRACE_FILE_DEFAULT      "sql.log"
#endif

/* SQL_ODBC_CURSORS */
#define SQL_CUR_USE_IF_NEEDED           0x0
#define SQL_CUR_USE_ODBC                0x1
#define SQL_CUR_USE_DRIVER              0x2
#define SQL_CUR_DEFAULT                 SQL_CUR_USE_DRIVER

/* SQL_ATTR_DISCONNECT_BEHAVIOR */
#define SQL_DB_RETURN_TO_POOL           0x0
#define SQL_DB_DISCONNECT               0x1
#define SQL_DB_DEFAULT                  SQL_DB_RETURN_TO_POOL

/* SQL_ATTR_ENLIST_IN_DTC */
#define SQL_DTC_DONE                    0x0

/* SQL_ATTR_CONNECTION_DEAD */
#define SQL_CD_TRUE                     0x1 /* dead */
#define SQL_CD_FALSE                    0x0 /* alive */

/* SQL_ATTR_ANSI_APP */
#define SQL_AA_TRUE                     0x1 /* ANSI */
#define SQL_AA_FALSE                    0x0 /* Unicode */

#define SQL_IS_POINTER                  (-0x4)
#define SQL_IS_UINTEGER                 (-0x5)
#define SQL_IS_INTEGER                  (-0x6)
#define SQL_IS_USMALLINT                (-0x7)
#define SQL_IS_SMALLINT                 (-0x8)

/* SQL_ATTR_PARAM_BIND_TYPE */
#define SQL_PARAM_BIND_BY_COLUMN        0x0
#define SQL_PARAM_BIND_TYPE_DEFAULT     SQL_PARAM_BIND_BY_COLUMN

/* SQL_QUERY_TIMEOUT */
#define SQL_QUERY_TIMEOUT_DEFAULT       0x0

/* SQL_MAX_ROWS */
#define SQL_MAX_ROWS_DEFAULT            0x0

/* SQL_NOSCAN */
#define SQL_NOSCAN_OFF                  0x0
#define SQL_NOSCAN_ON                   0x1
#define SQL_NOSCAN_DEFAULT              SQL_NOSCAN_OFF

/* SQL_MAX_LENGTH */
#define SQL_MAX_LENGTH_DEFAULT          0x0

/* SQL_ATTR_ASYNC_ENABLE */
#define SQL_ASYNC_ENABLE_OFF            0x0
#define SQL_ASYNC_ENABLE_ON             0x1
#define SQL_ASYNC_ENABLE_DEFAULT        SQL_ASYNC_ENABLE_OFF

/* SQL_BIND_TYPE  */
#define SQL_BIND_BY_COLUMN              0x0
#define SQL_BIND_TYPE_DEFAULT           SQL_BIND_BY_COLUMN

/* SQL_CONCURRENCY  */
#define SQL_CONCUR_READ_ONLY            0x1
#define SQL_CONCUR_LOCK                 0x2
#define SQL_CONCUR_ROWVER               0x3
#define SQL_CONCUR_VALUES               0x4
#define SQL_CONCUR_DEFAULT              SQL_CONCUR_READ_ONLY

/* SQL_CURSOR_TYPE */
#define SQL_CURSOR_FORWARD_ONLY         0x0
#define SQL_CURSOR_KEYSET_DRIVEN        0x1
#define SQL_CURSOR_DYNAMIC              0x2
#define SQL_CURSOR_STATIC               0x3
#define SQL_CURSOR_TYPE_DEFAULT         SQL_CURSOR_FORWARD_ONLY

/* SQL_ROWSET_SIZE */
#define SQL_ROWSET_SIZE_DEFAULT         0x1

/* SQL_KEYSET_SIZE */
#define SQL_KEYSET_SIZE_DEFAULT         0x0

/* SQL_SIMULATE_CURSOR */
#define SQL_SC_NON_UNIQUE               0x0
#define SQL_SC_TRY_UNIQUE               0x1
#define SQL_SC_UNIQUE                   0x2

/* SQL_RETRIEVE_DATA */
#define SQL_RD_OFF                      0x0
#define SQL_RD_ON                       0x1
#define SQL_RD_DEFAULT                  SQL_RD_ON

/* SQL_USE_BOOKMARKS options */
#define SQL_UB_OFF                      0x0
#define SQL_UB_ON                       0x1
#define SQL_UB_DEFAULT                  SQL_UB_OFF
#define SQL_UB_FIXED                    SQL_UB_ON
#define SQL_UB_VARIABLE                 0x2


/* SQL extended datatypes */
#define SQL_DATE                        0x9
#define SQL_INTERVAL                    0xA
#define SQL_TIME                        0xA
#define SQL_TIMESTAMP                   0xB
#define SQL_LONGVARCHAR                 (-0x1)
#define SQL_BINARY                      (-0x2)
#define SQL_VARBINARY                   (-0x3)
#define SQL_LONGVARBINARY               (-0x4)
#define SQL_BIGINT                      (-0x5)
#define SQL_TINYINT                     (-0x6)
#define SQL_BIT                         (-0x7)
#define SQL_GUID                        (-0xB)

#if (ODBCVER >= 0x0300)
/* interval code */
#define SQL_CODE_YEAR                   0x1
#define SQL_CODE_MONTH                  0x2
#define SQL_CODE_DAY                    0x3
#define SQL_CODE_HOUR                   0x4
#define SQL_CODE_MINUTE                 0x5
#define SQL_CODE_SECOND                 0x6
#define SQL_CODE_YEAR_TO_MONTH          0x7
#define SQL_CODE_DAY_TO_HOUR            0x8
#define SQL_CODE_DAY_TO_MINUTE          0x9
#define SQL_CODE_DAY_TO_SECOND          0xA
#define SQL_CODE_HOUR_TO_MINUTE         0xB
#define SQL_CODE_HOUR_TO_SECOND         0xC
#define SQL_CODE_MINUTE_TO_SECOND       0xD

#define SQL_INTERVAL_YEAR               (0x64 + SQL_CODE_YEAR)
#define SQL_INTERVAL_MONTH              (0x64 + SQL_CODE_MONTH)
#define SQL_INTERVAL_DAY                (0x64 + SQL_CODE_DAY)
#define SQL_INTERVAL_HOUR               (0x64 + SQL_CODE_HOUR)
#define SQL_INTERVAL_MINUTE             (0x64 + SQL_CODE_MINUTE)
#define SQL_INTERVAL_SECOND             (0x64 + SQL_CODE_SECOND)
#define SQL_INTERVAL_YEAR_TO_MONTH      (0x64 + SQL_CODE_YEAR_TO_MONTH)
#define SQL_INTERVAL_DAY_TO_HOUR        (0x64 + SQL_CODE_DAY_TO_HOUR)
#define SQL_INTERVAL_DAY_TO_MINUTE      (0x64 + SQL_CODE_DAY_TO_MINUTE)
#define SQL_INTERVAL_DAY_TO_SECOND      (0x64 + SQL_CODE_DAY_TO_SECOND)
#define SQL_INTERVAL_HOUR_TO_MINUTE     (0x64 + SQL_CODE_HOUR_TO_MINUTE)
#define SQL_INTERVAL_HOUR_TO_SECOND     (0x64 + SQL_CODE_HOUR_TO_SECOND)
#define SQL_INTERVAL_MINUTE_TO_SECOND   (0x64 + SQL_CODE_MINUTE_TO_SECOND)
#else /* ODBCVER >= 0x0300 */
#define SQL_INTERVAL_YEAR               (-0x50)
#define SQL_INTERVAL_MONTH              (-0x51)
#define SQL_INTERVAL_YEAR_TO_MONTH      (-0x52)
#define SQL_INTERVAL_DAY                (-0x53)
#define SQL_INTERVAL_HOUR               (-0x54)
#define SQL_INTERVAL_MINUTE             (-0x55)
#define SQL_INTERVAL_SECOND             (-0x56)
#define SQL_INTERVAL_DAY_TO_HOUR        (-0x57)
#define SQL_INTERVAL_DAY_TO_MINUTE      (-0x58)
#define SQL_INTERVAL_DAY_TO_SECOND      (-0x59)
#define SQL_INTERVAL_HOUR_TO_MINUTE     (-0x5A)
#define SQL_INTERVAL_HOUR_TO_SECOND     (-0x5B)
#define SQL_INTERVAL_MINUTE_TO_SECOND   (-0x5C)
#endif  /* ODBCVER >= 0x0300 */

#define SQL_UNICODE                     SQL_WCHAR
#define SQL_UNICODE_VARCHAR             SQL_WVARCHAR
#define SQL_UNICODE_LONGVARCHAR         SQL_WLONGVARCHAR
#define SQL_UNICODE_CHAR                SQL_WCHAR

/* C datatype */
#define SQL_C_CHAR                      SQL_CHAR
#define SQL_C_LONG                      SQL_INTEGER
#define SQL_C_SHORT                     SQL_SMALLINT
#define SQL_C_FLOAT                     SQL_REAL
#define SQL_C_DOUBLE                    SQL_DOUBLE
#define SQL_C_NUMERIC                   SQL_NUMERIC
#define SQL_C_DEFAULT                   0x63

#define SQL_SIGNED_OFFSET               (-0x14)
#define SQL_UNSIGNED_OFFSET             (-0x16)

#define SQL_C_DATE                      SQL_DATE
#define SQL_C_TIME                      SQL_TIME
#define SQL_C_TIMESTAMP                 SQL_TIMESTAMP
#define SQL_C_TYPE_DATE                 SQL_TYPE_DATE
#define SQL_C_TYPE_TIME                 SQL_TYPE_TIME
#define SQL_C_TYPE_TIMESTAMP            SQL_TYPE_TIMESTAMP
#define SQL_C_INTERVAL_YEAR             SQL_INTERVAL_YEAR
#define SQL_C_INTERVAL_MONTH            SQL_INTERVAL_MONTH
#define SQL_C_INTERVAL_DAY              SQL_INTERVAL_DAY
#define SQL_C_INTERVAL_HOUR             SQL_INTERVAL_HOUR
#define SQL_C_INTERVAL_MINUTE           SQL_INTERVAL_MINUTE
#define SQL_C_INTERVAL_SECOND           SQL_INTERVAL_SECOND
#define SQL_C_INTERVAL_YEAR_TO_MONTH    SQL_INTERVAL_YEAR_TO_MONTH
#define SQL_C_INTERVAL_DAY_TO_HOUR      SQL_INTERVAL_DAY_TO_HOUR
#define SQL_C_INTERVAL_DAY_TO_MINUTE    SQL_INTERVAL_DAY_TO_MINUTE
#define SQL_C_INTERVAL_DAY_TO_SECOND    SQL_INTERVAL_DAY_TO_SECOND
#define SQL_C_INTERVAL_HOUR_TO_MINUTE   SQL_INTERVAL_HOUR_TO_MINUTE
#define SQL_C_INTERVAL_HOUR_TO_SECOND   SQL_INTERVAL_HOUR_TO_SECOND
#define SQL_C_INTERVAL_MINUTE_TO_SECOND SQL_INTERVAL_MINUTE_TO_SECOND
#define SQL_C_BINARY                    SQL_BINARY
#define SQL_C_BIT                       SQL_BIT
#define SQL_C_SBIGINT                   (SQL_BIGINT+SQL_SIGNED_OFFSET)
#define SQL_C_UBIGINT                   (SQL_BIGINT+SQL_UNSIGNED_OFFSET)
#define SQL_C_TINYINT                   SQL_TINYINT
#define SQL_C_SLONG                     (SQL_C_LONG+SQL_SIGNED_OFFSET)
#define SQL_C_SSHORT                    (SQL_C_SHORT+SQL_SIGNED_OFFSET)
#define SQL_C_STINYINT                  (SQL_TINYINT+SQL_SIGNED_OFFSET)
#define SQL_C_ULONG                     (SQL_C_LONG+SQL_UNSIGNED_OFFSET)
#define SQL_C_USHORT                    (SQL_C_SHORT+SQL_UNSIGNED_OFFSET)
#define SQL_C_UTINYINT                  (SQL_TINYINT+SQL_UNSIGNED_OFFSET)

#if defined(_WIN64) || defined(UNIX_64BIT)
#define SQL_C_BOOKMARK                  SQL_C_UBIGINT
#else
#define SQL_C_BOOKMARK                  SQL_C_ULONG
#endif

#define SQL_C_GUID                      SQL_GUID

#define SQL_TYPE_NULL                   0x0
#define SQL_C_VARBOOKMARK               SQL_C_BINARY

/* SQL_DIAG_ROW_NUMBER, SQL_DIAG_COLUMN_NUMBER */
#define SQL_NO_ROW_NUMBER               (-0x1)
#define SQL_NO_COLUMN_NUMBER            (-0x1)
#define SQL_ROW_NUMBER_UNKNOWN          (-0x2)
#define SQL_COLUMN_NUMBER_UNKNOWN       (-0x2)

/* SQLBindParameter */
#define SQL_DEFAULT_PARAM               (-0x5)
#define SQL_IGNORE                      (-0x6)
#define SQL_COLUMN_IGNORE               SQL_IGNORE
#define SQL_LEN_DATA_AT_EXEC_OFFSET     (-0x64)
#define SQL_LEN_DATA_AT_EXEC(len)       (SQL_LEN_DATA_AT_EXEC_OFFSET - (len))

#define SQL_LEN_BINARY_ATTR_OFFSET      (-0x64)
#define SQL_LEN_BINARY_ATTR(len)        (SQL_LEN_BINARY_ATTR_OFFSET - (len))

/* used by Driver Manager when mapping SQLSetParam to SQLBindParameter */
#define SQL_PARAM_TYPE_DEFAULT          SQL_PARAM_INPUT_OUTPUT
#define SQL_SETPARAM_VALUE_MAX          (-0x1L)

/* SQLColAttributes  */
#define SQL_COLUMN_COUNT                0x0
#define SQL_COLUMN_NAME                 0x1
#define SQL_COLUMN_TYPE                 0x2
#define SQL_COLUMN_LENGTH               0x3
#define SQL_COLUMN_PRECISION            0x4
#define SQL_COLUMN_SCALE                0x5
#define SQL_COLUMN_DISPLAY_SIZE         0x6
#define SQL_COLUMN_NULLABLE             0x7
#define SQL_COLUMN_UNSIGNED             0x8
#define SQL_COLUMN_MONEY                0x9
#define SQL_COLUMN_UPDATABLE            0xA
#define SQL_COLUMN_AUTO_INCREMENT       0xB
#define SQL_COLUMN_CASE_SENSITIVE       0xC
#define SQL_COLUMN_SEARCHABLE           0xD
#define SQL_COLUMN_TYPE_NAME            0xE
#define SQL_COLUMN_TABLE_NAME           0xF
#define SQL_COLUMN_OWNER_NAME           0x10
#define SQL_COLUMN_QUALIFIER_NAME       0x11
#define SQL_COLUMN_LABEL                0x12
#define SQL_COLATT_OPT_MAX              SQL_COLUMN_LABEL

#define SQL_COLATT_OPT_MIN              SQL_COLUMN_COUNT

/* SQLColAttributes SQL_COLUMN_UPDATABLE */
#define SQL_ATTR_READONLY               0x0
#define SQL_ATTR_WRITE                  0x1
#define SQL_ATTR_READWRITE_UNKNOWN      0x2

/* SQLGetData */
#define SQL_NO_TOTAL                         (-0x4)

#define SQL_API_ALL_FUNCTIONS                0x0

#define SQL_API_LOADBYORDINAL                0xC7

#define SQL_API_ODBC3_ALL_FUNCTIONS          0x3E7
#define SQL_API_ODBC3_ALL_FUNCTIONS_SIZE     0xFA     /* array of 250 words */

#define SQL_FUNC_EXISTS(pfExists, uwAPI) \
                ((*(((unsigned short*)(pfExists)) + ((uwAPI) >> 0x4)) \
                    & (0x1 << ((uwAPI) & 0x000F))) != 0x0)

#define SQL_INFO_FIRST                       0x0
#define SQL_ACTIVE_CONNECTIONS               0x0
#define SQL_ACTIVE_STATEMENTS                0x1
#define SQL_DRIVER_HDBC                      0x3
#define SQL_DRIVER_HENV                      0x4
#define SQL_DRIVER_HSTMT                     0x5
#define SQL_DRIVER_NAME                      0x6
#define SQL_DRIVER_VER                       0x7
#define SQL_ODBC_API_CONFORMANCE             0x9
#define SQL_ODBC_VER                         0xA
#define SQL_ROW_UPDATES                      0xB
#define SQL_ODBC_SAG_CLI_CONFORMANCE         0xC
#define SQL_ODBC_SQL_CONFORMANCE             0xF
#define SQL_PROCEDURES                       0x15
#define SQL_CONCAT_NULL_BEHAVIOR             0x16
#define SQL_CURSOR_ROLLBACK_BEHAVIOR         0x18
#define SQL_EXPRESSIONS_IN_ORDERBY           0x1B
#define SQL_MAX_OWNER_NAME_LEN               0x20
#define SQL_MAX_PROCEDURE_NAME_LEN           0x21
#define SQL_MAX_QUALIFIER_NAME_LEN           0x22
#define SQL_MULT_RESULT_SETS                 0x24
#define SQL_MULTIPLE_ACTIVE_TXN              0x25
#define SQL_OUTER_JOINS                      0x26
#define SQL_OWNER_TERM                       0x27
#define SQL_PROCEDURE_TERM                   0x28
#define SQL_QUALIFIER_NAME_SEPARATOR         0x29
#define SQL_QUALIFIER_TERM                   0x2A
#define SQL_SCROLL_OPTIONS                   0x2C
#define SQL_TABLE_TERM                       0x2D
#define SQL_CONVERT_FUNCTIONS                0x30
#define SQL_NUMERIC_FUNCTIONS                0x31
#define SQL_STRING_FUNCTIONS                 0x32
#define SQL_SYSTEM_FUNCTIONS                 0x33
#define SQL_TIMEDATE_FUNCTIONS               0x34
#define SQL_CONVERT_BIGINT                   0x35
#define SQL_CONVERT_BINARY                   0x36
#define SQL_CONVERT_BIT                      0x37
#define SQL_CONVERT_CHAR                     0x38
#define SQL_CONVERT_DATE                     0x39
#define SQL_CONVERT_DECIMAL                  0x3A
#define SQL_CONVERT_DOUBLE                   0x3B
#define SQL_CONVERT_FLOAT                    0x3C
#define SQL_CONVERT_INTEGER                  0x3D
#define SQL_CONVERT_LONGVARCHAR              0x3E
#define SQL_CONVERT_NUMERIC                  0x3F
#define SQL_CONVERT_REAL                     0x40
#define SQL_CONVERT_SMALLINT                 0x41
#define SQL_CONVERT_TIME                     0x42
#define SQL_CONVERT_TIMESTAMP                0x43
#define SQL_CONVERT_TINYINT                  0x44
#define SQL_CONVERT_VARBINARY                0x45
#define SQL_CONVERT_VARCHAR                  0x46
#define SQL_CONVERT_LONGVARBINARY            0x47
#define SQL_ODBC_SQL_OPT_IEF                 0x49
#define SQL_CORRELATION_NAME                 0x4A
#define SQL_NON_NULLABLE_COLUMNS             0x4B
#define SQL_DRIVER_HLIB                      0x4C
#define SQL_DRIVER_ODBC_VER                  0x4D
#define SQL_LOCK_TYPES                       0x4E
#define SQL_POS_OPERATIONS                   0x4F
#define SQL_POSITIONED_STATEMENTS            0x50
#define SQL_BOOKMARK_PERSISTENCE             0x52
#define SQL_STATIC_SENSITIVITY               0x53
#define SQL_FILE_USAGE                       0x54
#define SQL_COLUMN_ALIAS                     0x57
#define SQL_GROUP_BY                         0x58
#define SQL_KEYWORDS                         0x59
#define SQL_OWNER_USAGE                      0x5B
#define SQL_QUALIFIER_USAGE                  0x5C
#define SQL_QUOTED_IDENTIFIER_CASE           0x5D
#define SQL_SUBQUERIES                       0x5F
#define SQL_UNION                            0x60
#define SQL_MAX_ROW_SIZE_INCLUDES_LONG       0x67
#define SQL_MAX_CHAR_LITERAL_LEN             0x6C
#define SQL_TIMEDATE_ADD_INTERVALS           0x6D
#define SQL_TIMEDATE_DIFF_INTERVALS          0x6E
#define SQL_NEED_LONG_DATA_LEN               0x6F
#define SQL_MAX_BINARY_LITERAL_LEN           0x70
#define SQL_LIKE_ESCAPE_CLAUSE               0x71
#define SQL_QUALIFIER_LOCATION               0x72

#define SQL_ACTIVE_ENVIRONMENTS              0x74
#define SQL_ALTER_DOMAIN                     0x75

#define SQL_SQL_CONFORMANCE                  0x76
#define SQL_DATETIME_LITERALS                0x77

#define SQL_ASYNC_MODE                       0x2725
#define SQL_BATCH_ROW_COUNT                  0x78
#define SQL_BATCH_SUPPORT                    0x79
#define SQL_CATALOG_LOCATION                 SQL_QUALIFIER_LOCATION
#define SQL_CATALOG_NAME_SEPARATOR           SQL_QUALIFIER_NAME_SEPARATOR
#define SQL_CATALOG_TERM                     SQL_QUALIFIER_TERM
#define SQL_CATALOG_USAGE                    SQL_QUALIFIER_USAGE
#define SQL_CONVERT_WCHAR                    0x7A
#define SQL_CONVERT_INTERVAL_DAY_TIME        0x7B
#define SQL_CONVERT_INTERVAL_YEAR_MONTH      0x7C
#define SQL_CONVERT_WLONGVARCHAR             0x7D
#define SQL_CONVERT_WVARCHAR                 0x7E
#define SQL_CREATE_ASSERTION                 0x7F
#define SQL_CREATE_CHARACTER_SET             0x80
#define SQL_CREATE_COLLATION                 0x81
#define SQL_CREATE_DOMAIN                    0x82
#define SQL_CREATE_SCHEMA                    0x83
#define SQL_CREATE_TABLE                     0x84
#define SQL_CREATE_TRANSLATION               0x85
#define SQL_CREATE_VIEW                      0x86
#define SQL_DRIVER_HDESC                     0x87
#define SQL_DROP_ASSERTION                   0x88
#define SQL_DROP_CHARACTER_SET               0x89
#define SQL_DROP_COLLATION                   0x8A
#define SQL_DROP_DOMAIN                      0x8B
#define SQL_DROP_SCHEMA                      0x8C
#define SQL_DROP_TABLE                       0x8D
#define SQL_DROP_TRANSLATION                 0x8E
#define SQL_DROP_VIEW                        0x8F
#define SQL_DYNAMIC_CURSOR_ATTRIBUTES1       0x90
#define SQL_DYNAMIC_CURSOR_ATTRIBUTES2       0x91
#define SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1  0x92
#define SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2  0x93
#define SQL_INDEX_KEYWORDS                   0x94
#define SQL_INFO_SCHEMA_VIEWS                0x95
#define SQL_KEYSET_CURSOR_ATTRIBUTES1        0x96
#define SQL_KEYSET_CURSOR_ATTRIBUTES2        0x97
#define SQL_MAX_ASYNC_CONCURRENT_STATEMENTS  0x2726
#define SQL_ODBC_INTERFACE_CONFORMANCE       0x98
#define SQL_PARAM_ARRAY_ROW_COUNTS           0x99
#define SQL_PARAM_ARRAY_SELECTS              0x9A
#define SQL_SCHEMA_TERM                      SQL_OWNER_TERM
#define SQL_SCHEMA_USAGE                     SQL_OWNER_USAGE
#define SQL_SQL92_DATETIME_FUNCTIONS         0x9B
#define SQL_SQL92_FOREIGN_KEY_DELETE_RULE    0x9C
#define SQL_SQL92_FOREIGN_KEY_UPDATE_RULE    0x9D
#define SQL_SQL92_GRANT                      0x9E
#define SQL_SQL92_NUMERIC_VALUE_FUNCTIONS    0x9F
#define SQL_SQL92_PREDICATES                 0xA0
#define SQL_SQL92_RELATIONAL_JOIN_OPERATORS  0xA1
#define SQL_SQL92_REVOKE                     0xA2
#define SQL_SQL92_ROW_VALUE_CONSTRUCTOR      0xA3
#define SQL_SQL92_STRING_FUNCTIONS           0xA4
#define SQL_SQL92_VALUE_EXPRESSIONS          0xA5
#define SQL_STANDARD_CLI_CONFORMANCE         0xA6
#define SQL_STATIC_CURSOR_ATTRIBUTES1        0xA7
#define SQL_STATIC_CURSOR_ATTRIBUTES2        0xA8
#define SQL_AGGREGATE_FUNCTIONS              0xA9
#define SQL_DDL_INDEX                        0xAA
#define SQL_DM_VER                           0xAB
#define SQL_INSERT_STATEMENT                 0xAC
#define SQL_CONVERT_GUID                     0xAD
#define SQL_UNION_STATEMENT                  SQL_UNION

#define SQL_DTC_TRANSITION_COST              0x6D6

/* SQL_CONVERT_ */
#define SQL_CVT_CHAR                        0x00000001L
#define SQL_CVT_NUMERIC                     0x00000002L
#define SQL_CVT_DECIMAL                     0x00000004L
#define SQL_CVT_INTEGER                     0x00000008L
#define SQL_CVT_SMALLINT                    0x00000010L
#define SQL_CVT_FLOAT                       0x00000020L
#define SQL_CVT_REAL                        0x00000040L
#define SQL_CVT_DOUBLE                      0x00000080L
#define SQL_CVT_VARCHAR                     0x00000100L
#define SQL_CVT_LONGVARCHAR                 0x00000200L
#define SQL_CVT_BINARY                      0x00000400L
#define SQL_CVT_VARBINARY                   0x00000800L
#define SQL_CVT_BIT                         0x00001000L
#define SQL_CVT_TINYINT                     0x00002000L
#define SQL_CVT_BIGINT                      0x00004000L
#define SQL_CVT_DATE                        0x00008000L
#define SQL_CVT_TIME                        0x00010000L
#define SQL_CVT_TIMESTAMP                   0x00020000L
#define SQL_CVT_LONGVARBINARY               0x00040000L
#define SQL_CVT_INTERVAL_YEAR_MONTH         0x00080000L
#define SQL_CVT_INTERVAL_DAY_TIME           0x00100000L
#define SQL_CVT_WCHAR                       0x00200000L
#define SQL_CVT_WLONGVARCHAR                0x00400000L
#define SQL_CVT_WVARCHAR                    0x00800000L
#define SQL_CVT_GUID                        0x01000000L

/* SQL_CONVERT_FUNCTIONS */
#define SQL_FN_CVT_CONVERT                  0x00000001L
#define SQL_FN_CVT_CAST                     0x00000002L

/* SQL_STRING_FUNCTIONS */
#define SQL_FN_STR_CONCAT                   0x00000001L
#define SQL_FN_STR_INSERT                   0x00000002L
#define SQL_FN_STR_LEFT                     0x00000004L
#define SQL_FN_STR_LTRIM                    0x00000008L
#define SQL_FN_STR_LENGTH                   0x00000010L
#define SQL_FN_STR_LOCATE                   0x00000020L
#define SQL_FN_STR_LCASE                    0x00000040L
#define SQL_FN_STR_REPEAT                   0x00000080L
#define SQL_FN_STR_REPLACE                  0x00000100L
#define SQL_FN_STR_RIGHT                    0x00000200L
#define SQL_FN_STR_RTRIM                    0x00000400L
#define SQL_FN_STR_SUBSTRING                0x00000800L
#define SQL_FN_STR_UCASE                    0x00001000L
#define SQL_FN_STR_ASCII                    0x00002000L
#define SQL_FN_STR_CHAR                     0x00004000L
#define SQL_FN_STR_DIFFERENCE               0x00008000L
#define SQL_FN_STR_LOCATE_2                 0x00010000L
#define SQL_FN_STR_SOUNDEX                  0x00020000L
#define SQL_FN_STR_SPACE                    0x00040000L
#define SQL_FN_STR_BIT_LENGTH               0x00080000L
#define SQL_FN_STR_CHAR_LENGTH              0x00100000L
#define SQL_FN_STR_CHARACTER_LENGTH         0x00200000L
#define SQL_FN_STR_OCTET_LENGTH             0x00400000L
#define SQL_FN_STR_POSITION                 0x00800000L

/* SQL_SQL92_STRING_FUNCTIONS */
#define SQL_SSF_CONVERT                     0x00000001L
#define SQL_SSF_LOWER                       0x00000002L
#define SQL_SSF_UPPER                       0x00000004L
#define SQL_SSF_SUBSTRING                   0x00000008L
#define SQL_SSF_TRANSLATE                   0x00000010L
#define SQL_SSF_TRIM_BOTH                   0x00000020L
#define SQL_SSF_TRIM_LEADING                0x00000040L
#define SQL_SSF_TRIM_TRAILING               0x00000080L

/* SQL_NUMERIC_FUNCTIONS functions */
#define SQL_FN_NUM_ABS                      0x00000001L
#define SQL_FN_NUM_ACOS                     0x00000002L
#define SQL_FN_NUM_ASIN                     0x00000004L
#define SQL_FN_NUM_ATAN                     0x00000008L
#define SQL_FN_NUM_ATAN2                    0x00000010L
#define SQL_FN_NUM_CEILING                  0x00000020L
#define SQL_FN_NUM_COS                      0x00000040L
#define SQL_FN_NUM_COT                      0x00000080L
#define SQL_FN_NUM_EXP                      0x00000100L
#define SQL_FN_NUM_FLOOR                    0x00000200L
#define SQL_FN_NUM_LOG                      0x00000400L
#define SQL_FN_NUM_MOD                      0x00000800L
#define SQL_FN_NUM_SIGN                     0x00001000L
#define SQL_FN_NUM_SIN                      0x00002000L
#define SQL_FN_NUM_SQRT                     0x00004000L
#define SQL_FN_NUM_TAN                      0x00008000L
#define SQL_FN_NUM_PI                       0x00010000L
#define SQL_FN_NUM_RAND                     0x00020000L
#define SQL_FN_NUM_DEGREES                  0x00040000L
#define SQL_FN_NUM_LOG10                    0x00080000L
#define SQL_FN_NUM_POWER                    0x00100000L
#define SQL_FN_NUM_RADIANS                  0x00200000L
#define SQL_FN_NUM_ROUND                    0x00400000L
#define SQL_FN_NUM_TRUNCATE                 0x00800000L

/* SQL_SQL92_NUMERIC_VALUE_FUNCTIONS */
#define SQL_SNVF_BIT_LENGTH                 0x00000001L
#define SQL_SNVF_CHAR_LENGTH                0x00000002L
#define SQL_SNVF_CHARACTER_LENGTH           0x00000004L
#define SQL_SNVF_EXTRACT                    0x00000008L
#define SQL_SNVF_OCTET_LENGTH               0x00000010L
#define SQL_SNVF_POSITION                   0x00000020L


/* SQL_TIMEDATE_FUNCTIONS functions */
#define SQL_FN_TD_NOW                       0x00000001L
#define SQL_FN_TD_CURDATE                   0x00000002L
#define SQL_FN_TD_DAYOFMONTH                0x00000004L
#define SQL_FN_TD_DAYOFWEEK                 0x00000008L
#define SQL_FN_TD_DAYOFYEAR                 0x00000010L
#define SQL_FN_TD_MONTH                     0x00000020L
#define SQL_FN_TD_QUARTER                   0x00000040L
#define SQL_FN_TD_WEEK                      0x00000080L
#define SQL_FN_TD_YEAR                      0x00000100L
#define SQL_FN_TD_CURTIME                   0x00000200L
#define SQL_FN_TD_HOUR                      0x00000400L
#define SQL_FN_TD_MINUTE                    0x00000800L
#define SQL_FN_TD_SECOND                    0x00001000L
#define SQL_FN_TD_TIMESTAMPADD              0x00002000L
#define SQL_FN_TD_TIMESTAMPDIFF             0x00004000L
#define SQL_FN_TD_DAYNAME                   0x00008000L
#define SQL_FN_TD_MONTHNAME                 0x00010000L
#define SQL_FN_TD_CURRENT_DATE              0x00020000L
#define SQL_FN_TD_CURRENT_TIME              0x00040000L
#define SQL_FN_TD_CURRENT_TIMESTAMP         0x00080000L
#define SQL_FN_TD_EXTRACT                   0x00100000L

/* SQL_SQL92_DATETIME_FUNCTIONS */
#define SQL_SDF_CURRENT_DATE                0x00000001L
#define SQL_SDF_CURRENT_TIME                0x00000002L
#define SQL_SDF_CURRENT_TIMESTAMP           0x00000004L

/* SQL_SYSTEM_FUNCTIONS  */
#define SQL_FN_SYS_USERNAME                 0x00000001L
#define SQL_FN_SYS_DBNAME                   0x00000002L
#define SQL_FN_SYS_IFNULL                   0x00000004L

/* SQL_TIMEDATE_ADD_INTERVALS and SQL_TIMEDATE_DIFF_INTERVALS */
#define SQL_FN_TSI_FRAC_SECOND              0x00000001L
#define SQL_FN_TSI_SECOND                   0x00000002L
#define SQL_FN_TSI_MINUTE                   0x00000004L
#define SQL_FN_TSI_HOUR                     0x00000008L
#define SQL_FN_TSI_DAY                      0x00000010L
#define SQL_FN_TSI_WEEK                     0x00000020L
#define SQL_FN_TSI_MONTH                    0x00000040L
#define SQL_FN_TSI_QUARTER                  0x00000080L
#define SQL_FN_TSI_YEAR                     0x00000100L

/* SQLFetchScroll FetchOrientation's */
#define SQL_CA1_NEXT                        0x00000001L
#define SQL_CA1_ABSOLUTE                    0x00000002L
#define SQL_CA1_RELATIVE                    0x00000004L
#define SQL_CA1_BOOKMARK                    0x00000008L

/* SQLSetPos LockType's */
#define SQL_CA1_LOCK_NO_CHANGE              0x00000040L
#define SQL_CA1_LOCK_EXCLUSIVE              0x00000080L
#define SQL_CA1_LOCK_UNLOCK                 0x00000100L

/* SQLSetPos Operations */
#define SQL_CA1_POS_POSITION                0x00000200L
#define SQL_CA1_POS_UPDATE                  0x00000400L
#define SQL_CA1_POS_DELETE                  0x00000800L
#define SQL_CA1_POS_REFRESH                 0x00001000L

/* positioned updates and deletes */
#define SQL_CA1_POSITIONED_UPDATE           0x00002000L
#define SQL_CA1_POSITIONED_DELETE           0x00004000L
#define SQL_CA1_SELECT_FOR_UPDATE           0x00008000L

/* SQLBulkOperations  */
#define SQL_CA1_BULK_ADD                    0x00010000L
#define SQL_CA1_BULK_UPDATE_BY_BOOKMARK     0x00020000L
#define SQL_CA1_BULK_DELETE_BY_BOOKMARK     0x00040000L
#define SQL_CA1_BULK_FETCH_BY_BOOKMARK      0x00080000L

/* SQL_ATTR_SCROLL_CONCURRENCY */
#define SQL_CA2_READ_ONLY_CONCURRENCY       0x00000001L
#define SQL_CA2_LOCK_CONCURRENCY            0x00000002L
#define SQL_CA2_OPT_ROWVER_CONCURRENCY      0x00000004L
#define SQL_CA2_OPT_VALUES_CONCURRENCY      0x00000008L

/* sensitivity of the cursor */
#define SQL_CA2_SENSITIVITY_ADDITIONS       0x00000010L
#define SQL_CA2_SENSITIVITY_DELETIONS       0x00000020L
#define SQL_CA2_SENSITIVITY_UPDATES         0x00000040L

/* SQL_ATTR_MAX_ROWS */
#define SQL_CA2_MAX_ROWS_SELECT             0x00000080L
#define SQL_CA2_MAX_ROWS_INSERT             0x00000100L
#define SQL_CA2_MAX_ROWS_DELETE             0x00000200L
#define SQL_CA2_MAX_ROWS_UPDATE             0x00000400L
#define SQL_CA2_MAX_ROWS_CATALOG            0x00000800L
#define SQL_CA2_MAX_ROWS_AFFECTS_ALL \
    (SQL_CA2_MAX_ROWS_SELECT |\
     SQL_CA2_MAX_ROWS_INSERT |\
     SQL_CA2_MAX_ROWS_DELETE |\
     SQL_CA2_MAX_ROWS_UPDATE |\
     SQL_CA2_MAX_ROWS_CATALOG)

/* SQL_DIAG_CURSOR_ROW_COUNT */
#define SQL_CA2_CRC_EXACT                   0x00001000L
#define SQL_CA2_CRC_APPROXIMATE             0x00002000L

/* positioned statements that can be simulated */
#define SQL_CA2_SIMULATE_NON_UNIQUE         0x00004000L
#define SQL_CA2_SIMULATE_TRY_UNIQUE         0x00008000L
#define SQL_CA2_SIMULATE_UNIQUE             0x00010000L

/* SQL_ODBC_API_CONFORMANCE */
#define SQL_OAC_NONE                        0x0
#define SQL_OAC_LEVEL1                      0x1
#define SQL_OAC_LEVEL2                      0x2

/* SQL_ODBC_SAG_CLI_CONFORMANCE */
#define SQL_OSCC_NOT_COMPLIANT              0x0
#define SQL_OSCC_COMPLIANT                  0x1

/* SQL_ODBC_SQL_CONFORMANCE  */
#define SQL_OSC_MINIMUM                     0x0
#define SQL_OSC_CORE                        0x1
#define SQL_OSC_EXTENDED                    0x2


/* SQL_CONCAT_NULL_BEHAVIOR  */
#define SQL_CB_NULL                         0x0
#define SQL_CB_NON_NULL                     0x1

/* SQL_SCROLL_OPTIONS */
#define SQL_SO_FORWARD_ONLY                 0x00000001L
#define SQL_SO_KEYSET_DRIVEN                0x00000002L
#define SQL_SO_DYNAMIC                      0x00000004L
#define SQL_SO_MIXED                        0x00000008L
#define SQL_SO_STATIC                       0x00000010L

#define SQL_FD_FETCH_BOOKMARK               0x00000080L

/* SQL_CORRELATION_NAME  */
#define SQL_CN_NONE                         0x0
#define SQL_CN_DIFFERENT                    0x1
#define SQL_CN_ANY                          0x2

/* SQL_NON_NULLABLE_COLUMNS */
#define SQL_NNC_NULL                        0x0
#define SQL_NNC_NON_NULL                    0x2

/* SQL_NULL_COLLATION */
#define SQL_NC_START                        0x2
#define SQL_NC_END                          0x4

/* SQL_FILE_USAGE */
#define SQL_FILE_NOT_SUPPORTED              0x0
#define SQL_FILE_TABLE                      0x1
#define SQL_FILE_QUALIFIER                  0x2
#define SQL_FILE_CATALOG                    SQL_FILE_QUALIFIER

/* SQL_GETDATA_EXTENSIONS */
#define SQL_GD_BLOCK                        0x4
#define SQL_GD_BOUND                        0x8

/* SQL_POSITIONED_STATEMENTS */
#define SQL_PS_POSITIONED_DELETE            0x1
#define SQL_PS_POSITIONED_UPDATE            0x2
#define SQL_PS_SELECT_FOR_UPDATE            0x4

/* SQL_GROUP_BY */

#define SQL_GB_NOT_SUPPORTED                0x0
#define SQL_GB_GROUP_BY_EQUALS_SELECT       0x1
#define SQL_GB_GROUP_BY_CONTAINS_SELECT     0x2
#define SQL_GB_NO_RELATION                  0x3
#define SQL_GB_COLLATE                      0x4

/* SQL_OWNER_USAGE */
#define SQL_OU_DML_STATEMENTS               0x00000001L
#define SQL_OU_PROCEDURE_INVOCATION         0x00000002L
#define SQL_OU_TABLE_DEFINITION             0x00000004L
#define SQL_OU_INDEX_DEFINITION             0x00000008L
#define SQL_OU_PRIVILEGE_DEFINITION         0x00000010L

/* SQL_SCHEMA_USAGE  */
#define SQL_SU_DML_STATEMENTS               SQL_OU_DML_STATEMENTS
#define SQL_SU_PROCEDURE_INVOCATION         SQL_OU_PROCEDURE_INVOCATION
#define SQL_SU_TABLE_DEFINITION             SQL_OU_TABLE_DEFINITION
#define SQL_SU_INDEX_DEFINITION             SQL_OU_INDEX_DEFINITION
#define SQL_SU_PRIVILEGE_DEFINITION         SQL_OU_PRIVILEGE_DEFINITION

/* SQL_QUALIFIER_USAGE */
#define SQL_QU_DML_STATEMENTS               0x00000001L
#define SQL_QU_PROCEDURE_INVOCATION         0x00000002L
#define SQL_QU_TABLE_DEFINITION             0x00000004L
#define SQL_QU_INDEX_DEFINITION             0x00000008L
#define SQL_QU_PRIVILEGE_DEFINITION         0x00000010L

/* SQL_CATALOG_USAGE */
#define SQL_CU_DML_STATEMENTS               SQL_QU_DML_STATEMENTS
#define SQL_CU_PROCEDURE_INVOCATION         SQL_QU_PROCEDURE_INVOCATION
#define SQL_CU_TABLE_DEFINITION             SQL_QU_TABLE_DEFINITION
#define SQL_CU_INDEX_DEFINITION             SQL_QU_INDEX_DEFINITION
#define SQL_CU_PRIVILEGE_DEFINITION         SQL_QU_PRIVILEGE_DEFINITION

/* SQL_SUBQUERIES */
#define SQL_SQ_COMPARISON                   0x00000001L
#define SQL_SQ_EXISTS                       0x00000002L
#define SQL_SQ_IN                           0x00000004L
#define SQL_SQ_QUANTIFIED                   0x00000008L
#define SQL_SQ_CORRELATED_SUBQUERIES        0x00000010L

/* SQL_UNION */
#define SQL_U_UNION                         0x00000001L
#define SQL_U_UNION_ALL                     0x00000002L

/* SQL_BOOKMARK_PERSISTENCE */
#define SQL_BP_CLOSE                        0x00000001L
#define SQL_BP_DELETE                       0x00000002L
#define SQL_BP_DROP                         0x00000004L
#define SQL_BP_TRANSACTION                  0x00000008L
#define SQL_BP_UPDATE                       0x00000010L
#define SQL_BP_OTHER_HSTMT                  0x00000020L
#define SQL_BP_SCROLL                       0x00000040L

/* SQL_STATIC_SENSITIVITY */
#define SQL_SS_ADDITIONS                    0x00000001L
#define SQL_SS_DELETIONS                    0x00000002L
#define SQL_SS_UPDATES                      0x00000004L

/* SQL_VIEW */
#define SQL_CV_CREATE_VIEW                  0x00000001L
#define SQL_CV_CHECK_OPTION                 0x00000002L
#define SQL_CV_CASCADED                     0x00000004L
#define SQL_CV_LOCAL                        0x00000008L

/* SQL_LOCK_TYPES */

#define SQL_LCK_NO_CHANGE                   0x00000001L
#define SQL_LCK_EXCLUSIVE                   0x00000002L
#define SQL_LCK_UNLOCK                      0x00000004L

/* SQL_POS_OPERATIONS */
#define SQL_POS_POSITION                    0x00000001L
#define SQL_POS_REFRESH                     0x00000002L
#define SQL_POS_UPDATE                      0x00000004L
#define SQL_POS_DELETE                      0x00000008L
#define SQL_POS_ADD                         0x00000010L

/* SQL_QUALIFIER_LOCATION */
#define SQL_QL_START                        0x0001
#define SQL_QL_END                          0x0002

/* SQL_AGGREGATE_FUNCTIONS */
#define SQL_AF_AVG                          0x00000001L
#define SQL_AF_COUNT                        0x00000002L
#define SQL_AF_MAX                          0x00000004L
#define SQL_AF_MIN                          0x00000008L
#define SQL_AF_SUM                          0x00000010L
#define SQL_AF_DISTINCT                     0x00000020L
#define SQL_AF_ALL                          0x00000040L

/* SQL_SQL_CONFORMANCE bit */
#define SQL_SC_SQL92_ENTRY                  0x00000001L
#define SQL_SC_FIPS127_2_TRANSITIONAL       0x00000002L
#define SQL_SC_SQL92_INTERMEDIATE           0x00000004L
#define SQL_SC_SQL92_FULL                   0x00000008L

/* SQL_DATETIME_LITERALS */
#define SQL_DL_SQL92_DATE                       0x00000001L
#define SQL_DL_SQL92_TIME                       0x00000002L
#define SQL_DL_SQL92_TIMESTAMP                  0x00000004L
#define SQL_DL_SQL92_INTERVAL_YEAR              0x00000008L
#define SQL_DL_SQL92_INTERVAL_MONTH             0x00000010L
#define SQL_DL_SQL92_INTERVAL_DAY               0x00000020L
#define SQL_DL_SQL92_INTERVAL_HOUR              0x00000040L
#define SQL_DL_SQL92_INTERVAL_MINUTE            0x00000080L
#define SQL_DL_SQL92_INTERVAL_SECOND            0x00000100L
#define SQL_DL_SQL92_INTERVAL_YEAR_TO_MONTH     0x00000200L
#define SQL_DL_SQL92_INTERVAL_DAY_TO_HOUR       0x00000400L
#define SQL_DL_SQL92_INTERVAL_DAY_TO_MINUTE     0x00000800L
#define SQL_DL_SQL92_INTERVAL_DAY_TO_SECOND     0x00001000L
#define SQL_DL_SQL92_INTERVAL_HOUR_TO_MINUTE    0x00002000L
#define SQL_DL_SQL92_INTERVAL_HOUR_TO_SECOND    0x00004000L
#define SQL_DL_SQL92_INTERVAL_MINUTE_TO_SECOND  0x00008000L

/* SQL_CATALOG_LOCATION */
#define SQL_CL_START                    SQL_QL_START
#define SQL_CL_END                      SQL_QL_END

/* SQL_BATCH_ROW_COUNT */
#define SQL_BRC_PROCEDURES              0x0000001
#define SQL_BRC_EXPLICIT                0x0000002
#define SQL_BRC_ROLLED_UP               0x0000004

/* SQL_BATCH_SUPPORT */
#define SQL_BS_SELECT_EXPLICIT          0x00000001L
#define SQL_BS_ROW_COUNT_EXPLICIT       0x00000002L
#define SQL_BS_SELECT_PROC              0x00000004L
#define SQL_BS_ROW_COUNT_PROC           0x00000008L

/* SQL_PARAM_ARRAY_ROW_COUNTS getinfo */
#define SQL_PARC_BATCH      0x1
#define SQL_PARC_NO_BATCH   0x2

/* SQL_PARAM_ARRAY_SELECTS */
#define SQL_PAS_BATCH       0x1
#define SQL_PAS_NO_BATCH    0x2
#define SQL_PAS_NO_SELECT   0x3

/* SQL_INDEX_KEYWORDS */
#define SQL_IK_NONE                         0x00000000L
#define SQL_IK_ASC                          0x00000001L
#define SQL_IK_DESC                         0x00000002L
#define SQL_IK_ALL                          (SQL_IK_ASC | SQL_IK_DESC)

/* SQL_INFO_SCHEMA_VIEWS */
#define SQL_ISV_ASSERTIONS                  0x00000001L
#define SQL_ISV_CHARACTER_SETS              0x00000002L
#define SQL_ISV_CHECK_CONSTRAINTS           0x00000004L
#define SQL_ISV_COLLATIONS                  0x00000008L
#define SQL_ISV_COLUMN_DOMAIN_USAGE         0x00000010L
#define SQL_ISV_COLUMN_PRIVILEGES           0x00000020L
#define SQL_ISV_COLUMNS                     0x00000040L
#define SQL_ISV_CONSTRAINT_COLUMN_USAGE     0x00000080L
#define SQL_ISV_CONSTRAINT_TABLE_USAGE      0x00000100L
#define SQL_ISV_DOMAIN_CONSTRAINTS          0x00000200L
#define SQL_ISV_DOMAINS                     0x00000400L
#define SQL_ISV_KEY_COLUMN_USAGE            0x00000800L
#define SQL_ISV_REFERENTIAL_CONSTRAINTS     0x00001000L
#define SQL_ISV_SCHEMATA                    0x00002000L
#define SQL_ISV_SQL_LANGUAGES               0x00004000L
#define SQL_ISV_TABLE_CONSTRAINTS           0x00008000L
#define SQL_ISV_TABLE_PRIVILEGES            0x00010000L
#define SQL_ISV_TABLES                      0x00020000L
#define SQL_ISV_TRANSLATIONS                0x00040000L
#define SQL_ISV_USAGE_PRIVILEGES            0x00080000L
#define SQL_ISV_VIEW_COLUMN_USAGE           0x00100000L
#define SQL_ISV_VIEW_TABLE_USAGE            0x00200000L
#define SQL_ISV_VIEWS                       0x00400000L

/* SQL_ASYNC_MODE */
#define SQL_AM_NONE         0x0
#define SQL_AM_CONNECTION   0x1
#define SQL_AM_STATEMENT    0x2

/* SQL_ALTER_DOMAIN */
#define SQL_AD_CONSTRAINT_NAME_DEFINITION           0x00000001L
#define SQL_AD_ADD_DOMAIN_CONSTRAINT                0x00000002L
#define SQL_AD_DROP_DOMAIN_CONSTRAINT               0x00000004L
#define SQL_AD_ADD_DOMAIN_DEFAULT                   0x00000008L
#define SQL_AD_DROP_DOMAIN_DEFAULT                  0x00000010L
#define SQL_AD_ADD_CONSTRAINT_INITIALLY_DEFERRED    0x00000020L
#define SQL_AD_ADD_CONSTRAINT_INITIALLY_IMMEDIATE   0x00000040L
#define SQL_AD_ADD_CONSTRAINT_DEFERRABLE            0x00000080L
#define SQL_AD_ADD_CONSTRAINT_NON_DEFERRABLE        0x00000100L


/* SQL_CREATE_SCHEMA */
#define SQL_CS_CREATE_SCHEMA                    0x00000001L
#define SQL_CS_AUTHORIZATION                    0x00000002L
#define SQL_CS_DEFAULT_CHARACTER_SET            0x00000004L

/* SQL_CREATE_TRANSLATION */
#define SQL_CTR_CREATE_TRANSLATION              0x00000001L

/* SQL_CREATE_ASSERTION */
#define SQL_CA_CREATE_ASSERTION                 0x00000001L
#define SQL_CA_CONSTRAINT_INITIALLY_DEFERRED    0x00000010L
#define SQL_CA_CONSTRAINT_INITIALLY_IMMEDIATE   0x00000020L
#define SQL_CA_CONSTRAINT_DEFERRABLE            0x00000040L
#define SQL_CA_CONSTRAINT_NON_DEFERRABLE        0x00000080L

/* SQL_CREATE_CHARACTER_SET */
#define SQL_CCS_CREATE_CHARACTER_SET            0x00000001L
#define SQL_CCS_COLLATE_CLAUSE                  0x00000002L
#define SQL_CCS_LIMITED_COLLATION               0x00000004L

/* SQL_CREATE_COLLATION */
#define SQL_CCOL_CREATE_COLLATION               0x00000001L

/* SQL_CREATE_DOMAIN */
#define SQL_CDO_CREATE_DOMAIN                   0x00000001L
#define SQL_CDO_DEFAULT                         0x00000002L
#define SQL_CDO_CONSTRAINT                      0x00000004L
#define SQL_CDO_COLLATION                       0x00000008L
#define SQL_CDO_CONSTRAINT_NAME_DEFINITION      0x00000010L
#define SQL_CDO_CONSTRAINT_INITIALLY_DEFERRED   0x00000020L
#define SQL_CDO_CONSTRAINT_INITIALLY_IMMEDIATE  0x00000040L
#define SQL_CDO_CONSTRAINT_DEFERRABLE           0x00000080L
#define SQL_CDO_CONSTRAINT_NON_DEFERRABLE       0x00000100L

/* SQL_CREATE_TABLE */
#define SQL_CT_CREATE_TABLE                     0x00000001L
#define SQL_CT_COMMIT_PRESERVE                  0x00000002L
#define SQL_CT_COMMIT_DELETE                    0x00000004L
#define SQL_CT_GLOBAL_TEMPORARY                 0x00000008L
#define SQL_CT_LOCAL_TEMPORARY                  0x00000010L
#define SQL_CT_CONSTRAINT_INITIALLY_DEFERRED    0x00000020L
#define SQL_CT_CONSTRAINT_INITIALLY_IMMEDIATE   0x00000040L
#define SQL_CT_CONSTRAINT_DEFERRABLE            0x00000080L
#define SQL_CT_CONSTRAINT_NON_DEFERRABLE        0x00000100L
#define SQL_CT_COLUMN_CONSTRAINT                0x00000200L
#define SQL_CT_COLUMN_DEFAULT                   0x00000400L
#define SQL_CT_COLUMN_COLLATION                 0x00000800L
#define SQL_CT_TABLE_CONSTRAINT                 0x00001000L
#define SQL_CT_CONSTRAINT_NAME_DEFINITION       0x00002000L

/* SQL_DDL_INDEX */
#define SQL_DI_CREATE_INDEX                     0x00000001L
#define SQL_DI_DROP_INDEX                       0x00000002L

/* SQL_DROP_COLLATION */
#define SQL_DC_DROP_COLLATION                   0x00000001L

/* SQL_DROP_DOMAIN */
#define SQL_DD_DROP_DOMAIN                      0x00000001L
#define SQL_DD_RESTRICT                         0x00000002L
#define SQL_DD_CASCADE                          0x00000004L

/* SQL_DROP_SCHEMA */
#define SQL_DS_DROP_SCHEMA                      0x00000001L
#define SQL_DS_RESTRICT                         0x00000002L
#define SQL_DS_CASCADE                          0x00000004L

/* SQL_DROP_CHARACTER_SET */
#define SQL_DCS_DROP_CHARACTER_SET              0x00000001L

/* SQL_DROP_ASSERTION */
#define SQL_DA_DROP_ASSERTION                   0x00000001L

/* SQL_DROP_TABLE */
#define SQL_DT_DROP_TABLE                       0x00000001L
#define SQL_DT_RESTRICT                         0x00000002L
#define SQL_DT_CASCADE                          0x00000004L

/* SQL_DROP_TRANSLATION */
#define SQL_DTR_DROP_TRANSLATION                0x00000001L

/* SQL_DROP_VIEW */
#define SQL_DV_DROP_VIEW                        0x00000001L
#define SQL_DV_RESTRICT                         0x00000002L
#define SQL_DV_CASCADE                          0x00000004L

/* SQL_INSERT_STATEMENT */
#define SQL_IS_INSERT_LITERALS                  0x00000001L
#define SQL_IS_INSERT_SEARCHED                  0x00000002L
#define SQL_IS_SELECT_INTO                      0x00000004L

/* SQL_ODBC_INTERFACE_CONFORMANCE */
#define SQL_OIC_CORE                            0x1UL
#define SQL_OIC_LEVEL1                          0x2UL
#define SQL_OIC_LEVEL2                          0x3UL

/* SQL_SQL92_FOREIGN_KEY_DELETE_RULE */
#define SQL_SFKD_CASCADE                        0x00000001L
#define SQL_SFKD_NO_ACTION                      0x00000002L
#define SQL_SFKD_SET_DEFAULT                    0x00000004L
#define SQL_SFKD_SET_NULL                       0x00000008L

/* SQL_SQL92_FOREIGN_KEY_UPDATE_RULE */
#define SQL_SFKU_CASCADE                        0x00000001L
#define SQL_SFKU_NO_ACTION                      0x00000002L
#define SQL_SFKU_SET_DEFAULT                    0x00000004L
#define SQL_SFKU_SET_NULL                       0x00000008L

/* SQL_SQL92_GRANT  bit*/
#define SQL_SG_USAGE_ON_DOMAIN                  0x00000001L
#define SQL_SG_USAGE_ON_CHARACTER_SET           0x00000002L
#define SQL_SG_USAGE_ON_COLLATION               0x00000004L
#define SQL_SG_USAGE_ON_TRANSLATION             0x00000008L
#define SQL_SG_WITH_GRANT_OPTION                0x00000010L
#define SQL_SG_DELETE_TABLE                     0x00000020L
#define SQL_SG_INSERT_TABLE                     0x00000040L
#define SQL_SG_INSERT_COLUMN                    0x00000080L
#define SQL_SG_REFERENCES_TABLE                 0x00000100L
#define SQL_SG_REFERENCES_COLUMN                0x00000200L
#define SQL_SG_SELECT_TABLE                     0x00000400L
#define SQL_SG_UPDATE_TABLE                     0x00000800L
#define SQL_SG_UPDATE_COLUMN                    0x00001000L

/* SQL_SQL92_PREDICATES */
#define SQL_SP_EXISTS                           0x00000001L
#define SQL_SP_ISNOTNULL                        0x00000002L
#define SQL_SP_ISNULL                           0x00000004L
#define SQL_SP_MATCH_FULL                       0x00000008L
#define SQL_SP_MATCH_PARTIAL                    0x00000010L
#define SQL_SP_MATCH_UNIQUE_FULL                0x00000020L
#define SQL_SP_MATCH_UNIQUE_PARTIAL             0x00000040L
#define SQL_SP_OVERLAPS                         0x00000080L
#define SQL_SP_UNIQUE                           0x00000100L
#define SQL_SP_LIKE                             0x00000200L
#define SQL_SP_IN                               0x00000400L
#define SQL_SP_BETWEEN                          0x00000800L
#define SQL_SP_COMPARISON                       0x00001000L
#define SQL_SP_QUANTIFIED_COMPARISON            0x00002000L

/* SQL_SQL92_RELATIONAL_JOIN_OPERATORS */
#define SQL_SRJO_CORRESPONDING_CLAUSE           0x00000001L
#define SQL_SRJO_CROSS_JOIN                     0x00000002L
#define SQL_SRJO_EXCEPT_JOIN                    0x00000004L
#define SQL_SRJO_FULL_OUTER_JOIN                0x00000008L
#define SQL_SRJO_INNER_JOIN                     0x00000010L
#define SQL_SRJO_INTERSECT_JOIN                 0x00000020L
#define SQL_SRJO_LEFT_OUTER_JOIN                0x00000040L
#define SQL_SRJO_NATURAL_JOIN                   0x00000080L
#define SQL_SRJO_RIGHT_OUTER_JOIN               0x00000100L
#define SQL_SRJO_UNION_JOIN                     0x00000200L

/* SQL_SQL92_REVOKE */
#define SQL_SR_USAGE_ON_DOMAIN                  0x00000001L
#define SQL_SR_USAGE_ON_CHARACTER_SET           0x00000002L
#define SQL_SR_USAGE_ON_COLLATION               0x00000004L
#define SQL_SR_USAGE_ON_TRANSLATION             0x00000008L
#define SQL_SR_GRANT_OPTION_FOR                 0x00000010L
#define SQL_SR_CASCADE                          0x00000020L
#define SQL_SR_RESTRICT                         0x00000040L
#define SQL_SR_DELETE_TABLE                     0x00000080L
#define SQL_SR_INSERT_TABLE                     0x00000100L
#define SQL_SR_INSERT_COLUMN                    0x00000200L
#define SQL_SR_REFERENCES_TABLE                 0x00000400L
#define SQL_SR_REFERENCES_COLUMN                0x00000800L
#define SQL_SR_SELECT_TABLE                     0x00001000L
#define SQL_SR_UPDATE_TABLE                     0x00002000L
#define SQL_SR_UPDATE_COLUMN                    0x00004000L

/* SQL_SQL92_ROW_VALUE_CONSTRUCTOR */
#define SQL_SRVC_VALUE_EXPRESSION               0x00000001L
#define SQL_SRVC_NULL                           0x00000002L
#define SQL_SRVC_DEFAULT                        0x00000004L
#define SQL_SRVC_ROW_SUBQUERY                   0x00000008L

/* SQL_SQL92_VALUE_EXPRESSIONS */
#define SQL_SVE_CASE                            0x00000001L
#define SQL_SVE_CAST                            0x00000002L
#define SQL_SVE_COALESCE                        0x00000004L
#define SQL_SVE_NULLIF                          0x00000008L

/* SQL_STANDARD_CLI_CONFORMANCE */
#define SQL_SCC_XOPEN_CLI_VERSION1              0x00000001L
#define SQL_SCC_ISO92_CLI                       0x00000002L

/* SQL_UNION_STATEMENT */
#define SQL_US_UNION                            SQL_U_UNION
#define SQL_US_UNION_ALL                        SQL_U_UNION_ALL

/* SQL_DTC_TRANSITION_COST */
#define SQL_DTC_ENLIST_EXPENSIVE        0x1
#define SQL_DTC_UNENLIST_EXPENSIVE      0x2

/* SQLDataSources  */
#define SQL_FETCH_FIRST_USER            0x1F
#define SQL_FETCH_FIRST_SYSTEM          0x20

/* SQLSetPos */
#define SQL_ENTIRE_ROWSET               0x0

/* SQLSetPos */
#define SQL_POSITION                    0x0
#define SQL_REFRESH                     0x1
#define SQL_UPDATE                      0x2
#define SQL_DELETE                      0x3

/* SQLBulkOperations */
#define SQL_ADD                         0x4
#define SQL_SETPOS_MAX_OPTION_VALUE     SQL_ADD
#define SQL_UPDATE_BY_BOOKMARK          0x5
#define SQL_DELETE_BY_BOOKMARK          0x6
#define SQL_FETCH_BY_BOOKMARK           0x7

/* SQLSetPos */
#define SQL_LOCK_NO_CHANGE              0x0
#define SQL_LOCK_EXCLUSIVE              0x1
#define SQL_LOCK_UNLOCK                 0x2

#define SQL_SETPOS_MAX_LOCK_VALUE       SQL_LOCK_UNLOCK

/* SQLSpecialColumns */
#define SQL_BEST_ROWID                  0x1
#define SQL_ROWVER                      0x2

#define SQL_PC_NOT_PSEUDO               0x1

/* SQLStatistics */
#define SQL_QUICK                       0x0
#define SQL_ENSURE                      0x1

#define SQL_TABLE_STAT                  0x0

/* SQLTables */
#define SQL_ALL_CATALOGS                "%"
#define SQL_ALL_SCHEMAS                 "%"
#define SQL_ALL_TABLE_TYPES             "%"

/* SQLDriverConnect */
#define SQL_DRIVER_NOPROMPT             0x0
#define SQL_DRIVER_COMPLETE             0x1
#define SQL_DRIVER_PROMPT               0x2
#define SQL_DRIVER_COMPLETE_REQUIRED    0x3


SQLRETURN SQL_API SQLAllocConnect(
    SQLHENV         EnvironmentHandle,
    SQLHDBC         *ConnectionHandle);

SQLRETURN SQL_API SQLAllocEnv(
    SQLHENV         *EnvironmentHandle);

SQLRETURN SQL_API SQLAllocHandle(
    SQLSMALLINT     HandleType,
    SQLHANDLE       InputHandle,
    SQLHANDLE       *OutputHandle);

SQLRETURN SQL_API SQLAllocStmt(
    SQLHDBC         ConnectionHandle,
    SQLHSTMT        *StatementHandle);

SQLRETURN SQL_API SQLBindCol(
    SQLHSTMT        StatementHandle,
    SQLUSMALLINT    ColumnNumber,
    SQLSMALLINT     TargetType,
    SQLPOINTER      TargetValue,
    SQLLEN          BufferLength,
    SQLLEN          *StrLen_or_Ind);

SQLRETURN SQL_API SQLBindParam(
    SQLHSTMT        StatementHandle,
    SQLUSMALLINT    ParameterNumber,
    SQLSMALLINT     ValueType,
    SQLSMALLINT     ParameterType,
    SQLULEN         LengthPrecision,
    SQLSMALLINT     ParameterScale,
    SQLPOINTER      ParameterValue,
    SQLLEN          *StrLen_or_Ind);

SQLRETURN SQL_API SQLCancel(
    SQLHSTMT        StatementHandle);

SQLRETURN SQL_API SQLCloseCursor(
    SQLHSTMT        StatementHandle);

#if defined(_WIN64) || defined(UNIX_64BIT)
SQLRETURN SQL_API SQLColAttribute(
    SQLHSTMT        StatementHandle,
    SQLUSMALLINT    ColumnNumber,
    SQLUSMALLINT    FieldIdentifier,
    SQLPOINTER      CharacterAttribute,
    SQLSMALLINT     BufferLength,
    SQLSMALLINT     *StringLength,
    SQLLEN          *NumericAttribute);
#else
SQLRETURN SQL_API SQLColAttribute(
    SQLHSTMT        StatementHandle,
    SQLUSMALLINT    ColumnNumber,
    SQLUSMALLINT    FieldIdentifier,
    SQLPOINTER      CharacterAttribute,
    SQLSMALLINT     BufferLength,
    SQLSMALLINT     *StringLength,
    SQLPOINTER      NumericAttribute);
#endif

SQLRETURN SQL_API SQLColumns(
    SQLHSTMT        StatementHandle,
    SQLCHAR         *CatalogName,
    SQLSMALLINT     NameLength1,
    SQLCHAR         *SchemaName,
    SQLSMALLINT     NameLength2,
    SQLCHAR         *TableName,
    SQLSMALLINT     NameLength3,
    SQLCHAR         *ColumnName,
    SQLSMALLINT     NameLength4);

SQLRETURN SQL_API SQLConnect(
    SQLHDBC         ConnectionHandle,
    SQLCHAR         *ServerName,
    SQLSMALLINT     NameLength1,
    SQLCHAR         *UserName,
    SQLSMALLINT     NameLength2,
    SQLCHAR         *Authentication,
    SQLSMALLINT     NameLength3);

SQLRETURN SQL_API SQLCopyDesc(
    SQLHDESC        SourceDescHandle,
    SQLHDESC        TargetDescHandle);

SQLRETURN SQL_API SQLDataSources(
    SQLHENV         EnvironmentHandle,
    SQLUSMALLINT    Direction,
    SQLCHAR         *ServerName,
    SQLSMALLINT     BufferLength1,
    SQLSMALLINT     *NameLength1,
    SQLCHAR         *Description,
    SQLSMALLINT     BufferLength2,
    SQLSMALLINT     *NameLength2);

SQLRETURN SQL_API SQLDescribeCol(
    SQLHSTMT        StatementHandle,
    SQLUSMALLINT    ColumnNumber,
    SQLCHAR         *ColumnName,
    SQLSMALLINT     BufferLength,
    SQLSMALLINT     *NameLength,
    SQLSMALLINT     *DataType,
    SQLULEN         *ColumnSize,
    SQLSMALLINT     *DecimalDigits,
    SQLSMALLINT     *Nullable);

SQLRETURN SQL_API SQLDisconnect(
    SQLHDBC         ConnectionHandle);

SQLRETURN SQL_API SQLEndTran(
    SQLSMALLINT     HandleType,
    SQLHANDLE       Handle,
    SQLSMALLINT     CompletionType);

SQLRETURN SQL_API SQLError(
    SQLHENV         EnvironmentHandle,
    SQLHDBC         ConnectionHandle,
    SQLHSTMT        StatementHandle,
    SQLCHAR         *Sqlstate,
    SQLINTEGER      *NativeError,
    SQLCHAR         *MessageText,
    SQLSMALLINT     BufferLength,
    SQLSMALLINT     *TextLength);

SQLRETURN SQL_API SQLExecDirect(
    SQLHSTMT        StatementHandle,
    SQLCHAR         *StatementText,
    SQLINTEGER      TextLength);

SQLRETURN SQL_API SQLExecute(
    SQLHSTMT        StatementHandle);

SQLRETURN SQL_API SQLFetch(
    SQLHSTMT        StatementHandle);

SQLRETURN SQL_API SQLFetchScroll(
    SQLHSTMT        StatementHandle,
    SQLSMALLINT     FetchOrientation,
    SQLLEN          FetchOffset);

SQLRETURN SQL_API SQLFreeConnect(
    SQLHDBC         ConnectionHandle);

SQLRETURN SQL_API SQLFreeEnv(
    SQLHENV         EnvironmentHandle);

SQLRETURN SQL_API SQLFreeHandle(
    SQLSMALLINT     HandleType,
    SQLHANDLE       Handle);

SQLRETURN SQL_API SQLFreeStmt(
    SQLHSTMT        StatementHandle,
    SQLUSMALLINT    Option);

SQLRETURN SQL_API SQLGetConnectAttr(
    SQLHDBC         ConnectionHandle,
    SQLINTEGER      Attribute,
    SQLPOINTER      Value,
    SQLINTEGER      BufferLength,
    SQLINTEGER      *StringLength);

SQLRETURN SQL_API SQLGetConnectOption(
    SQLHDBC         ConnectionHandle,
    SQLUSMALLINT    Option,
    SQLPOINTER      Value);

SQLRETURN SQL_API SQLGetCursorName(
    SQLHSTMT        StatementHandle,
    SQLCHAR         *CursorName,
    SQLSMALLINT     BufferLength,
    SQLSMALLINT     *NameLength);

SQLRETURN SQL_API SQLGetData(
    SQLHSTMT        StatementHandle,
    SQLUSMALLINT    ColumnNumber,
    SQLSMALLINT     TargetType,
    SQLPOINTER      TargetValue,
    SQLLEN          BufferLength,
    SQLLEN          *StrLen_or_Ind);

SQLRETURN SQL_API SQLGetDescField(
    SQLHDESC        DescriptorHandle,
    SQLSMALLINT     RecNumber,
    SQLSMALLINT     FieldIdentifier,
    SQLPOINTER      Value,
    SQLINTEGER      BufferLength,
    SQLINTEGER      *StringLength);

SQLRETURN SQL_API SQLGetDescRec(
    SQLHDESC        DescriptorHandle,
    SQLSMALLINT     RecNumber,
    SQLCHAR         *Name,
    SQLSMALLINT     BufferLength,
    SQLSMALLINT     *StringLength,
    SQLSMALLINT     *Type,
    SQLSMALLINT     *SubType,
    SQLLEN          *Length,
    SQLSMALLINT     *Precision,
    SQLSMALLINT     *Scale,
    SQLSMALLINT     *Nullable);

SQLRETURN SQL_API SQLGetDiagField(
    SQLSMALLINT     HandleType,
    SQLHANDLE       Handle,
    SQLSMALLINT     RecNumber,
    SQLSMALLINT     DiagIdentifier,
    SQLPOINTER      DiagInfo,
    SQLSMALLINT     BufferLength,
    SQLSMALLINT     *StringLength);

SQLRETURN SQL_API SQLGetDiagRec(
    SQLSMALLINT     HandleType,
    SQLHANDLE       Handle,
    SQLSMALLINT     RecNumber,
    SQLCHAR         *Sqlstate,
    SQLINTEGER      *NativeError,
    SQLCHAR         *MessageText,
    SQLSMALLINT     BufferLength,
    SQLSMALLINT     *TextLength);

SQLRETURN SQL_API SQLGetEnvAttr(
    SQLHENV         EnvironmentHandle,
    SQLINTEGER      Attribute,
    SQLPOINTER      Value,
    SQLINTEGER      BufferLength,
    SQLINTEGER      *StringLength);

SQLRETURN SQL_API SQLGetFunctions(
    SQLHDBC         ConnectionHandle,
    SQLUSMALLINT    FunctionId,
    SQLUSMALLINT    *Supported);

SQLRETURN SQL_API SQLGetInfo(
    SQLHDBC         ConnectionHandle,
    SQLUSMALLINT    InfoType,
    SQLPOINTER      InfoValue,
    SQLSMALLINT     BufferLength,
    SQLSMALLINT     *StringLength);

SQLRETURN SQL_API SQLGetStmtAttr(
    SQLHSTMT        StatementHandle,
    SQLINTEGER      Attribute,
    SQLPOINTER      Value,
    SQLINTEGER      BufferLength,
    SQLINTEGER      *StringLength);

SQLRETURN SQL_API SQLGetStmtOption(
    SQLHSTMT        StatementHandle,
    SQLUSMALLINT    Option,
    SQLPOINTER      Value);

SQLRETURN SQL_API SQLGetTypeInfo(
    SQLHSTMT        StatementHandle,
    SQLSMALLINT     DataType);

SQLRETURN SQL_API SQLNumResultCols(
    SQLHSTMT        StatementHandle,
    SQLSMALLINT     *ColumnCount);

SQLRETURN SQL_API SQLParamData(
    SQLHSTMT        StatementHandle,
    SQLPOINTER      *Value);

SQLRETURN SQL_API SQLPrepare(
    SQLHSTMT        StatementHandle,
    SQLCHAR         *StatementText,
    SQLINTEGER      TextLength);

SQLRETURN SQL_API SQLPutData(
    SQLHSTMT        StatementHandle,
    SQLPOINTER      Data,
    SQLLEN          StrLen_or_Ind);

SQLRETURN SQL_API SQLRowCount(
    SQLHSTMT        StatementHandle,
    SQLLEN*         RowCount);

SQLRETURN SQL_API SQLSetConnectAttr(
    SQLHDBC         ConnectionHandle,
    SQLINTEGER      Attribute,
    SQLPOINTER      Value,
    SQLINTEGER      StringLength);

SQLRETURN SQL_API SQLSetConnectOption(
    SQLHDBC         ConnectionHandle,
    SQLUSMALLINT    Option,
    SQLULEN         Value);

SQLRETURN SQL_API SQLSetCursorName(
    SQLHSTMT        StatementHandle,
    SQLCHAR         *CursorName,
    SQLSMALLINT     NameLength);

SQLRETURN SQL_API SQLSetDescField(
    SQLHDESC        DescriptorHandle,
    SQLSMALLINT     RecNumber,
    SQLSMALLINT     FieldIdentifier,
    SQLPOINTER      Value,
    SQLINTEGER      BufferLength);

SQLRETURN SQL_API SQLSetDescRec(
    SQLHDESC        DescriptorHandle,
    SQLSMALLINT     RecNumber,
    SQLSMALLINT     Type,
    SQLSMALLINT     SubType,
    SQLLEN          Length,
    SQLSMALLINT     Precision,
    SQLSMALLINT     Scale,
    SQLPOINTER      Data,
    SQLLEN          *StringLength,
    SQLLEN          *Indicator);

SQLRETURN SQL_API SQLSetEnvAttr(
    SQLHENV         EnvironmentHandle,
    SQLINTEGER      Attribute,
    SQLPOINTER      Value,
    SQLINTEGER      StringLength);

SQLRETURN SQL_API SQLSetParam(
    SQLHSTMT        StatementHandle,
    SQLUSMALLINT    ParameterNumber,
    SQLSMALLINT     ValueType,
    SQLSMALLINT     ParameterType,
    SQLULEN         LengthPrecision,
    SQLSMALLINT     ParameterScale,
    SQLPOINTER      ParameterValue,
    SQLLEN          *StrLen_or_Ind);

SQLRETURN SQL_API SQLSetStmtAttr(
    SQLHSTMT        StatementHandle,
    SQLINTEGER      Attribute,
    SQLPOINTER      Value,
    SQLINTEGER      StringLength);

SQLRETURN SQL_API SQLSetStmtOption(
    SQLHSTMT        StatementHandle,
    SQLUSMALLINT    Option,
    SQLULEN         Value);

SQLRETURN SQL_API SQLSpecialColumns(
    SQLHSTMT        StatementHandle,
    SQLUSMALLINT    IdentifierType,
    SQLCHAR         *CatalogName,
    SQLSMALLINT     NameLength1,
    SQLCHAR         *SchemaName,
    SQLSMALLINT     NameLength2,
    SQLCHAR         *TableName,
    SQLSMALLINT     NameLength3,
    SQLUSMALLINT    Scope,
    SQLUSMALLINT    Nullable);

SQLRETURN SQL_API SQLStatistics(
    SQLHSTMT        StatementHandle,
    SQLCHAR         *CatalogName,
    SQLSMALLINT     NameLength1,
    SQLCHAR         *SchemaName,
    SQLSMALLINT     NameLength2,
    SQLCHAR         *TableName,
    SQLSMALLINT     NameLength3,
    SQLUSMALLINT    Unique,
    SQLUSMALLINT    Reserved);

SQLRETURN SQL_API SQLTables(
    SQLHSTMT        StatementHandle,
    SQLCHAR         *CatalogName,
    SQLSMALLINT     NameLength1,
    SQLCHAR         *SchemaName,
    SQLSMALLINT     NameLength2,
    SQLCHAR         *TableName,
    SQLSMALLINT     NameLength3,
    SQLCHAR         *TableType,
    SQLSMALLINT     NameLength4);

SQLRETURN SQL_API SQLTransact(
    SQLHENV         EnvironmentHandle,
    SQLHDBC         ConnectionHandle,
    SQLUSMALLINT    CompletionType);

/* sql.h end */
/* sqlext.h begin */

SQLRETURN SQL_API SQLDriverConnect(
    SQLHDBC         hdbc,
    SQLHWND         hwnd,
    SQLCHAR         *szConnStrIn,
    SQLSMALLINT     cbConnStrIn,
    SQLCHAR         *szConnStrOut,
    SQLSMALLINT     cbConnStrOutMax,
    SQLSMALLINT     *pcbConnStrOut,
    SQLUSMALLINT    fDriverCompletion);

/* SQLExtendedFetch fFetchType */
#define SQL_FETCH_BOOKMARK              0x8

/* SQLExtendedFetch rgfRowStatus */
#define SQL_ROW_SUCCESS                 0x0
#define SQL_ROW_DELETED                 0x1
#define SQL_ROW_UPDATED                 0x2
#define SQL_ROW_NOROW                   0x3
#define SQL_ROW_ADDED                   0x4
#define SQL_ROW_ERROR                   0x5
#define SQL_ROW_SUCCESS_WITH_INFO       0x6
#define SQL_ROW_PROCEED                 0x0
#define SQL_ROW_IGNORE                  0x1

/* SQL_DESC_ARRAY_STATUS_PTR */
#define SQL_PARAM_SUCCESS               0x0
#define SQL_PARAM_SUCCESS_WITH_INFO     0x6
#define SQL_PARAM_ERROR                 0x5
#define SQL_PARAM_UNUSED                0x7
#define SQL_PARAM_DIAG_UNAVAILABLE      0x1

#define SQL_PARAM_PROCEED               0x0
#define SQL_PARAM_IGNORE                0x1

/* SQLForeignKeys  */
#define SQL_CASCADE                     0x0
#define SQL_RESTRICT                    0x1
#define SQL_SET_NULL                    0x2
#define SQL_NO_ACTION                   0x3
#define SQL_SET_DEFAULT                 0x4

#define SQL_INITIALLY_DEFERRED          0x5
#define SQL_INITIALLY_IMMEDIATE         0x6
#define SQL_NOT_DEFERRABLE              0x7

/* SQLBindParameter and SQLProcedureColumns */
#define SQL_PARAM_TYPE_UNKNOWN          0x0
#define SQL_PARAM_INPUT                 0x1
#define SQL_PARAM_INPUT_OUTPUT          0x2
#define SQL_RESULT_COL                  0x3
#define SQL_PARAM_OUTPUT                0x4
#define SQL_RETURN_VALUE                0x5

/* SQLProcedures */
#define SQL_PT_UNKNOWN                  0x0
#define SQL_PT_PROCEDURE                0x1
#define SQL_PT_FUNCTION                 0x2

#define SQL_ODBC_KEYWORDS \
"ABSOLUTE,ACTION,ADA,ADD,ALL,ALLOCATE,ALTER,AND,ANY,ARE,AS,"\
"ASC,ASSERTION,AT,AUTHORIZATION,AVG,"\
"BEGIN,BETWEEN,BIT,BIT_LENGTH,BOTH,BY,CASCADE,CASCADED,CASE,CAST,CATALOG,"\
"CHAR,CHAR_LENGTH,CHARACTER,CHARACTER_LENGTH,CHECK,CLOSE,COALESCE,"\
"COLLATE,COLLATION,COLUMN,COMMIT,CONNECT,CONNECTION,CONSTRAINT,"\
"CONSTRAINTS,CONTINUE,CONVERT,CORRESPONDING,COUNT,CREATE,CROSS,CURRENT,"\
"CURRENT_DATE,CURRENT_TIME,CURRENT_TIMESTAMP,CURRENT_USER,CURSOR,"\
"DATE,DAY,DEALLOCATE,DEC,DECIMAL,DECLARE,DEFAULT,DEFERRABLE,"\
"DEFERRED,DELETE,DESC,DESCRIBE,DESCRIPTOR,DIAGNOSTICS,DISCONNECT,"\
"DISTINCT,DOMAIN,DOUBLE,DROP,"\
"ELSE,END,END-EXEC,ESCAPE,EXCEPT,EXCEPTION,EXEC,EXECUTE,"\
"EXISTS,EXTERNAL,EXTRACT,"\
"FALSE,FETCH,FIRST,FLOAT,FOR,FOREIGN,FORTRAN,FOUND,FROM,FULL,"\
"GET,GLOBAL,GO,GOTO,GRANT,GROUP,HAVING,HOUR,"\
"IDENTITY,IMMEDIATE,IN,INCLUDE,INDEX,INDICATOR,INITIALLY,INNER,"\
"INPUT,INSENSITIVE,INSERT,INT,INTEGER,INTERSECT,INTERVAL,INTO,IS,ISOLATION,"\
"JOIN,KEY,LANGUAGE,LAST,LEADING,LEFT,LEVEL,LIKE,LOCAL,LOWER,"\
"MATCH,MAX,MIN,MINUTE,MODULE,MONTH,"\
"NAMES,NATIONAL,NATURAL,NCHAR,NEXT,NO,NONE,NOT,NULL,NULLIF,NUMERIC,"\
"OCTET_LENGTH,OF,ON,ONLY,OPEN,OPTION,OR,ORDER,OUTER,OUTPUT,OVERLAPS,"\
"PAD,PARTIAL,PASCAL,PLI,POSITION,PRECISION,PREPARE,PRESERVE,"\
"PRIMARY,PRIOR,PRIVILEGES,PROCEDURE,PUBLIC,"\
"READ,REAL,REFERENCES,RELATIVE,RESTRICT,REVOKE,RIGHT,ROLLBACK,ROWS"\
"SCHEMA,SCROLL,SECOND,SECTION,SELECT,SESSION,SESSION_USER,SET,SIZE,"\
"SMALLINT,SOME,SPACE,SQL,SQLCA,SQLCODE,SQLERROR,SQLSTATE,SQLWARNING,"\
"SUBSTRING,SUM,SYSTEM_USER,"\
"TABLE,TEMPORARY,THEN,TIME,TIMESTAMP,TIMEZONE_HOUR,TIMEZONE_MINUTE,"\
"TO,TRAILING,TRANSACTION,TRANSLATE,TRANSLATION,TRIM,TRUE,"\
"UNION,UNIQUE,UNKNOWN,UPDATE,UPPER,USAGE,USER,USING,"\
"VALUE,VALUES,VARCHAR,VARYING,VIEW,WHEN,WHENEVER,WHERE,WITH,WORK,WRITE,"\
"YEAR,ZONE"

SQLRETURN SQL_API SQLBrowseConnect(
    SQLHDBC         hdbc,
    SQLCHAR         *szConnStrIn,
    SQLSMALLINT     cbConnStrIn,
    SQLCHAR         *szConnStrOut,
    SQLSMALLINT     cbConnStrOutMax,
    SQLSMALLINT     *pcbConnStrOut);

SQLRETURN   SQL_API SQLBulkOperations(
    SQLHSTMT        StatementHandle,
    SQLSMALLINT     Operation);

SQLRETURN SQL_API SQLColAttributes(
    SQLHSTMT        hstmt,
    SQLUSMALLINT    icol,
    SQLUSMALLINT    fDescType,
    SQLPOINTER      rgbDesc,
    SQLSMALLINT     cbDescMax,
    SQLSMALLINT     *pcbDesc,
    SQLLEN          *pfDesc);

SQLRETURN SQL_API SQLColumnPrivileges(
    SQLHSTMT        hstmt,
    SQLCHAR         *szCatalogName,
    SQLSMALLINT     cbCatalogName,
    SQLCHAR         *szSchemaName,
    SQLSMALLINT     cbSchemaName,
    SQLCHAR         *szTableName,
    SQLSMALLINT     cbTableName,
    SQLCHAR         *szColumnName,
    SQLSMALLINT     cbColumnName);

SQLRETURN SQL_API SQLDescribeParam(
    SQLHSTMT        hstmt,
    SQLUSMALLINT    ipar,
    SQLSMALLINT     *pfSqlType,
    SQLULEN         *pcbParamDef,
    SQLSMALLINT     *pibScale,
    SQLSMALLINT     *pfNullable);

SQLRETURN SQL_API SQLExtendedFetch(
    SQLHSTMT        hstmt,
    SQLUSMALLINT    fFetchType,
    SQLLEN          irow,
    SQLULEN         *pcrow,
    SQLUSMALLINT    *rgfRowStatus);

SQLRETURN SQL_API SQLForeignKeys(
    SQLHSTMT        hstmt,
    SQLCHAR         *szPkCatalogName,
    SQLSMALLINT     cbPkCatalogName,
    SQLCHAR         *szPkSchemaName,
    SQLSMALLINT     cbPkSchemaName,
    SQLCHAR         *szPkTableName,
    SQLSMALLINT     cbPkTableName,
    SQLCHAR         *szFkCatalogName,
    SQLSMALLINT     cbFkCatalogName,
    SQLCHAR         *szFkSchemaName,
    SQLSMALLINT     cbFkSchemaName,
    SQLCHAR         *szFkTableName,
    SQLSMALLINT     cbFkTableName);

SQLRETURN SQL_API SQLMoreResults(
    SQLHSTMT        hstmt);

SQLRETURN SQL_API SQLNativeSql(
    SQLHDBC         hdbc,
    SQLCHAR         *szSqlStrIn,
    SQLINTEGER      cbSqlStrIn,
    SQLCHAR         *szSqlStr,
    SQLINTEGER      cbSqlStrMax,
    SQLINTEGER      *pcbSqlStr);

SQLRETURN SQL_API SQLNumParams(
    SQLHSTMT        hstmt,
    SQLSMALLINT     *pcpar);

SQLRETURN SQL_API SQLParamOptions(
    SQLHSTMT        hstmt,
    SQLULEN         crow,
    SQLULEN         *pirow);

SQLRETURN SQL_API SQLPrimaryKeys(
    SQLHSTMT        hstmt,
    SQLCHAR         *szCatalogName,
    SQLSMALLINT     cbCatalogName,
    SQLCHAR         *szSchemaName,
    SQLSMALLINT     cbSchemaName,
    SQLCHAR         *szTableName,
    SQLSMALLINT     cbTableName);

SQLRETURN SQL_API SQLProcedureColumns(
    SQLHSTMT        hstmt,
    SQLCHAR         *szCatalogName,
    SQLSMALLINT     cbCatalogName,
    SQLCHAR         *szSchemaName,
    SQLSMALLINT     cbSchemaName,
    SQLCHAR         *szProcName,
    SQLSMALLINT     cbProcName,
    SQLCHAR         *szColumnName,
    SQLSMALLINT     cbColumnName);

SQLRETURN SQL_API SQLProcedures(
    SQLHSTMT        hstmt,
    SQLCHAR         *szCatalogName,
    SQLSMALLINT     cbCatalogName,
    SQLCHAR         *szSchemaName,
    SQLSMALLINT     cbSchemaName,
    SQLCHAR         *szProcName,
    SQLSMALLINT     cbProcName);

SQLRETURN SQL_API SQLSetPos(
    SQLHSTMT        hstmt,
    SQLSETPOSIROW   irow,
    SQLUSMALLINT    fOption,
    SQLUSMALLINT    fLock);

SQLRETURN SQL_API SQLTablePrivileges(
    SQLHSTMT        hstmt,
    SQLCHAR         *szCatalogName,
    SQLSMALLINT     cbCatalogName,
    SQLCHAR         *szSchemaName,
    SQLSMALLINT     cbSchemaName,
    SQLCHAR         *szTableName,
    SQLSMALLINT      cbTableName);

SQLRETURN SQL_API SQLDrivers(
    SQLHENV         henv,
    SQLUSMALLINT    fDirection,
    SQLCHAR         *szDriverDesc,
    SQLSMALLINT     cbDriverDescMax,
    SQLSMALLINT     *pcbDriverDesc,
    SQLCHAR         *szDriverAttributes,
    SQLSMALLINT     cbDrvrAttrMax,
    SQLSMALLINT     *pcbDrvrAttr);

SQLRETURN SQL_API SQLBindParameter(
    SQLHSTMT        hstmt,
    SQLUSMALLINT    ipar,
    SQLSMALLINT     fParamType,
    SQLSMALLINT     fCType,
    SQLSMALLINT     fSqlType,
    SQLULEN         cbColDef,
    SQLSMALLINT     ibScale,
    SQLPOINTER      rgbValue,
    SQLLEN          cbValueMax,
    SQLLEN          *pcbValue);


/* sqlext.h end */

/* Unicode definitions */

#define SQL_WCHAR           (-0x8)
#define SQL_WVARCHAR        (-0x9)
#define SQL_WLONGVARCHAR    (-0xA)
#define SQL_C_WCHAR         SQL_WCHAR

#if defined(UNICODE)
#define SQL_C_TCHAR         SQL_C_WCHAR
#else
#define SQL_C_TCHAR         SQL_C_CHAR
#endif /* UNICODE */

/* size of wide char SQLSTATE, not including null termination */
#define SQL_SQLSTATE_SIZEW  (0x5 * sizeof(SQLWCHAR))

/* Unicode */
#if defined(_WIN64) || defined(UNIX_64BIT)
SQLRETURN SQL_API SQLColAttributeW(
    SQLHSTMT        hstmt,
    SQLUSMALLINT    iCol,
    SQLUSMALLINT    iField,
    SQLPOINTER      pCharAttr,
    SQLSMALLINT     cbCharAttrMax,
    SQLSMALLINT     *pcbCharAttr,
    SQLLEN          *pNumAttr);
#else /* _WIN64 || UNIX_64BIT */
SQLRETURN SQL_API SQLColAttributeW(
    SQLHSTMT        hstmt,
    SQLUSMALLINT    iCol,
    SQLUSMALLINT    iField,
    SQLPOINTER      pCharAttr,
    SQLSMALLINT     cbCharAttrMax,
    SQLSMALLINT     *pcbCharAttr,
    SQLPOINTER      pNumAttr);
#endif /* _WIN64 || UNIX_64BIT */

SQLRETURN SQL_API SQLColAttributesW(
    SQLHSTMT        hstmt,
    SQLUSMALLINT    icol,
    SQLUSMALLINT    fDescType,
    SQLPOINTER      rgbDesc,
    SQLSMALLINT     cbDescMax,
    SQLSMALLINT     *pcbDesc,
    SQLLEN          *pfDesc);

SQLRETURN SQL_API SQLConnectW(
    SQLHDBC         hdbc,
    SQLWCHAR        *szDSN,
    SQLSMALLINT     cbDSN,
    SQLWCHAR        *szUID,
    SQLSMALLINT     cbUID,
    SQLWCHAR        *szAuthStr,
    SQLSMALLINT     cbAuthStr);

SQLRETURN SQL_API SQLDescribeColW(
    SQLHSTMT        hstmt,
    SQLUSMALLINT    icol,
    SQLWCHAR        *szColName,
    SQLSMALLINT     cbColNameMax,
    SQLSMALLINT     *pcbColName,
    SQLSMALLINT     *pfSqlType,
    SQLULEN         *pcbColDef,
    SQLSMALLINT     *pibScale,
    SQLSMALLINT     *pfNullable);

SQLRETURN SQL_API SQLErrorW(
    SQLHENV         henv,
    SQLHDBC         hdbc,
    SQLHSTMT        hstmt,
    SQLWCHAR        *szSqlState,
    SQLINTEGER      *pfNativeError,
    SQLWCHAR        *szErrorMsg,
    SQLSMALLINT     cbErrorMsgMax,
    SQLSMALLINT     *pcbErrorMsg);

SQLRETURN SQL_API SQLExecDirectW(
    SQLHSTMT        hstmt,
    SQLWCHAR        *szSqlStr,
    SQLINTEGER      cbSqlStr);

SQLRETURN SQL_API SQLGetConnectAttrW(
    SQLHDBC         hdbc,
    SQLINTEGER      fAttribute,
    SQLPOINTER      rgbValue,
    SQLINTEGER      cbValueMax,
    SQLINTEGER      *pcbValue);

SQLRETURN SQL_API SQLGetCursorNameW(
    SQLHSTMT        hstmt,
    SQLWCHAR        *szCursor,
    SQLSMALLINT     cbCursorMax,
    SQLSMALLINT     *pcbCursor);

SQLRETURN SQL_API SQLSetDescFieldW(
    SQLHDESC        DescriptorHandle,
    SQLSMALLINT     RecNumber,
    SQLSMALLINT     FieldIdentifier,
    SQLPOINTER      Value,
    SQLINTEGER      BufferLength);

SQLRETURN SQL_API SQLGetDescFieldW(
    SQLHDESC        hdesc,
    SQLSMALLINT     iRecord,
    SQLSMALLINT     iField,
    SQLPOINTER      rgbValue,
    SQLINTEGER      cbValueMax,
    SQLINTEGER      *pcbValue);

SQLRETURN SQL_API SQLGetDescRecW(
    SQLHDESC        hdesc,
    SQLSMALLINT     iRecord,
    SQLWCHAR        *szName,
    SQLSMALLINT     cbNameMax,
    SQLSMALLINT     *pcbName,
    SQLSMALLINT     *pfType,
    SQLSMALLINT     *pfSubType,
    SQLLEN          *pLength,
    SQLSMALLINT     *pPrecision,
    SQLSMALLINT     *pScale,
    SQLSMALLINT     *pNullable);

SQLRETURN SQL_API SQLGetDiagFieldW(
    SQLSMALLINT     fHandleType,
    SQLHANDLE       handle,
    SQLSMALLINT     iRecord,
    SQLSMALLINT     fDiagField,
    SQLPOINTER      rgbDiagInfo,
    SQLSMALLINT     cbDiagInfoMax,
    SQLSMALLINT     *pcbDiagInfo);

SQLRETURN SQL_API SQLGetDiagRecW(
    SQLSMALLINT     fHandleType,
    SQLHANDLE       handle,
    SQLSMALLINT     iRecord,
    SQLWCHAR        *szSqlState,
    SQLINTEGER      *pfNativeError,
    SQLWCHAR        *szErrorMsg,
    SQLSMALLINT     cbErrorMsgMax,
    SQLSMALLINT     *pcbErrorMsg);

SQLRETURN SQL_API SQLPrepareW(
    SQLHSTMT        hstmt,
    SQLWCHAR        *szSqlStr,
    SQLINTEGER      cbSqlStr);

SQLRETURN SQL_API SQLSetConnectAttrW(
    SQLHDBC         hdbc,
    SQLINTEGER      fAttribute,
    SQLPOINTER      rgbValue,
    SQLINTEGER      cbValue);

SQLRETURN SQL_API SQLSetCursorNameW(
    SQLHSTMT        hstmt,
    SQLWCHAR        *szCursor,
    SQLSMALLINT     cbCursor);

SQLRETURN SQL_API SQLColumnsW(
    SQLHSTMT        hstmt,
    SQLWCHAR        *szCatalogName,
    SQLSMALLINT     cbCatalogName,
    SQLWCHAR        *szSchemaName,
    SQLSMALLINT     cbSchemaName,
    SQLWCHAR        *szTableName,
    SQLSMALLINT     cbTableName,
    SQLWCHAR        *szColumnName,
    SQLSMALLINT     cbColumnName);

SQLRETURN SQL_API SQLGetConnectOptionW(
    SQLHDBC         hdbc,
    SQLUSMALLINT    fOption,
    SQLPOINTER      pvParam);

SQLRETURN SQL_API SQLGetInfoW(
    SQLHDBC         hdbc,
    SQLUSMALLINT    fInfoType,
    SQLPOINTER      rgbInfoValue,
    SQLSMALLINT     cbInfoValueMax,
    SQLSMALLINT     *pcbInfoValue);

SQLRETURN SQL_API SQLGetTypeInfoW(
    SQLHSTMT        StatementHandle,
    SQLSMALLINT     DataType);

SQLRETURN SQL_API SQLSetConnectOptionW(
    SQLHDBC         hdbc,
    SQLUSMALLINT    fOption,
    SQLULEN         vParam);

SQLRETURN SQL_API SQLSpecialColumnsW(
    SQLHSTMT        hstmt,
    SQLUSMALLINT    fColType,
    SQLWCHAR        *szCatalogName,
    SQLSMALLINT     cbCatalogName,
    SQLWCHAR        *szSchemaName,
    SQLSMALLINT     cbSchemaName,
    SQLWCHAR        *szTableName,
    SQLSMALLINT     cbTableName,
    SQLUSMALLINT    fScope,
    SQLUSMALLINT    fNullable);

SQLRETURN SQL_API SQLStatisticsW(
    SQLHSTMT        hstmt,
    SQLWCHAR        *szCatalogName,
    SQLSMALLINT     cbCatalogName,
    SQLWCHAR        *szSchemaName,
    SQLSMALLINT     cbSchemaName,
    SQLWCHAR        *szTableName,
    SQLSMALLINT     cbTableName,
    SQLUSMALLINT    fUnique,
    SQLUSMALLINT    fAccuracy);

SQLRETURN SQL_API SQLTablesW(
    SQLHSTMT        hstmt,
    SQLWCHAR        *szCatalogName,
    SQLSMALLINT     cbCatalogName,
    SQLWCHAR        *szSchemaName,
    SQLSMALLINT     cbSchemaName,
    SQLWCHAR        *szTableName,
    SQLSMALLINT     cbTableName,
    SQLWCHAR        *szTableType,
    SQLSMALLINT     cbTableType);

SQLRETURN SQL_API SQLDataSourcesW(
    SQLHENV         henv,
    SQLUSMALLINT    fDirection,
    SQLWCHAR        *szDSN,
    SQLSMALLINT     cbDSNMax,
    SQLSMALLINT     *pcbDSN,
    SQLWCHAR        *szDescription,
    SQLSMALLINT     cbDescriptionMax,
    SQLSMALLINT     *pcbDescription);

SQLRETURN SQL_API SQLDriverConnectW(
    SQLHDBC         hdbc,
    SQLHWND         hwnd,
    SQLWCHAR        *szConnStrIn,
    SQLSMALLINT     cbConnStrIn,
    SQLWCHAR        *szConnStrOut,
    SQLSMALLINT     cbConnStrOutMax,
    SQLSMALLINT     *pcbConnStrOut,
    SQLUSMALLINT    fDriverCompletion);

SQLRETURN SQL_API SQLBrowseConnectW(
    SQLHDBC         hdbc,
    SQLWCHAR        *szConnStrIn,
    SQLSMALLINT     cbConnStrIn,
    SQLWCHAR        *szConnStrOut,
    SQLSMALLINT     cbConnStrOutMax,
    SQLSMALLINT     *pcbConnStrOut);

SQLRETURN SQL_API SQLColumnPrivilegesW(
    SQLHSTMT        hstmt,
    SQLWCHAR        *szCatalogName,
    SQLSMALLINT     cbCatalogName,
    SQLWCHAR        *szSchemaName,
    SQLSMALLINT     cbSchemaName,
    SQLWCHAR        *szTableName,
    SQLSMALLINT     cbTableName,
    SQLWCHAR        *szColumnName,
    SQLSMALLINT     cbColumnName);

SQLRETURN SQL_API SQLGetStmtAttrW(
    SQLHSTMT        hstmt,
    SQLINTEGER      fAttribute,
    SQLPOINTER      rgbValue,
    SQLINTEGER      cbValueMax,
    SQLINTEGER      *pcbValue);

SQLRETURN SQL_API SQLSetStmtAttrW(
    SQLHSTMT        hstmt,
    SQLINTEGER      fAttribute,
    SQLPOINTER      rgbValue,
    SQLINTEGER      cbValueMax);

SQLRETURN SQL_API SQLForeignKeysW(
    SQLHSTMT        hstmt,
    SQLWCHAR        *szPkCatalogName,
    SQLSMALLINT     cbPkCatalogName,
    SQLWCHAR        *szPkSchemaName,
    SQLSMALLINT     cbPkSchemaName,
    SQLWCHAR        *szPkTableName,
    SQLSMALLINT     cbPkTableName,
    SQLWCHAR        *szFkCatalogName,
    SQLSMALLINT     cbFkCatalogName,
    SQLWCHAR        *szFkSchemaName,
    SQLSMALLINT     cbFkSchemaName,
    SQLWCHAR        *szFkTableName,
    SQLSMALLINT     cbFkTableName);

SQLRETURN SQL_API SQLNativeSqlW(
    SQLHDBC         hdbc,
    SQLWCHAR        *szSqlStrIn,
    SQLINTEGER      cbSqlStrIn,
    SQLWCHAR        *szSqlStr,
    SQLINTEGER      cbSqlStrMax,
    SQLINTEGER      *pcbSqlStr);

SQLRETURN SQL_API SQLPrimaryKeysW(
    SQLHSTMT        hstmt,
    SQLWCHAR        *szCatalogName,
    SQLSMALLINT     cbCatalogName,
    SQLWCHAR        *szSchemaName,
    SQLSMALLINT     cbSchemaName,
    SQLWCHAR        *szTableName,
    SQLSMALLINT     cbTableName);

SQLRETURN SQL_API SQLProcedureColumnsW(
    SQLHSTMT        hstmt,
    SQLWCHAR        *szCatalogName,
    SQLSMALLINT     cbCatalogName,
    SQLWCHAR        *szSchemaName,
    SQLSMALLINT     cbSchemaName,
    SQLWCHAR        *szProcName,
    SQLSMALLINT     cbProcName,
    SQLWCHAR        *szColumnName,
    SQLSMALLINT     cbColumnName);

SQLRETURN SQL_API SQLProceduresW(
    SQLHSTMT        hstmt,
    SQLWCHAR        *szCatalogName,
    SQLSMALLINT     cbCatalogName,
    SQLWCHAR        *szSchemaName,
    SQLSMALLINT     cbSchemaName,
    SQLWCHAR        *szProcName,
    SQLSMALLINT     cbProcName);

SQLRETURN SQL_API SQLTablePrivilegesW(
    SQLHSTMT        hstmt,
    SQLWCHAR        *szCatalogName,
    SQLSMALLINT     cbCatalogName,
    SQLWCHAR        *szSchemaName,
    SQLSMALLINT     cbSchemaName,
    SQLWCHAR        *szTableName,
    SQLSMALLINT     cbTableName);

SQLRETURN SQL_API SQLDriversW(
    SQLHENV         henv,
    SQLUSMALLINT    fDirection,
    SQLWCHAR        *szDriverDesc,
    SQLSMALLINT     cbDriverDescMax,
    SQLSMALLINT     *pcbDriverDesc,
    SQLWCHAR        *szDriverAttributes,
    SQLSMALLINT     cbDrvrAttrMax,
    SQLSMALLINT     *pcbDrvrAttr);

/* ANSI */
#if defined(_WIN64) || defined(UNIX_64BIT)
SQLRETURN SQL_API SQLColAttributeA(
    SQLHSTMT        hstmt,
    SQLSMALLINT     iCol,
    SQLSMALLINT     iField,
    SQLPOINTER      pCharAttr,
    SQLSMALLINT     cbCharAttrMax,
    SQLSMALLINT     *pcbCharAttr,
    SQLLEN          *pNumAttr);
#else /* _WIN64 || UNIX_64BIT */
SQLRETURN SQL_API SQLColAttributeA(
    SQLHSTMT        hstmt,
    SQLSMALLINT     iCol,
    SQLSMALLINT     iField,
    SQLPOINTER      pCharAttr,
    SQLSMALLINT     cbCharAttrMax,
    SQLSMALLINT     *pcbCharAttr,
    SQLPOINTER      pNumAttr);
#endif /* _WIN64 || UNIX_64BIT */

SQLRETURN SQL_API SQLColAttributesA(
    SQLHSTMT        hstmt,
    SQLUSMALLINT    icol,
    SQLUSMALLINT    fDescType,
    SQLPOINTER      rgbDesc,
    SQLSMALLINT     cbDescMax,
    SQLSMALLINT     *pcbDesc,
    SQLLEN          *pfDesc);

SQLRETURN SQL_API SQLConnectA(
    SQLHDBC         hdbc,
    SQLCHAR         *szDSN,
    SQLSMALLINT     cbDSN,
    SQLCHAR         *szUID,
    SQLSMALLINT     cbUID,
    SQLCHAR         *szAuthStr,
    SQLSMALLINT     cbAuthStr);

SQLRETURN SQL_API SQLDescribeColA(
    SQLHSTMT        hstmt,
    SQLUSMALLINT    icol,
    SQLCHAR         *szColName,
    SQLSMALLINT     cbColNameMax,
    SQLSMALLINT     *pcbColName,
    SQLSMALLINT     *pfSqlType,
    SQLUINTEGER     *pcbColDef,
    SQLSMALLINT     *pibScale,
    SQLSMALLINT     *pfNullable);

SQLRETURN SQL_API SQLErrorA(
    SQLHENV         henv,
    SQLHDBC         hdbc,
    SQLHSTMT        hstmt,
    SQLCHAR         *szSqlState,
    SQLINTEGER      *pfNativeError,
    SQLCHAR         *szErrorMsg,
    SQLSMALLINT     cbErrorMsgMax,
    SQLSMALLINT     *pcbErrorMsg);

SQLRETURN SQL_API SQLExecDirectA(
    SQLHSTMT        hstmt,
    SQLCHAR         *szSqlStr,
    SQLINTEGER      cbSqlStr);

SQLRETURN SQL_API SQLGetConnectAttrA(
    SQLHDBC         hdbc,
    SQLINTEGER      fAttribute,
    SQLPOINTER      rgbValue,
    SQLINTEGER      cbValueMax,
    SQLINTEGER      *pcbValue);

SQLRETURN SQL_API SQLGetCursorNameA(
    SQLHSTMT        hstmt,
    SQLCHAR         *szCursor,
    SQLSMALLINT     cbCursorMax,
    SQLSMALLINT     *pcbCursor);

SQLRETURN SQL_API SQLGetDescFieldA(
    SQLHDESC        hdesc,
    SQLSMALLINT     iRecord,
    SQLSMALLINT     iField,
    SQLPOINTER      rgbValue,
    SQLINTEGER      cbValueMax,
    SQLINTEGER      *pcbValue);

SQLRETURN SQL_API SQLGetDescRecA(
    SQLHDESC        hdesc,
    SQLSMALLINT     iRecord,
    SQLCHAR         *szName,
    SQLSMALLINT     cbNameMax,
    SQLSMALLINT     *pcbName,
    SQLSMALLINT     *pfType,
    SQLSMALLINT     *pfSubType,
    SQLINTEGER      *pLength,
    SQLSMALLINT     *pPrecision,
    SQLSMALLINT     *pScale,
    SQLSMALLINT     *pNullable);

SQLRETURN SQL_API SQLGetDiagFieldA(
    SQLSMALLINT     fHandleType,
    SQLHANDLE       handle,
    SQLSMALLINT     iRecord,
    SQLSMALLINT     fDiagField,
    SQLPOINTER      rgbDiagInfo,
    SQLSMALLINT     cbDiagInfoMax,
    SQLSMALLINT     *pcbDiagInfo);

SQLRETURN SQL_API SQLGetDiagRecA(
    SQLSMALLINT     fHandleType,
    SQLHANDLE       handle,
    SQLSMALLINT     iRecord,
    SQLCHAR         *szSqlState,
    SQLINTEGER      *pfNativeError,
    SQLCHAR         *szErrorMsg,
    SQLSMALLINT     cbErrorMsgMax,
    SQLSMALLINT     *pcbErrorMsg);

SQLRETURN SQL_API SQLGetStmtAttrA(
    SQLHSTMT        hstmt,
    SQLINTEGER      fAttribute,
    SQLPOINTER      rgbValue,
    SQLINTEGER      cbValueMax,
    SQLINTEGER      *pcbValue);

SQLRETURN SQL_API SQLGetTypeInfoA(
    SQLHSTMT        StatementHandle,
    SQLSMALLINT     DataTyoe);

SQLRETURN SQL_API SQLPrepareA(
    SQLHSTMT        hstmt,
    SQLCHAR         *szSqlStr,
    SQLINTEGER      cbSqlStr);

SQLRETURN SQL_API SQLSetConnectAttrA(
    SQLHDBC         hdbc,
    SQLINTEGER      fAttribute,
    SQLPOINTER      rgbValue,
    SQLINTEGER      cbValue);

SQLRETURN SQL_API SQLSetCursorNameA(
    SQLHSTMT        hstmt,
    SQLCHAR         *szCursor,
    SQLSMALLINT     cbCursor);

SQLRETURN SQL_API SQLColumnsA(
    SQLHSTMT        hstmt,
    SQLCHAR         *szCatalogName,
    SQLSMALLINT     cbCatalogName,
    SQLCHAR         *szSchemaName,
    SQLSMALLINT     cbSchemaName,
    SQLCHAR         *szTableName,
    SQLSMALLINT     cbTableName,
    SQLCHAR         *szColumnName,
    SQLSMALLINT     cbColumnName);

SQLRETURN SQL_API SQLGetConnectOptionA(
    SQLHDBC         hdbc,
    SQLUSMALLINT    fOption,
    SQLPOINTER      pvParam);

SQLRETURN SQL_API SQLGetInfoA(
    SQLHDBC         hdbc,
    SQLUSMALLINT    fInfoType,
    SQLPOINTER      rgbInfoValue,
    SQLSMALLINT     cbInfoValueMax,
    SQLSMALLINT    *pcbInfoValue);

SQLRETURN SQL_API SQLGetStmtOptionA(
    SQLHSTMT        hstmt,
    SQLUSMALLINT    fOption,
    SQLPOINTER      pvParam);

SQLRETURN SQL_API SQLSetConnectOptionA(
    SQLHDBC         hdbc,
    SQLUSMALLINT    fOption,
    SQLULEN         vParam);

SQLRETURN SQL_API SQLSetStmtOptionA(
    SQLHSTMT        hstmt,
    SQLUSMALLINT    fOption,
    SQLULEN         vParam);

SQLRETURN SQL_API SQLSpecialColumnsA(
    SQLHSTMT        hstmt,
    SQLUSMALLINT    fColType,
    SQLCHAR         *szCatalogName,
    SQLSMALLINT     cbCatalogName,
    SQLCHAR         *szSchemaName,
    SQLSMALLINT     cbSchemaName,
    SQLCHAR         *szTableName,
    SQLSMALLINT     cbTableName,
    SQLUSMALLINT    fScope,
    SQLUSMALLINT    fNullable);

SQLRETURN SQL_API SQLStatisticsA(
    SQLHSTMT        hstmt,
    SQLCHAR         *szCatalogName,
    SQLSMALLINT     cbCatalogName,
    SQLCHAR         *szSchemaName,
    SQLSMALLINT     cbSchemaName,
    SQLCHAR         *szTableName,
    SQLSMALLINT     cbTableName,
    SQLUSMALLINT    fUnique,
    SQLUSMALLINT    fAccuracy);

SQLRETURN SQL_API SQLTablesA(
    SQLHSTMT        hstmt,
    SQLCHAR         *szCatalogName,
    SQLSMALLINT     cbCatalogName,
    SQLCHAR         *szSchemaName,
    SQLSMALLINT     cbSchemaName,
    SQLCHAR         *szTableName,
    SQLSMALLINT     cbTableName,
    SQLCHAR         *szTableType,
    SQLSMALLINT     cbTableType);

SQLRETURN SQL_API SQLDataSourcesA(
    SQLHENV         henv,
    SQLUSMALLINT    fDirection,
    SQLCHAR         *szDSN,
    SQLSMALLINT     cbDSNMax,
    SQLSMALLINT     *pcbDSN,
    SQLCHAR         *szDescription,
    SQLSMALLINT     cbDescriptionMax,
    SQLSMALLINT     *pcbDescription);

SQLRETURN SQL_API SQLDriverConnectA(
    SQLHDBC         hdbc,
    SQLHWND         hwnd,
    SQLCHAR         *szConnStrIn,
    SQLSMALLINT     cbConnStrIn,
    SQLCHAR         *szConnStrOut,
    SQLSMALLINT     cbConnStrOutMax,
    SQLSMALLINT     *pcbConnStrOut,
    SQLUSMALLINT    fDriverCompletion);

SQLRETURN SQL_API SQLBrowseConnectA(
    SQLHDBC         hdbc,
    SQLCHAR         *szConnStrIn,
    SQLSMALLINT     cbConnStrIn,
    SQLCHAR         *szConnStrOut,
    SQLSMALLINT     cbConnStrOutMax,
    SQLSMALLINT     *pcbConnStrOut);

SQLRETURN SQL_API SQLColumnPrivilegesA(
    SQLHSTMT        hstmt,
    SQLCHAR         *szCatalogName,
    SQLSMALLINT     cbCatalogName,
    SQLCHAR         *szSchemaName,
    SQLSMALLINT     cbSchemaName,
    SQLCHAR         *szTableName,
    SQLSMALLINT     cbTableName,
    SQLCHAR         *szColumnName,
    SQLSMALLINT     cbColumnName);

SQLRETURN SQL_API SQLDescribeParamA(
    SQLHSTMT        hstmt,
    SQLUSMALLINT    ipar,
    SQLSMALLINT     *pfSqlType,
    SQLUINTEGER     *pcbParamDef,
    SQLSMALLINT     *pibScale,
    SQLSMALLINT     *pfNullable);

SQLRETURN SQL_API SQLForeignKeysA(
    SQLHSTMT        hstmt,
    SQLCHAR         *szPkCatalogName,
    SQLSMALLINT     cbPkCatalogName,
    SQLCHAR         *szPkSchemaName,
    SQLSMALLINT     cbPkSchemaName,
    SQLCHAR         *szPkTableName,
    SQLSMALLINT     cbPkTableName,
    SQLCHAR         *szFkCatalogName,
    SQLSMALLINT     cbFkCatalogName,
    SQLCHAR         *szFkSchemaName,
    SQLSMALLINT     cbFkSchemaName,
    SQLCHAR         *szFkTableName,
    SQLSMALLINT     cbFkTableName);

SQLRETURN SQL_API SQLNativeSqlA(
    SQLHDBC         hdbc,
    SQLCHAR         *szSqlStrIn,
    SQLINTEGER      cbSqlStrIn,
    SQLCHAR         *szSqlStr,
    SQLINTEGER      cbSqlStrMax,
    SQLINTEGER      *pcbSqlStr);

SQLRETURN SQL_API SQLPrimaryKeysA(
    SQLHSTMT        hstmt,
    SQLCHAR         *szCatalogName,
    SQLSMALLINT     cbCatalogName,
    SQLCHAR         *szSchemaName,
    SQLSMALLINT     cbSchemaName,
    SQLCHAR         *szTableName,
    SQLSMALLINT     cbTableName);

SQLRETURN SQL_API SQLProcedureColumnsA(
    SQLHSTMT        hstmt,
    SQLCHAR         *szCatalogName,
    SQLSMALLINT     cbCatalogName,
    SQLCHAR         *szSchemaName,
    SQLSMALLINT     cbSchemaName,
    SQLCHAR         *szProcName,
    SQLSMALLINT     cbProcName,
    SQLCHAR         *szColumnName,
    SQLSMALLINT     cbColumnName);

SQLRETURN SQL_API SQLProceduresA(
    SQLHSTMT        hstmt,
    SQLCHAR         *szCatalogName,
    SQLSMALLINT     cbCatalogName,
    SQLCHAR         *szSchemaName,
    SQLSMALLINT     cbSchemaName,
    SQLCHAR         *szProcName,
    SQLSMALLINT     cbProcName);

SQLRETURN SQL_API SQLTablePrivilegesA(
    SQLHSTMT        hstmt,
    SQLCHAR         *szCatalogName,
    SQLSMALLINT     cbCatalogName,
    SQLCHAR         *szSchemaName,
    SQLSMALLINT     cbSchemaName,
    SQLCHAR         *szTableName,
    SQLSMALLINT     cbTableName);

SQLRETURN SQL_API SQLDriversA(
    SQLHENV         henv,
    SQLUSMALLINT    fDirection,
    SQLCHAR         *szDriverDesc,
    SQLSMALLINT     cbDriverDescMax,
    SQLSMALLINT     *pcbDriverDesc,
    SQLCHAR         *szDriverAttributes,
    SQLSMALLINT     cbDrvrAttrMax,
    SQLSMALLINT     *pcbDrvrAttr);

#if !defined(SQL_NOUNICODEMAP)
#if defined(UNICODE)

#define SQLBrowseConnect        SQLBrowseConnectW
#define SQLColAttribute         SQLColAttributeW
#define SQLColAttributes        SQLColAttributesW
#define SQLColumnPrivileges     SQLColumnPrivilegesW
#define SQLColumns              SQLColumnsW
#define SQLConnect              SQLConnectW
#define SQLDataSources          SQLDataSourcesW
#define SQLDescribeCol          SQLDescribeColW
#define SQLDriverConnect        SQLDriverConnectW
#define SQLDrivers              SQLDriversW
#define SQLError                SQLErrorW
#define SQLExecDirect           SQLExecDirectW
#define SQLForeignKeys          SQLForeignKeysW
#define SQLGetConnectAttr       SQLGetConnectAttrW
#define SQLGetConnectOption     SQLGetConnectOptionW
#define SQLGetCursorName        SQLGetCursorNameW
#define SQLGetDescField         SQLGetDescFieldW
#define SQLGetDescRec           SQLGetDescRecW
#define SQLGetDiagField         SQLGetDiagFieldW
#define SQLGetDiagRec           SQLGetDiagRecW
#define SQLGetInfo              SQLGetInfoW
#define SQLGetStmtAttr          SQLGetStmtAttrW
#define SQLGetTypeInfo          SQLGetTypeInfoW
#define SQLNativeSql            SQLNativeSqlW
#define SQLPrepare              SQLPrepareW
#define SQLPrimaryKeys          SQLPrimaryKeysW
#define SQLProcedureColumns     SQLProcedureColumnsW
#define SQLProcedures           SQLProceduresW
#define SQLSetConnectAttr       SQLSetConnectAttrW
#define SQLSetConnectOption     SQLSetConnectOptionW
#define SQLSetCursorName        SQLSetCursorNameW
#define SQLSetDescField         SQLSetDescFieldW
#define SQLSetStmtAttr          SQLSetStmtAttrW
#define SQLSpecialColumns       SQLSpecialColumnsW
#define SQLStatistics           SQLStatisticsW
#define SQLTables               SQLTablesW
#define SQLTablePrivileges      SQLTablePrivilegesW

#endif /* UNICODE */
#endif /* !SQL_NOUNICODEMAP */

#if defined(__cplusplus)
}
#endif

#endif /* SOLIDODBC3_H */
