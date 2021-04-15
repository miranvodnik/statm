/************************************************************************\
**  source       * solidodbc3.h
**  directory    * 
**  description  * Solid ODBC driver include
**               *
**               * Copyright (c) Solid Information Technology Ltd. 1993, 2008.
\*************************************************************************/

#ifndef SOLIDODBC3_H
#define SOLIDODBC3_H


/***************************************************************
 *
 * Solid extension defines
 *
 */
#define SQL_ATTR_SET_CONNECTION_DEAD	1210	/* Solid extension attribute */

#define SQL_ATTR_TF_LEVEL           1317  /* Solid extension attribute */
#define SQL_ATTR_TC_PRIMARY         1318  /* Solid extension attribute */
#define SQL_ATTR_TF_PRIMARY         SQL_ATTR_TC_PRIMARY
#define SQL_ATTR_TC_SECONDARY       1319  /* Solid extension attribute */
#define SQL_ATTR_TF_WAITING         1320  /* Solid extension attribute */
#define SQL_ATTR_PA_LEVEL           1321  /* Solid extension attribute */
#define SQL_ATTR_TC_WORKLOAD_CONNECTION 1322 /* Solid extension attribute */

/* Extension attributes for millisecond-resolution timeouts. */
#define SQL_ATTR_LOGIN_TIMEOUT_MS      1323  /* Solid extension attribute */
#define SQL_ATTR_CONNECTION_TIMEOUT_MS 1324  /* Solid extension attribute */
#define SQL_ATTR_QUERY_TIMEOUT_MS      1325  /* Solid extension attribute */

#define SQL_ATTR_HANDLE_VALIDATION  1326

#define SQL_ATTR_IDLE_TIMEOUT       1327

#define SQL_ATTR_PASSTHROUGH_READ   1328
#define SQL_ATTR_PASSTHROUGH_WRITE  1329

#define SQL_ATTR_LC_CTYPE           1330
#define SQL_C_UTF8                  1999


/***************************************************************
 *
 * Additional defines for Unix platforms
 *
 */
#if !defined(SS_NT) && !defined(SS_WCE)

#ifdef SS_CRX
#include <runetype_f.h>
#elif SS_FBX==5
# include <wchar.h>
# define _WCHAR_T_DEFINED 1
#include <runetype.h>
#elif defined(SS_VXW)
#define wchar_t unsigned short
#elif defined(SS_OSE)
#include <stddef.h>
#define _WCHAR_T_DEFINED 1
/* #define wchar_t int */
#elif defined(SS_BSI)
#include <rune.h>
#elif defined(SS_E32)
#else
#include <wchar.h>
# ifndef _WCHAR_T_DEFINED
#  define _WCHAR_T_DEFINED 1
# endif
#endif


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

#ifndef Widget
#define Widget unsigned long
#endif

#ifndef WORD
#define WORD short int
#endif

#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef CHAR
#define CHAR unsigned char
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

#ifdef _WCHAR_T_DEFINED
#define SQLWCHAR wchar_t
#else
#define SQLWCHAR unsigned short 
#endif

#define SQLROWCOUNT     unsigned long
#define SQLROWSETSIZE   unsigned long
#define SQLTRANSID      unsigned long
#define SQLROWOFFSET    long
#define SQLINTEGER      int
#define SQLUINTEGER     unsigned int
#define SQLLEN          long            
#define SQLULEN         unsigned long   
#define SDWORD          int               
#define SLONG           signed int
#define ULONG           unsigned int
#define SWORD           short int         
#define SSHORT          signed short
#define UDWORD          unsigned int      
#define UWORD           unsigned short int
#define DWORD           int
#define SFLOAT          float
#define SDOUBLE         double
#define LDOUBLE         double
#define SQLSMALLINT     short           
#define SQLUSMALLINT    unsigned short  
#define SQLRETURN       short
#define RETCODE         signed  short
#define SQLCHAR         unsigned char   
#define SQLSCHAR        signed char   
#define SQLHWND         unsigned long
#define SQLNUMERIC      unsigned char 
#define SQLREAL         float         
#define SQLDOUBLE       double
#define SQLDECIMAL      unsigned char 
#define SQLFLOAT        double
#define SQLTIME         unsigned char 
#define SQLDATE         unsigned char
#define SQLTIMESTAMP    unsigned char 
#define SQLVARCHAR      char

