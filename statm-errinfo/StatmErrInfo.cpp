
/****************************************************************/
/*      Header files                                            */
/****************************************************************/

#include <sys/types.h>		// C types
#include <sys/socket.h>		// BSD socket interface
#include <sys/un.h>		// Unix Domain sockets
#include <string.h>		// C strings
#include <stdlib.h>		// standard libraries
#include <unistd.h>		// standard libraries
#include <stdio.h>		// standard I/O
#include <errno.h>		// error numbers
#include <time.h>		// time manipulation functions
#include <syslog.h>		// syslog
#include "StatmErrInfo.h"	// error info functions
#include "StatmErrConst.h"	// error info constants

#include <iostream>	// C++ I/O
#include <sstream>	// string stream
using namespace std;

/****************************************************************/
/*      Global variables for this file                          */
/****************************************************************/

StatmErrInfo	*StatmErrInfo::g_errInfo = new StatmErrInfo ();	//!< error info initializer

/*! @brief create error info object
 *
 *  initializes:
 *
 *  - log level: initialized to SC_ERR. It can be redefined with
 *  environment variable STATM_LOGLEVEL
 *  - syslog socket path which will be used to report log messages:
 *  initialized to "/var/run/sc_print.stat", can be redefined with
 *  environment variable STATM_SYSLOG_SOCKET
 */
StatmErrInfo::StatmErrInfo ()
{
	char	*env;

	m_disabled = false;
	m_logLevel = SC_ERR;
	m_syslogSocketPath = "/var/run/sc_print.stat";
	m_syslogSocket = -1;

	if ((env = getenv ("STATM_LOGLEVEL")) != NULL)
	{
		if (strcmp (env, "error") == 0)
			m_logLevel = SC_ERR;
		else	if (strcmp (env, "warning") == 0)
			m_logLevel = SC_WRN;
		else	if (strcmp (env, "info") == 0)
			m_logLevel = SC_APL;
	}

	if ((env = getenv ("STATM_SYSLOG_SOCKET")) != NULL)
	{
		m_syslogSocketPath = env;
	}

	setvbuf (stdout, 0, _IOLBF, 0);
	setvbuf (stderr, 0, _IOLBF, 0);
}

/*! @brief connect to syslog message reporting system
 *
 */
int	StatmErrInfo::Connect (void)
{
	struct	sockaddr_un	clnaddr;

	Disconnect ();

	if ((m_syslogSocket = socket (AF_LOCAL, SOCK_STREAM, 0)) < 0)
	{
		cerr << "socket() failed, errno = " << errno << endl;
	}
	else
	{
		bzero (&clnaddr, sizeof (clnaddr));
		clnaddr.sun_family = AF_LOCAL;
		strncpy (clnaddr.sun_path, m_syslogSocketPath.c_str(), sizeof (clnaddr.sun_path) -1);
		if (connect (m_syslogSocket, (sockaddr*) &clnaddr, sizeof (clnaddr)) < 0)
		{
			close (m_syslogSocket);
			m_syslogSocket = -1;
			cerr << "connect() failed, errno = " << errno << endl;
		}
	}
	return	m_syslogSocket;
}

/*! @brief disconnect from error reporting system
 *
 */
void	StatmErrInfo::Disconnect (void)
{
	if (m_syslogSocket < 0)
		return;
	close (m_syslogSocket);
	m_syslogSocket = -1;
}

/*! @brief disable error reporting application wide
 *
 */
bool	StatmErrInfo::disable (bool disable)
{
	bool	retcode = m_disabled;
	m_disabled = disable;
	return	retcode;
}

static	const	char*	modulName[] =
{
	"",
	"STATM",
	"CHRG",
	"SDL ",
	"GAD ",
	"SFTP",
};

/*! @brief report error to syslog system
 *
 * @param module module identity
 * @param severity message severity
 * @param format message format
 * @param msg additional message argument
 *
 * @return 0 always
 */
int	StatmErrInfo::ErrReport (int module, int severity, const char* format, va_list msg)
{
	ostringstream	logMessage;
	char	logStr[1024];

	if (m_disabled)
		return	0;

	if (severity > m_logLevel)
		return	0;

	time_t	now = time (NULL);
	struct	tm	ltime;

	localtime_r (&now, &ltime);
	sprintf (logStr, "%04d-%02d-%02d %02d:%02d:%02d %s(%5d) : ",
		1900 + ltime.tm_year, 1 + ltime.tm_mon, ltime.tm_mday,
		ltime.tm_hour, ltime.tm_min, ltime.tm_sec, modulName[module], getpid());

	// beginning of syslog protocol message (HEADER has default values - RFC3164):
	// <facility,severity> = <16,3> or <16,4> or <16,5>, timestamp = NILLVALUE (-), hostname = NILVALUE (-), process = NILVALUE (-)
	// logStr contains generated part of message (date, time and process id)

	switch (severity)
	{
	case	SC_ERR:		// syslog header: facility = local use 0, severity = Error
		logMessage << "<" << (16<<3)+3 << ">- - -: ";
		break;
	case	SC_WRN:		// syslog header: facility = local use 0, severity = Warning
		logMessage << "<" << (16<<3)+4 << ">- - -: ";
		break;
	case	SC_APL:		// syslog header: facility = local use 0, severity = Notice
		logMessage << "<" << (16<<3)+5 << ">- - -: ";
		break;
	default:		// default in statm: syslog header: facility = local use 0, severity = Error
		logMessage << "<" << (16<<3)+3 << ">- - -: ";
		break;
	}

	logMessage << logStr;

	// MSG part of syslog protocol message: append severity

	switch (severity)
	{
	case	SC_ERR:
		logMessage << "(E) ";
		break;
	case	SC_WRN:
		logMessage << "(W) ";
		break;
	case	SC_APL:
		logMessage << "(I) ";
		break;
	default:
		logMessage << "(E) ";
		break;
	}

	// MSG part of syslog protocol message: append application defined message

	vsnprintf (logStr, 1024, format, msg);
	logMessage << logStr << ends;

	basic_string<char>	str = logMessage.str();
	const char	*msgString = str.c_str();

	for (int i = 0; i < 2; ++i)
	{
		if ((m_syslogSocket < 0) && ((m_syslogSocket = Connect()) < 0))
			cerr << "cannot send syslog message: " << msgString << endl;
		else
		{
			if (write (m_syslogSocket, msgString, strlen (msgString) + 1) <= 0)
			{
				cerr << "failed to write syslog message (errno = " << errno << "): " << msgString << endl;
				Disconnect();
				continue;
			}
		}
		break;
	}

#ifdef	STATM_TEST
	switch (m_logLevel)
	{
	case	LOG_ERR:
		if (severity != SC_ERR)
			goto	endlab;
		break;
	case	LOG_WARNING:
		if ((severity != SC_ERR) && (severity != SC_WRN))
			goto	endlab;
		break;
	case	LOG_INFO:
		if ((severity != SC_ERR) && (severity != SC_WRN) && (severity != SC_APL))
			goto	endlab;
		break;
	default:
		if (severity != SC_ERR)
			goto	endlab;
		break;
	}

	cout << msgString + 12 << endl;
endlab:

#endif	// STATM_TEST

	return	0;
}
