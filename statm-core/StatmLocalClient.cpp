/*
 * StatmLocalClient.cpp
 *
 *  Created on: 30. okt. 2015
 *      Author: miran
 */

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "StatmLocalClient.h"
#include "StatmClientDriver.h"
#include "StatmWorker.h"
#include "StatmMessages.h"

#include <sstream>
using namespace std;

namespace statm_daemon
{

const char*	StatmLocalClient::g_errorString[] =
{
		/*	0	*/	"no error",
		/*	1	*/	"not connected",
		/*	2	*/	"already connected",
		/*	3	*/	"unknown sql request",
};
int StatmLocalClient::g_reqIndex = 0;

/*! @brief create an instance class representing UNIX domain client connection with XML DB server
 *
 *  initialize internal I/O buffers (initialy empty) and creates I/O handler for XML DB
 *  server connection FD by registering HandleLocalClient() as I/O handler for that FD.
 *  It also initializes its superclass interface StatmWorkingClient
 *
 *  @param ctx reference to I/O multiplexer which will handle its I/O activity
 *  @param fd UNIX domain socket FD connecting this client with XML DB server
 *  @param mainThread local client driver thread reference (it should imply I/O multiplexer)
 *
 */
StatmLocalClient::StatmLocalClient(StatmRunningContext* ctx, int fd, StatmRunnable* mainThread) : StatmWorkingClient (mainThread)
{
#if defined(DEBUG)
	if (g_debug) statmErrReport (SC_STATM, SC_APL, "CREATE StatmLocalClient %ld", this);
#endif
	m_ctx = ctx;
	m_fd = fd;

	m_connected = false;

	m_localHandler = ctx->RegisterDescriptor (EPOLLIN, fd, HandleLocalClient, this, ctx_info);

	int	dummy = 0;
	m_intSize = xdr_sizeof ((xdrproc_t)xdr_int, &dummy);

	m_inputBuffer = 0;
	m_inputPtr = 0;
	m_inputEnd = 0;

	m_outputBuffer = 0;
	m_outputPtr = 0;
	m_outputEnd = 0;

	m_errorList = 0;
}

/*! @brief destroy all allocated resources
 *
 */
StatmLocalClient::~StatmLocalClient()
{
#if defined(DEBUG)
	if (g_debug) statmErrReport (SC_STATM, SC_APL, "DELETE StatmLocalClient %ld", this);
#endif
	Release ();
}

/*! @brief destroy all allocated resources
 *
 *  release all allocated resources: I/O handler, I/O internal buffers,
 *  allocated shared memory objects and list of error reporting objects
 *
 */
void StatmLocalClient::Release ()
{
	if (m_localHandler != 0)
		m_ctx->RemoveDescriptor(m_localHandler);
	m_localHandler = 0;

	if (m_fd > 0)
		close (m_fd);
	m_fd = 0;

	if (m_inputBuffer != 0)
		free (m_inputBuffer);
	m_inputBuffer = 0;
	m_inputPtr = 0;
	m_inputEnd = 0;

	if (m_outputBuffer != 0)
		free (m_outputBuffer);
	m_outputBuffer = 0;
	m_outputPtr = 0;
	m_outputEnd = 0;

	if (m_errorList != 0)
		xdr_free ((xdrproc_t) xdr_StatError, (char*) m_errorList);

	((StatmClientDriver*)mainThread())->ReleaseLocalClient(this);
}

/*! @brief I/O handler for UNIX domain client
 *
 *  function processes input, output and exception processing for
 *  UNIX domain client I/O activity
 *
 *  Input processing: allocates sufficient space in input buffer for all
 *  incoming messages, deserializes them and processes them one by one
 *  in incoming order
 *
 *  Output processing: send as many posted requests as possible and
 *  rearrange output buffer appropriately
 *
 *  Exception processing: release I/O resources causing client to cease
 *
 *  All errors in steps implicitly or explicitly appearing in above
 *  statements have fatal consequences: I/O resources are released
 *  causing client to cease
 *
 *  @param ctx I/O multiplexer reference (provided by constructor)
 *  @param flags I/O processing type (input - EPOLLIN, output - EPOLLOUT, exception - other)
 *  @param handler handler reference associated with this function
 *  @param fd UNIX domain socket FD for whom this handler was registered
 *
 */
void	StatmLocalClient::HandleLocalClient (StatmRunningContext *ctx, uint flags, ctx_fddes_t handler, int fd)
{
	if (flags & EPOLLIN)
	{
		int	needSpace = 0;
		int	usedSpace = m_inputPtr - m_inputBuffer;
		int	freeSpace = m_inputEnd - m_inputPtr;

		if (ioctl (fd, FIONREAD, &needSpace) < 0)
		{
			statmErrReport (SC_STATM, SC_ERR, err_info("cannot receive client request, ioctl() failed, errno = %d"), errno);
			delete this;
			return;
		}

		if (needSpace == 0)
		{
			statmErrReport (SC_STATM, SC_APL, err_info("cannot receive client request, no more data"));
			delete this;
			return;
		}

		if (freeSpace < needSpace)
		{
			needSpace += usedSpace;
			needSpace >>= 10;
			needSpace++;
			needSpace <<= 10;
			u_char*	buffer = (u_char*) malloc (needSpace);
			if (buffer == 0)
			{
				statmErrReport (SC_STATM, SC_ERR, err_info("cannot receive client request, malloc() failed, errno = %d"), errno);
				delete this;
				return;
			}
			if (m_inputBuffer != 0)
			{
				memcpy (buffer, m_inputBuffer, usedSpace);
				free (m_inputBuffer);
			}
			m_inputBuffer = buffer;
			m_inputPtr = buffer + usedSpace;
			m_inputEnd = buffer + needSpace;
			freeSpace = needSpace - usedSpace;
		}

		int	recvSize;
		if ((recvSize = recv (fd, m_inputPtr, freeSpace, 0)) <= 0)
		{
			if (errno == EWOULDBLOCK)
				return;
			statmErrReport (SC_STATM, SC_ERR, err_info("cannot receive client request, recv() failed, errno = %d"), errno);
			delete this;
			return;
		}

		usedSpace += recvSize;

		u_char*	ptr = m_inputBuffer;
		while (true)
		{
			if (usedSpace < (int) m_intSize)
				break;

			XDR	xdr;

			int	msgSize;
			xdrmem_create(&xdr, (char*) ptr, m_intSize, XDR_DECODE);
			if (xdr_int (&xdr, &msgSize) != TRUE)
			{
				statmErrReport (SC_STATM, SC_ERR, err_info("cannot decode client message size"));
				xdr_destroy (&xdr);
				return;
			}
			xdr_destroy (&xdr);

			if (usedSpace < msgSize + (int) m_intSize)
				break;

			StatRequest*	req = new StatRequest;
			memset (req, 0, sizeof (StatRequest));
			xdrmem_create(&xdr, (char*) (ptr + m_intSize), msgSize, XDR_DECODE);
			if (xdr_StatRequest(&xdr, req) != TRUE)
			{
				statmErrReport (SC_STATM, SC_ERR, err_info("cannot decode client message"));
				xdr_destroy (&xdr);
				return;
			}
			xdr_destroy (&xdr);

			ptr += msgSize + m_intSize;
			usedSpace -= msgSize + m_intSize;
			if (usedSpace > 0)
				memcpy (m_inputBuffer, ptr, usedSpace);
			m_inputPtr = m_inputBuffer + usedSpace;

			DisplayStatRequest(req);

			switch (req->m_requestType)
			{
			case	AdminRequestCode:
				{
					((StatmClientDriver*)mainThread())->HandleAdminRequest (this, req);
					((StatmClientDriver*)mainThread())->ReleaseLocalClient (this);
					xdr_free ((xdrproc_t) xdr_StatRequest, (char*) req);
					delete	req;
					delete	this;
				}
				break;
			default:
				statmErrReport (SC_STATM, SC_ERR, err_info("unknown client message type = %d"), req->m_requestType);
				xdr_free ((xdrproc_t) xdr_StatRequest, (char*) req);
				delete	req;
				delete this;
				break;
			}
		}
	}
	else	if (flags & EPOLLOUT)
	{
		size_t	size = m_outputPtr - m_outputBuffer;
		ssize_t	count = send (fd, m_outputBuffer, size, 0);
		if (count <= 0)
		{
			statmErrReport (SC_STATM, SC_ERR, err_info("cannot send client reply, send() failed, errno = %d"), errno);
			delete this;
			return;
		}
		if ((size_t)count < size)
			memcpy (m_outputBuffer, m_outputBuffer + count, size - count);
		else
			ctx->DisableDescriptor(handler, EPOLLOUT);
		m_outputPtr -= count;
	}
	else
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("cannot determine client I/O type = %d"), flags);
		Release ();
//		delete this;
	}
}

