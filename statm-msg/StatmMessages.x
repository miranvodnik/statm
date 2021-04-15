
#ifdef RPC_HDR
%#include <uuid/uuid.h>
#define STATPROG          0x20004060
#define STATFUNCTUN          1			/* union of all CDCP administrative requests */
#define	STATVERSION          STATVERSION_1	/* current version */

#if	0
	static	int	iencode = 0;
	static	int	idecode = 0;
	static	int	ifree = 0;

	switch (xdrs->x_op)
	{
	case	XDR_ENCODE:
		if (++iencode > 1000)
		{
			printf ("ENCODE %016x\n", objp);
			iencode = 0;
		}
		break;
	case	XDR_DECODE:
		if (++idecode > 1000)
		{
			printf ("DECODE %016x\n", objp);
			idecode = 0;
		}
		break;
	case	XDR_FREE:
		if (++ifree > 1000)
		{
			printf ("FREE %016x\n", objp);
			ifree = 0;
		}
		break;
	}
#endif

#endif

struct	StatVersion
{
	u_int	m_major;
	u_int	m_minor;
};

enum	StatEntity
{
	StatNoEntity,
	StatClientEntity,
	StatControllerEntity,
	StatWorkerEntity
};

enum	StatErrorType
{
	StatAppErrorType,
	StatSysErrorType
};

enum	StatErrorCode
{
	NoSqlError,
	SqlNotConnected,
	SqlAlreadyConnected,
	SqlUnknownRequest
};

struct	StatError
{
	StatErrorType	m_type;
	int	m_errorCode;
	string	m_errorMessage<>;
	struct StatError*	m_next;
};

struct	StatMessageHeader
{
	StatVersion	m_version;
	StatEntity	m_entity;
	u_int	m_pid;
	u_int	m_index;
	u_char	m_hash[16];
	struct StatError*	m_error;
};

enum	StatRequestType
{
	FirstRequestCode = 0,
	StatUnknownRequestCode = -1,
	AdminRequestCode = 1
};

enum	StatReplyType
{
	FirstReplyCode = 0,
	StatUnknownReplyCode = -1,
	AdminReplyCode = 1
};

struct	StatParameter
{
	u_int	m_flags;
	uint64_t	m_longVal;
	string	m_stringVal<>;
};

struct	StatAdminRequest
{
	StatMessageHeader	m_header;
	u_int	m_command;
	StatParameter	m_parameters<>;
};

union	StatRequest
switch (StatRequestType m_requestType)
{
	case	StatUnknownRequestCode:
		StatMessageHeader	m_header;
	case	AdminRequestCode:
		StatAdminRequest	m_adminRequest;
};

union	StatReply
switch (StatReplyType m_replyType)
{
	case	StatUnknownReplyCode:
		StatMessageHeader	m_header;
};

program	STAT_RPC
{
	version	STATVERSION_1
	{
		StatReply	StatExecRequest (StatRequest) = STATFUNCTUN;
	} = 1;
} = STATPROG;
