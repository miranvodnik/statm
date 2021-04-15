/*************************************************************************\
**  source       * sa.h
**  directory    * \solid\os2_32\sa
**  description  * SOLID API interface functions.
**               * 
**  author       * SOLID / jarmo
**  date         * 1993-09-14
**               * 
**               * Copyright UNICOM Systems, Inc. 1993, 2015.
**************************************************************************
** TLIB version ** '%v'
** This source was extracted from library version = '2'
**************************************************************************
** TLIB history **
1 sa.h 15-Mar-94,23:07:24,`JARMO' First version
2 sa.h 27-Apr-94,14:01:20,`JARMO' v.01.10.0000
** TLIB history **
\*************************************************************************/

#ifndef SA_H
#define SA_H

#ifdef __cplusplus
extern "C" {                    /* Assume C declarations for
C++   */
#endif  /* __cplusplus */

/* User must define the used operating system to get correct define
   for SA_EXPORT_H macro:

        SS_WIN  Windows 16-bit
        SS_OS2  OS/2, define as value 16 for 16-bit OS/2 (1.x) and
                value 32 for 32-bit OS/2 (2.x and later)
        SS_NT   Windows NT
*/
#if !defined(SA_EXPORT_H)
#  if defined(SS_WIN)                       /* Windows */
#    define SA_EXPORT_H __far __pascal
#  elif defined(SS_OS2) && (SS_OS2 == 16)   /* OS/2 1.x */
#    define SA_EXPORT_H far pascal
#  elif defined(SS_OS2) && (SS_OS2 == 32)   /* OS/2 2.x */
#    if defined(GCC)
#      define SA_EXPORT_H
#    else /* GCC */
#      define SA_EXPORT_H _System
#    endif /* GCC */
#  elif defined(SS_NT) || defined(SS_WCE)   /* Windows NT / Windows CE*/
#    define SA_EXPORT_H __stdcall
#  else
#    define SA_EXPORT_H
#  endif
#endif

typedef enum {
        SA_RC_SUCC,
        SA_RC_END,
        SA_RC_CONT,
        SA_ERR_FAILED = 100,    /* 100 */
        SA_ERR_CURNOTOPENED,    /* 101 */
        SA_ERR_CUROPENED,       /* 102 */
        SA_ERR_CURNOSEARCH,     /* 103 */
        SA_ERR_CURSEARCH,       /* 104 */
        SA_ERR_ORDERBYILL,      /* 105 */
        SA_ERR_COLNAMEILL,      /* 106 */
        SA_ERR_CONSTRILL,       /* 107 */
        SA_ERR_TYPECONVILL,     /* 108 */
        SA_ERR_UNIQUE,          /* 109 */
        SA_ERR_LOSTUPDATE,      /* 110 */
        SA_ERR_SORTFAILED,      /* 111 */
        SA_ERR_CHSETUNSUPP,     /* 112 */
        SA_ERR_CURNOROW,        /* 113 */
        SA_ERR_COLISNOTNULL,    /* 114 */
        SA_ERR_LOCALSORT,       /* 115 */
        SA_ERR_COMERROR,        /* 116 */
        SA_ERR_NOSTRCONSTR,     /* 117 */
        SA_ERR_ILLENUMVAL,      /* 118 */
        SA_ERR_COLNOTBOUND,     /* 119 */
        SA_ERR_CALLNOSUP,       /* 120 */
        SA_ERR_RPCPARAM,        /* 121 */
        SA_ERR_TABLENOTFOUND,   /* 122 */
        SA_ERR_READONLY,        /* 123 */
        SA_ERR_ILLPARAMCOUNT,   /* 124 */
        SA_ERR_INVARG,          /* 125 */
        SA_ERR_INVCALLSEQ,      /* 126 */
        SA_ERR_NORESETSEARCH,   /* 127 */
        SA_ERR_TOOLONGKEY       /* 128 */
} SaRetT;

typedef enum {
        SADT_CHAR    = 0,
        SADT_INTEGER = 1,
        SADT_FLOAT   = 2,
        SADT_DOUBLE  = 3,
        SADT_DATE    = 4,
        SADT_DFLOAT  = 5,
        SADT_BIGINT  = 6
} SaDataTypeT;

typedef SaDataTypeT SaDatatypeT;

typedef enum {
        SAPT_INT,
        SAPT_LONG,
        SAPT_FLOAT,
        SAPT_DOUBLE,
        SAPT_STR,
        SAPT_DYNDATA
} SaParamTypeT;

typedef enum {
        SA_CHSET_DEFAULT    = 0,    /* Default character set */
        SA_CHSET_NOCNV      = 1,    /* No conversion */
        SA_CHSET_ANSI       = 2     /* ISO Latin 1, e.g. MS Windows */
} SaChSetT;

typedef enum {
        SA_CHCOLLATION_ISO8859_1 = 0,
        SA_CHCOLLATION_FIN       = 1
} SaChCollationT;

typedef enum {
        SA_LOCK_SHARE,
        SA_LOCK_FORUPDATE,
        SA_LOCK_EXCLUSIVE
} sa_lockmode_t;

typedef enum {
        SA_TRANS_READ_COMMITTED  = 1,
        SA_TRANS_REPEATABLE_READ = 2,
        SA_TRANS_SERIALIZABLE    = 3
} SaTransIsolationT;

typedef struct SaConnectSt      SaConnectT;
typedef struct SaCursorSt       SaCursorT;
typedef struct SaSQLCursorSt    SaSQLCursorT;
typedef struct SaColSearchSt    SaColSearchT;

typedef struct SaDynDataSt      SaDynDataT;
typedef char*                   SaDynStrT;

typedef struct {
        char data[28];
} SaDfloatT;

typedef struct {
        char data[11];
} SaDateT;

/**********************************************************************\
        SOLID API ROUTINES
\**********************************************************************/

int SA_EXPORT_H SaIsNetwork(void);

SaConnectT* SA_EXPORT_H SaConnect(
        char* servername,
        char* username,
        char* password);

SaConnectT* SA_EXPORT_H SaConnectUTF8(
        char* servername,
        char* username,
        char* password);

void SA_EXPORT_H SaDisconnect(
        SaConnectT* scon);

SaRetT SA_EXPORT_H SaDefineChSet(
        SaConnectT* scon,
        SaChSetT chset);

SaRetT SA_EXPORT_H SaSetDateFormat(
        SaConnectT* scon,
        char* dateformat);

SaRetT SA_EXPORT_H SaSetTimeFormat(
        SaConnectT* scon,
        char* timeformat);

SaRetT SA_EXPORT_H SaSetTimestampFormat(
        SaConnectT* scon,
        char* timestampformat);

SaRetT SA_EXPORT_H SaSetSortBufSize(
        SaConnectT* scon,
        unsigned long size);

SaRetT SA_EXPORT_H SaSetSortMaxFiles(
        SaConnectT* scon,
        unsigned int nfiles);

SaRetT SA_EXPORT_H SaSetTransIsolation(
        SaConnectT* scon,
        SaTransIsolationT isolation);
    
void SA_EXPORT_H SaTransBegin(
        SaConnectT* scon);

SaRetT SA_EXPORT_H SaTransCommit(
        SaConnectT* scon);

SaRetT SA_EXPORT_H SaTransRollback(
        SaConnectT* scon);

int SA_EXPORT_H SaErrorInfo(
        SaConnectT* scon,
        char** errstr,
        int* errcode);

SaCursorT* SA_EXPORT_H SaCursorCreate(
        SaConnectT* scon,
        char* tablename);

void SA_EXPORT_H SaCursorFree(
        SaCursorT* scur);

int SA_EXPORT_H SaCursorErrorInfo(
        SaCursorT* scur,
        char** errstr,
        int* errcode);

SaRetT SA_EXPORT_H SaCursorColInt(
        SaCursorT* scur,
        char* colname,
        int* intptr);

SaRetT SA_EXPORT_H SaCursorColLong(
        SaCursorT* scur,
        char* colname,
        long* longptr);

SaRetT SA_EXPORT_H SaCursorColFloat(
        SaCursorT* scur,
        char* colname,
        float* floatptr);

SaRetT SA_EXPORT_H SaCursorColDouble(
        SaCursorT* scur,
        char* colname,
        double* doubleptr);

SaRetT SA_EXPORT_H SaCursorColDfloat(
        SaCursorT* scur,
        char* colname,
        SaDfloatT* dfloatptr);

SaRetT SA_EXPORT_H SaCursorColBigintStr(
        SaCursorT* scur,
        char* colname,
        char** strptr);

SaRetT SA_EXPORT_H SaCursorColBigint(
        SaCursorT* scur,
        char* colname,
        void* bigintptr);
    

SaRetT SA_EXPORT_H SaCursorColStr(
        SaCursorT* scur,
        char* colname,
        char** strptr);

SaRetT SA_EXPORT_H SaCursorColDate(
        SaCursorT* scur,
        char* colname,
        SaDateT* dateptr);

SaRetT SA_EXPORT_H SaCursorColTime(
        SaCursorT* scur,
        char* colname,
        SaDateT* timeptr);

SaRetT SA_EXPORT_H SaCursorColTimestamp(
        SaCursorT* scur,
        char* colname,
        SaDateT* timestampptr);

SaRetT SA_EXPORT_H SaCursorColData(
        SaCursorT* scur,
        char* colname,
        char** dataptr,
        unsigned* lenptr);

SaRetT SA_EXPORT_H SaCursorColFixstr(
        SaCursorT* scur,
        char* colname,
        char* fixstr,
        unsigned size);

SaRetT SA_EXPORT_H SaCursorColDynData(
        SaCursorT* scur,
        char* colname,
        SaDynDataT* dd);
#ifdef SS_UNICODE_DATA

SaRetT SA_EXPORT_H SaCursorColDynDataUTF8(
        SaCursorT* scur,
        char* colname,
        SaDynDataT* dd);

#endif /* SS_UNICODE_DATA */

SaRetT SA_EXPORT_H SaCursorColDynStr(
        SaCursorT* scur,
        char* colname,
        SaDynStrT* ds);

SaRetT SA_EXPORT_H SaCursorColNullFlag(
        SaCursorT* scur,
        char* colname,
        int* p_isnullflag);

SaRetT SA_EXPORT_H SaCursorColDateFormat(
        SaCursorT* scur,
        char* colname,
        char* dtformat);

SaRetT SA_EXPORT_H SaCursorAscending(
        SaCursorT* scur,
        char* colname);

SaRetT SA_EXPORT_H SaCursorDescending(
        SaCursorT* scur,
        char* colname);

SaRetT SA_EXPORT_H SaCursorOrderby(
        SaCursorT* scur,
        char* colname);

SaRetT SA_EXPORT_H SaCursorEqual(
        SaCursorT* scur,
        char* colname);

SaRetT SA_EXPORT_H SaCursorLike(
        SaCursorT* scur,
        char* colname,
        int likelen);

SaRetT SA_EXPORT_H SaCursorAtleast(
        SaCursorT* scur,
        char* colname);

SaRetT SA_EXPORT_H SaCursorAtmost(
        SaCursorT* scur,
        char* colname);

SaRetT SA_EXPORT_H SaCursorSetLockMode(
        SaCursorT* scur,
        sa_lockmode_t lockmode);

SaRetT SA_EXPORT_H SaCursorSetRowsPerMessage(
        SaCursorT* scur,
        int rows_per_message);

SaRetT SA_EXPORT_H SaCursorOpen(
        SaCursorT* scur);

SaRetT SA_EXPORT_H SaCursorSearch(
        SaCursorT* scur);

SaRetT SA_EXPORT_H SaCursorReSearch(
        SaCursorT* scur);

SaRetT SA_EXPORT_H SaCursorSearchByRowid(
        SaCursorT* scur,
        void* rowid,
        int rowidlen);

SaRetT SA_EXPORT_H SaCursorSearchReset(
        SaCursorT* scur);

SaRetT SA_EXPORT_H SaCursorNext(
        SaCursorT* scur);

SaRetT SA_EXPORT_H SaCursorPrev(
        SaCursorT* scur);

SaRetT SA_EXPORT_H SaCursorBegin(
        SaCursorT* scur);

SaRetT SA_EXPORT_H SaCursorEnd(
        SaCursorT* scur);

SaRetT SA_EXPORT_H SaCursorSetPosition(
        SaCursorT* scur);

SaRetT SA_EXPORT_H SaCursorClearConstr(
        SaCursorT* scur);

SaRetT SA_EXPORT_H SaCursorInsert(
        SaCursorT* scur);

SaRetT SA_EXPORT_H SaCursorDelete(
        SaCursorT* scur);

SaRetT SA_EXPORT_H SaCursorUpdate(
        SaCursorT* scur);

SaColSearchT* SA_EXPORT_H SaColSearchCreate(
        SaConnectT* scon,
        char* tablename);

void SA_EXPORT_H SaColSearchFree(
        SaColSearchT* colsearch);

int SA_EXPORT_H SaColSearchNext(
        SaColSearchT* colsearch,
        char** p_colname,
        SaDataTypeT* p_coltype);

SaRetT SA_EXPORT_H SaSQLExecDirect(
        SaConnectT* scon,
        char* sqlstr);

/* Init routine. Needed when SA routines are called before SaConnect.
 */
void SA_EXPORT_H SaGlobalInit(
        void);

void SA_EXPORT_H SaGlobalDone(
        void);

/* Array routines.
 */
SaRetT SA_EXPORT_H SaArrayInsert(
        SaCursorT* scur);

SaRetT SA_EXPORT_H SaArrayFlush(
        SaConnectT* scon,
        SaRetT* rctab);

/**********************************************************************\
        DYNAMIC STRING ROUTINES
\**********************************************************************/

SaDynStrT SA_EXPORT_H SaDynStrCreate(
        void);

void SA_EXPORT_H SaDynStrFree(
        SaDynStrT* p_ds);

void SA_EXPORT_H SaDynStrMove(
        SaDynStrT* p_ds,
        char* str);

void SA_EXPORT_H SaDynStrAppend(
        SaDynStrT* p_ds,
        char* str);

/**********************************************************************\
        DYNAMIC DATA ROUTINES
\**********************************************************************/

SaDynDataT* SA_EXPORT_H SaDynDataCreate(
	void);

void SA_EXPORT_H SaDynDataFree(
	SaDynDataT* dd);

void SA_EXPORT_H SaDynDataClear(
	SaDynDataT* dd);

void SA_EXPORT_H SaDynDataMove(
	SaDynDataT* dd,
	char* data,
	unsigned len);

void SA_EXPORT_H SaDynDataMoveRef(
	SaDynDataT* dd,
	char* data,
	unsigned len);

void SA_EXPORT_H SaDynDataAppend(
	SaDynDataT* dd,
	char* data,
	unsigned len);

void SA_EXPORT_H SaDynDataChLen(
	SaDynDataT* dd,
	unsigned len);

char* SA_EXPORT_H SaDynDataGetData(
	SaDynDataT* dd);

unsigned SA_EXPORT_H SaDynDataGetLen(
	SaDynDataT* dd);

/**********************************************************************\
        DATE ROUTINES
\**********************************************************************/

SaDateT* SA_EXPORT_H SaDateCreate(
        void);

void SA_EXPORT_H SaDateFree(
        SaDateT* date);

SaRetT SA_EXPORT_H SaDateSetAsciiz(
        SaDateT* date,
        char* format,
        char* asciiz);

SaRetT SA_EXPORT_H SaDateSetTimet(
        SaDateT* date,
        long timet);

SaRetT SA_EXPORT_H SaDateToAsciiz(
        SaDateT* date,
        char* format,
        char* asciiz);

SaRetT SA_EXPORT_H SaDateToTimet(
        SaDateT* date,
        long* p_timet);

/**********************************************************************\
        DFLOAT ROUTINES
\**********************************************************************/

SaRetT SA_EXPORT_H SaDfloatSum(
        SaDfloatT* p_result_dfl,
        SaDfloatT* p_dfl1,
        SaDfloatT* p_dfl2);

SaRetT SA_EXPORT_H SaDfloatDiff(
        SaDfloatT* p_result_dfl,
        SaDfloatT* p_dfl1,
        SaDfloatT* p_dfl2);

SaRetT SA_EXPORT_H SaDfloatProd(
        SaDfloatT* p_result_dfl,
        SaDfloatT* p_dfl1,
        SaDfloatT* p_dfl2);

SaRetT SA_EXPORT_H SaDfloatQuot(
        SaDfloatT* p_result_dfl,
        SaDfloatT* p_dfl1,
        SaDfloatT* p_dfl2);

int SA_EXPORT_H SaDfloatCmp(
        SaDfloatT* p_dfl1,
        SaDfloatT* p_dfl2);

int SA_EXPORT_H SaDfloatOverflow(
        SaDfloatT* p_dfl);

int SA_EXPORT_H SaDfloatUnderflow(
        SaDfloatT* p_dfl);

SaRetT SA_EXPORT_H SaDfloatSetAsciiz(
        SaDfloatT* p_dfl,
        char* asciiz);

SaRetT SA_EXPORT_H SaDfloatToAsciiz(
        SaDfloatT* p_dfl,
        char* asciiz);

int SA_EXPORT_H SaIsLocalConnect(
        SaConnectT* scon);

int SA_EXPORT_H SaIsLocalServer(void);

#ifdef __cplusplus
}                                    /* End of extern "C" { */
#endif  /* __cplusplus */

#endif /* SA_H */