#ifdef UNIX_64BIT
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
#   define ODBCINT64    struct solid_odbc_bigint_struct
#   define UODBCINT64   struct solid_odbc_bigint_struct_u
#  endif
# endif
#endif

#ifdef ODBCINT64
typedef ODBCINT64   SQLBIGINT;
#endif
#ifdef UODBCINT64
typedef UODBCINT64  SQLUBIGINT;
#endif

#ifdef UNIX_64BIT
  #define SQLSETPOSIROW   unsigned long   
  typedef void*   SQLHANDLE;       
  typedef void*   SQLHENV;         
  typedef void*   SQLHDBC;         
  typedef void*   SQLHSTMT;        
#else
  #define SQLSETPOSIROW   unsigned short
  typedef int     SQLHANDLE;       
  typedef int     SQLHENV;         
  typedef int     SQLHDBC;         
  typedef int     SQLHSTMT;        
#endif

typedef void*   HENV;  
typedef void*   HDBC;            
typedef void*   HSTMT;           
typedef void*   SQLPOINTER;      
typedef void*   SQLHDESC;        
typedef void*   PTR;             


typedef struct date_st
{
        SQLSMALLINT  year;
        SQLUSMALLINT  month;
        SQLUSMALLINT  day;
} SQL_DATE_STRUCT;

typedef struct time_st
{
        SQLUSMALLINT  hour;
        SQLUSMALLINT  minute;
        SQLUSMALLINT  second;
} SQL_TIME_STRUCT;

typedef struct timestamp_st
{
        SQLSMALLINT  year;
        SQLUSMALLINT month;
        SQLUSMALLINT day;
        SQLUSMALLINT hour;
        SQLUSMALLINT minute;
        SQLUSMALLINT second;
        SQLUINTEGER  fraction;
} SQL_TIMESTAMP_STRUCT;

#define SQL_MAX_NUMERIC_LEN		16
typedef struct numeric_st
{
	SQLCHAR	    precision;
	SQLSCHAR	scale;
	SQLCHAR	    sign;  
	SQLCHAR	    val[SQL_MAX_NUMERIC_LEN];
} SQL_NUMERIC_STRUCT;

#define DATE_STRUCT         SQL_DATE_STRUCT
#define TIME_STRUCT         SQL_TIME_STRUCT
#define TIMESTAMP_STRUCT    SQL_TIMESTAMP_STRUCT


typedef struct yearmonth_st
{
        SQLUINTEGER    year;
        SQLUINTEGER    month;
} SQL_YEAR_MONTH_STRUCT;

typedef struct daysecond_st
{
        SQLUINTEGER    day;
        SQLUINTEGER    hour;
        SQLUINTEGER    minute;
        SQLUINTEGER    second;
        SQLUINTEGER    fraction;
} SQL_DAY_SECOND_STRUCT;

typedef enum 
{
	SQL_IS_YEAR	                = 1,
	SQL_IS_MONTH                = 2,
	SQL_IS_DAY	                = 3,
	SQL_IS_HOUR	                = 4,
	SQL_IS_MINUTE               = 5,
	SQL_IS_SECOND               = 6,
	SQL_IS_YEAR_TO_MONTH        = 7,
	SQL_IS_DAY_TO_HOUR	        = 8,
	SQL_IS_DAY_TO_MINUTE        = 9,
	SQL_IS_DAY_TO_SECOND        = 10,
	SQL_IS_HOUR_TO_MINUTE       = 11,
	SQL_IS_HOUR_TO_SECOND       = 12,
	SQL_IS_MINUTE_TO_SECOND     = 13
} SQLINTERVAL;

typedef struct interval_st
{
	SQLINTERVAL interval_type;
	SQLSMALLINT interval_sign;
	union {
		SQL_YEAR_MONTH_STRUCT   year_month;
		SQL_DAY_SECOND_STRUCT   day_second;
	} intval;

} SQL_INTERVAL_STRUCT;



#define SQL_API

#define __SQLTYPES

#endif	/* End of additional Unix defines */

/***************************************************************
 *
 *   Microsoft's ODBC headers
 *
 */

#ifdef UNIX_64BIT
#define _WIN64
#endif

#include <sqltypes.h>
#include <sql.h>
#include <sqlext.h>
#include <sqlucode.h>


#ifdef UNIX_64BIT
#undef _WIN64
#endif

#else
#endif /* SOLIDODBC3_H */