/*! @param create message handler
 *
 *  create common message header: version number, message index, UUID,
 *  message type and PID of sending process
 *
 *  @param header reference of message header
 *
 */
static	void	CreateMessageHeader (StatMessageHeader* header)
{
	static	int	g_messageIndex = 0;

	header->m_version.m_major = 1;
	header->m_version.m_minor = 0;
	header->m_index = ++g_messageIndex;
	uuid_generate (header->m_hash);
	header->m_error = 0;
	header->m_entity = StatmCommon::entity();
	header->m_pid = StatmCommon::pid();
}

/*! @brief form error response message
 *
 *  prepare error message which will be appended to SQL reply
 *
 *  @param errorType error type
 *  @param errorCode error code
 *  @param errorMessage detailed error message text
 *
 */
void StatmLocalClient::PostSqlErrorResponse (StatErrorType errorType, StatErrorCode errorCode, const char* errorMessage)
{
	if (g_debug == true)
		statmErrReport (SC_STATM, SC_ERR, err_info("ERROR: errorType = %d, errorCode = %d, errorMessage = %s"), errorType, errorCode, ((errorMessage == 0) ? "*** NULL ***" : errorMessage));
	StatError*	error = new StatError;
	error->m_type = errorType;
	error->m_errorCode = errorCode;
	error->m_errorMessage = strdup ((char*) errorMessage);
	error->m_next = m_errorList;
	m_errorList = error;
}

