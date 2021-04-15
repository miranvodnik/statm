/*************************************************************************\
**  source       * sscapi.h
**  
**  description  * SOLID Accelerator Control API.
**               * 
**               * 
**               * Copyright UNICOM Systems, Inc. 2000, 2015.
\*************************************************************************/

#ifndef SSCAPI_H
#define SSCAPI_H

#ifdef __cplusplus
extern "C" {                    /* Assume C declarations for C++   */
#endif  /* __cplusplus */


#if defined(WIN32)
#define SSC_CALL __stdcall
#else
#define SSC_CALL /* no special calling convention */
#endif

/* API return codes */
typedef enum {
        SSC_SUCCESS                     = 0,
        SSC_ERROR                       = 1,
        SSC_ABORT                       = 2,
        SSC_FINISHED                    = 3,
        SSC_CONT                        = 4,
        SSC_CONNECTIONS_EXIST           = 5,
        SSC_UNFINISHED_TASKS            = 6,
        SSC_INVALID_HANDLE              = 7,
        SSC_INVALID_LICENSE             = 8,
        SSC_NODATABASEFILE              = 9,
        SSC_SERVER_NOTRUNNING           = 10,
        SSC_INFO_SERVER_RUNNING         = 11,
        SSC_SERVER_INNETCOPYMODE        = 12,
        SSC_DBOPENFAIL                  = 13,
        SSC_DBCONNFAIL                  = 14,
        SSC_DBTESTFAIL                  = 15,
        SSC_DBFIXFAIL                   = 16,
        SSC_MUSTCONVERT                 = 17,
        SSC_DBEXIST                     = 18,
        SSC_DBNOTCREATED                = 19,
        SSC_DBCREATEFAIL                = 20,
        SSC_COMINITFAIL                 = 21,
        SSC_COMLISTENFAIL               = 22,
        SSC_SERVICEFAIL                 = 23,
        SSC_ILLARGUMENT                 = 24,
        SSC_CHDIRFAIL                   = 25,
        SSC_INFILEOPENFAIL              = 26,
        SSC_OUTFILEOPENFAIL             = 27,
        SSC_SRVCONNFAIL                 = 28,
        SSC_INITERROR                   = 29,
        SSC_CORRUPTED_DBFILE            = 30,
        SSC_CORRUPTED_LOGFILE           = 31,
        SSC_BROKENNETCOPY               = 32,
        SSC_INVALIDVERSION              = 33
} SscRetT;

/* Task priorities */
typedef enum {
        SSC_PRIO_HIGH = 0,
        SSC_PRIO_NORMAL,
        SSC_PRIO_IDLE,
        SSC_PRIO_DEFAULT
} SscTaskPrioT;

/* Notify function types */
typedef enum {
        SSC_NOTIFY_SHUTDOWN = 0,
        SSC_NOTIFY_SHUTDOWN_REQUEST,
        SSC_NOTIFY_BACKUP_REQUEST,
        SSC_NOTIFY_ROWSTOMERGE,
        SSC_NOTIFY_MERGE_REQUEST,
        SSC_NOTIFY_CHECKPOINT_REQUEST,
        SSC_NOTIFY_IDLE,
        SSC_NOTIFY_NETCOPY_REQUEST,
        SSC_NOTIFY_NETCOPY_FINISHED,
        SSC_NOTIFY_EMERGENCY_EXIT,
        SSC_NOTIFY_ENUMEND
} SscNotFunT;

/* Server status identifiers */
typedef enum {
        SSC_STAT_NUM_WRITES_NOT_MERGED = 0,
        SSC_STAT_NUM_SERVER_THREADS,
        SSC_STAT_NUM_END
} SscStatusT;


/* LocalServer States: */
typedef long SscStateT;

/*
 * set server open:   state = state | SSC_STATE_OPEN;
 * set server closed: state = state & ~SSC_STATE_OPEN;
 * set prefetch on:   state = state | SSC_STATE_PREFETCH;
 * set prefetch off:  state = state & ~SSC_STATE_PREFETCH;
 */

#define SSC_STATE_CLOSED           (0)
#define SSC_STATE_OPEN             (1 << 0)
#define SSC_STATE_PREFETCH         (1 << 1)
#define SSC_STATE_DISABLE_NETCOPY  (1 << 2)
#define SSC_STATE_MSGLOG_ALLOWED   (1 << 3)

/* Task classes */
typedef long SscTaskSetT;

#define SSC_TASK_NONE           (0)
#define SSC_TASK_CHECKPOINT     (1 << 0)
#define SSC_TASK_BACKUP         (1 << 1)
#define SSC_TASK_MERGE          (1 << 2)
#define SSC_TASK_LOCALUSERS     (1 << 3)
#define SSC_TASK_REMOTEUSERS    (1 << 4)
#define SSC_TASK_SYNC_HISTCLEAN (1 << 5)
#define SSC_TASK_SYNC_MESSAGE   (1 << 6)
#define SSC_TASK_HOTSTANDBY     (1 << 7)
#define SSC_TASK_HOTSTANDBY_CATCHUP     (1 << 8)
#define SSC_TASK_BACKGROUND     (1 << 9)

#define SSC_TASK_ALL            (SSC_TASK_CHECKPOINT | SSC_TASK_BACKUP | \
                                SSC_TASK_MERGE | SSC_TASK_LOCALUSERS | \
                                SSC_TASK_REMOTEUSERS | SSC_TASK_SYNC_HISTCLEAN | \
                                SSC_TASK_SYNC_MESSAGE | SSC_TASK_HOTSTANDBY | \
                                SSC_TASK_HOTSTANDBY_CATCHUP | SSC_TASK_BACKGROUND)