/*! @brief display protocol request
 *
 *  if debugging is enabled (g_debug == true) display all
 *  components of XML DB protocol request message
 *
 *  @param req request reference
 *
 */
void StatmLocalClient::DisplayStatRequest (StatRequest* req)
{
	if (g_debug == false)
		return;

	stringstream	str;
	str << "STATM REQUEST" << endl;
	str << "{" << endl;
	switch (req->m_requestType)
	{
	case	AdminRequestCode:
		{
			str << "\tindex = " << req->StatRequest_u.m_adminRequest.m_header.m_index << endl;
			str << "\ttype = admin" << endl;
			str << "\trequest = ";
			switch (req->StatRequest_u.m_adminRequest.m_command)
			{
			case	StatmAdminStartWord:
				str << "start";
				break;
			case	StatmAdminRestartWord:
				str << "restart";
				break;
			case	StatmAdminStopWord:
				str << "stop";
				break;
			case	StatmAdminStatusWord:
				str << "status";
				break;
			default:
				str << "unknown";
				break;
			}
			str << endl;
		}
		break;
	default:
		str << "\ttype = unknown" << endl;
		break;
	}
	str << "}" << endl;
	str << ends;

	const char*	msg = str.str().c_str();
	statmErrReport(SC_STATM, SC_ERR, msg);
}

/*! @brief display protocol reply
 *
 *  if debugging is enabled (g_debug == true) display all
 *  components of XML DB protocol reply message
 *
 *  @param req request reference
 *
 */
void StatmLocalClient::DisplayStatReply (StatReply* rpl)
{
	if (g_debug == false)
		return;

	stringstream	str;
	str << "STATM REPLY" << endl;
	str << "{" << endl;

	str << "\ttype = ";
	switch (rpl->m_replyType)
	{
	case	AdminReplyCode:
		str << "admin";
		break;
	default:
		str << "unknown";
		break;
	}
	str << endl;
	str << "}" << endl;
	str << ends;

	const char*	msg = str.str().c_str();
	statmErrReport(SC_STATM, SC_ERR, msg);
}

/*! @brief XML DB client activation
 *
 *  main thread of working entity accepts client connections, selects
 *  one of client driver threads and sends it 'client activation' message
 *  consisting mainly of client connection FD. After this message has been
 *  processed by actual client driver thread, connection is declared
 *  'activated'. After activation all client server communication is
 *  processed by I/O multiplexer residing in client driver thread.
 *
 *  @param ctx I/O multiplexer reference
 *
 */
int	StatmLocalClientActivationMessage::Invoke (StatmRunningContext* ctx)
{
	((StatmClientDriver*)m_clientDriver)->ActivateLocalClientHandler(m_localClientFd);
	return	0;
}

/*! @brief post client response messages
 *
 *  all client responses are actually generated as a result of SQL jobs
 *  processed by working threads. Since client communication is governed
 *  by client driver threads these responses must also be communicated by
 *  them and not by working threads where they are actually created. This
 *  function does this job. It posts reply messages generated by working
 *  threads into internal output buffers of I/O multiplexer processing
 *  client communication
 *
 *  @param ctx I/O multiplexer reference
 *
 */
int	StatmLocalClientResponseMessage::Invoke (StatmRunningContext* ctx)
{
	return	0;
}

} /* namespace statm_daemon */