/* Server handle */
typedef struct ssc_serverhandle_st* ssc_serverhandle_t;
typedef ssc_serverhandle_t SscServerT;

/* Notifier function type */
typedef int (SSC_CALL *notify_fun)(
                SscServerT h,
                SscNotFunT what,
                void* userdata);


/*********************************
 *  SOLID Accelerator Control API. *
 *********************************/

/* Starts the server */
SscRetT SSC_CALL SSCStartServer(
            int     argc,
            char*   argv[],
            SscServerT* h,
            SscStateT runflags);

SscRetT SSC_CALL SSCStartSMAServer(
            int     argc,
            char*   argv[],
            SscServerT* h,
            SscStateT runflags);

/* Starts the server */
SscRetT SSC_CALL SSCStartDisklessServer(
            int     argc,
            char*   argv[],
            SscServerT* h,
            SscStateT runflags,
            char*   lic_string,
            char*   ini_string);

SscRetT SSC_CALL SSCStartSMADisklessServer(
            int     argc,
            char*   argv[],
            SscServerT* h,
            SscStateT runflags,
            char*   lic_string,
            char*   ini_string);

/* Shutdown */
SscRetT SSC_CALL SSCStopServer(
            SscServerT  h,
            int                 force);

/* Is server running */
int SSC_CALL SSCIsRunning(
            SscServerT h);

/* Sets the state of the server */
SscRetT SSC_CALL SSCSetState(
            SscServerT  h,
            SscStateT           runflags);

/* Sets a notifier */
SscRetT SSC_CALL SSCSetNotifier(
            SscServerT h,
            SscNotFunT  what,
            notify_fun  handler,
            void*       userdata);

/* Run active tasks */
SscRetT SSC_CALL SSCAdvanceTasks(
            SscServerT  h,
            long timeout_ms);

/* Returns types of currently active tasks */
SscTaskSetT SSC_CALL SSCGetActiveTaskClass(
            SscServerT h);

/* Returns types of currently suspended tasks */
SscTaskSetT SSC_CALL SSCGetSuspendedTaskClass(
            SscServerT h);

/* Sets tasks to suspend state */
SscRetT SSC_CALL SSCSuspendTaskClass(
            SscServerT h,
            SscTaskSetT tasktype);

/* Resumes a task class */
SscRetT SSC_CALL SSCResumeTaskClass(
            SscServerT h,
            SscTaskSetT tasktype);

/* Sets task class priority */
SscRetT SSC_CALL SSCSetTaskClassPrio(
            SscServerT h,
            SscTaskSetT tasktype,
            SscTaskPrioT prio);

/* Returns the priority of a given task class */
SscRetT SSC_CALL SSCGetTaskClassPrio(
            SscServerT h,
            SscTaskSetT tasktype,
            SscTaskPrioT* prio);

/* Sets the connection priority */
SscRetT SSC_CALL SSCSetConnectionPrio(
            SscServerT h,
            int userid,
            SscTaskPrioT prio);

/* Get the connection priority */
SscRetT SSC_CALL SSCGetConnectionPrio(
            SscServerT h,
            int userid,
            SscTaskPrioT* p_prio);

/* Returns counter status */
SscRetT SSC_CALL SSCGetStatusNum(
            SscServerT h,
            SscStatusT stat,
            long* num);

/* Runs merge */
SscRetT SSC_CALL SSCRunMerge(
            SscServerT h,
            long timeout_ms);

/* Returns non zero if this is really linked to the LocalServer */
int SSC_CALL SSCIsThisLocalServer(
            void);

/* Registers a user thread */
SscRetT SSC_CALL SSCRegisterThread(
            SscServerT h);

/* Unregisters a user thread */
SscRetT SSC_CALL SSCUnregisterThread(
            SscServerT h);

/* Returns the server handle */
SscServerT SSC_CALL SSCGetServerHandle(void);

/* some utility functions */

int SSC_CALL SSCTaskClass2Str(
            SscTaskSetT tasktype,
            char** p_strtaskclass);

int SSC_CALL SSCStr2TaskClass(
            char* strtaskclass,
            SscTaskSetT* p_tasktype);

int SSC_CALL SSCPrio2Str(
            SscTaskPrioT prio,
            char** p_strprio);

int SSC_CALL SSCStr2Prio(
            char* strprio,
            SscTaskPrioT* p_prio);


#if defined(USER_DEFINED_EXT_FUNCTIONS)
/*##**********************************************************************\
 *
 *      SSCGetUDFContext
 *
 * Reads address of the memory context of exteernal procedure
 * being executed. Address of external procedure object is
 * stored in connection handle.
 *
 * Parameters:
 *      hdbc - in, use
 *          Connection handle
 *
 * Return value - ref:
 *      address of memory context if it exists
 *      NULL if there is no context in procedure
 *
 * Limitations:
 *
 * Globals used:
 */
void* SSC_CALL SSCGetUDFContext(
        void* hdbc);

/*##**********************************************************************\
 *
 *      SSCGetUDFContextSize
 *
 * Size of memory context of external procedure.
 *
 * Parameters:
 *      hdbc - in, use
 *          Connection handle
 *
 * Return value:
 *      context size
 *
 * Limitations:
 *
 * Globals used:
 */
size_t SSC_CALL SSCGetUDFContextSize(
        void* hdbc);

/*##**********************************************************************\
 *
 *      *SSCAllocUDFContext
 *
 * Allocates memory context that is used during execution of user-defined
 * functions and external procedures.
 *
 * Parameters:
 *      hdbc - in out, use
 *          conection handle
 *
 *      size - in
 *          needed context size in bytes
 *
 * Return value - ref:
 *      pointer to allocated memory context.
 *
 * Limitations:
 *
 * Globals used:
 */
void* SSC_CALL SSCAllocUDFContext(
        void* hdbc,
        size_t size);

/*##**********************************************************************\
 *
 *      SSCFreeUDFContext
 *
 * Frees memory context
 *
 * Parameters:
 *      hdbc - in out, use
 *          Connection handle
 *
 * Return value:
 *
 * Limitations:
 *
 * Globals used:
 */
void SSC_CALL SSCFreeUDFContext(
        void* hdbc);

/*##**********************************************************************\
 *
 *      SSCGetUDFParamBufSize
 *
 * Gets parameter buffer size for a given external function parameter.
 * This must only be used for the parameters before the nullind_arra
 *
 * Parameters:
 *      param - in, use
 *          parameter pointer as given to external procedure or function
 *
 * Return value:
 *      size of the buffer area for the data. Note: If parameter is mode IN,
 *      this can not (or should not) be used for writing.
 *
 * Limitations:
 *
 * Globals used:
 */
size_t SSC_CALL SSCGetUDFParamBufSize(
        void* param);
    
#endif /* USER_DEFINED_EXT_FUNCTIONS */
          
#ifdef __cplusplus
}                                    /* End of extern "C" { */
#endif  /* __cplusplus */

#endif /* SSCAPI_H */
