/*
 * StatmCommon.h
 *
 *  Created on: 5. nov. 2015
 *      Author: miran
 */

#pragma once

#include "StatmErrApi.h"
#include "StatmMessages.h"
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <map>
using namespace std;

namespace statm_daemon
{

#define	STATM_SEM_KEY	0x00001000

const	unsigned int	g_localListener = 3;	// worker's unix domain listener
const	unsigned int	g_localCtlWriter = 4;	// worker's unix domain master client
const	unsigned int	g_localWrkReader = 4;	// worker's unix domain master client
const	unsigned int	g_localCtlReader = 5;	// worker's unix domain master client
const	unsigned int	g_localWrkWriter = 5;	// worker's unix domain master client
const	bool	g_debug = false;	// display debug info?
const	unsigned int	g_threadCount = 4;	// minimal number of working threads

#define	_STR2(line)	#line
#define	_STR(line)	_STR2(line)
#define	err_info(fmt)	__FILE__ ":" _STR(__LINE__) ": " fmt

typedef	enum
{
	StatmFirstWord,
	StatmAdminWord,
	StatmAdminStartWord,
	StatmAdminRestartWord,
	StatmAdminStopWord,
	StatmAdminStatusWord,
	StatmWorkerWord,
	StatmLastWord,
}	StatmWordCode;

class	wordcmp : public binary_function < const char*, const char*, bool >
{
public:
	bool operator () (const first_argument_type& x, const first_argument_type& y) const
	{
		return	strcasecmp (x, y) < 0;
	}
};

class	StatmCommon
{
private:
	typedef map < const char*, StatmWordCode, wordcmp >	wordmap;
	StatmCommon ()
	{
		m_wordmap["admin"] = StatmAdminWord;
		m_wordmap["start"] = StatmAdminStartWord;
		m_wordmap["restart"] = StatmAdminRestartWord;
		m_wordmap["stop"] = StatmAdminStopWord;
		m_wordmap["status"] = StatmAdminStatusWord;
		m_wordmap["worker"] = StatmWorkerWord;

		m_entity = StatNoEntity;
		m_pid = getpid ();

		m_syslogSocketPath = "/var/run/sc_print.statm";
		m_localSocketPath = "/var/run/statm.socket";
		m_workingQueueKeyPath = "/var/run/statm.mq";

		char*	env;
		if ((env = getenv ("STATM_SYSLOG_SOCKET")) != 0)
		{
			m_syslogSocketPath = env;
		}
		if ((env = getenv ("STATM_LOCAL_SOCKET")) != 0)
		{
			m_localSocketPath = env;
		}
		if ((env = getenv ("STATM_WORKING_QUEUE_KEY_PATH")) != 0)
		{
			m_workingQueueKeyPath = env;
		}
	}
	~StatmCommon () {}
public:
	inline static StatmWordCode	findWord (const char* word) { return g_commonInitializer->_findWord (word); }
	inline static StatEntity	entity () { return g_commonInitializer->_entity(); }
	inline static void entity (StatEntity entity) { g_commonInitializer->_entity (entity); }
	inline static pid_t	pid () { return g_commonInitializer->_pid(); }
	inline static string	syslogSocketPath () { return g_commonInitializer->_syslogSocketPath (); }
	inline static string	localSocketPath () { return g_commonInitializer->_localSocketPath (); }
	inline static string	workingQueueKeyPath () { return g_commonInitializer->_workingQueueKeyPath (); }
private:
	inline	StatmWordCode	_findWord (const char* word)
	{
		return (m_wordmap.find (word) == m_wordmap.end ()) ? StatmFirstWord : m_wordmap [word];
	}
	inline StatEntity	_entity () { return m_entity; }
	inline void _entity (StatEntity entity) { m_entity = entity; }
	inline pid_t	_pid () { return m_pid; }
	inline string	_syslogSocketPath () { return m_syslogSocketPath; }
	inline string	_localSocketPath () { return m_localSocketPath; }
	inline string	_workingQueueKeyPath () { return m_workingQueueKeyPath; }
private:
	static	StatmCommon*	g_commonInitializer;
	wordmap	m_wordmap;
	StatEntity	m_entity;
	pid_t	m_pid;
	string	m_syslogSocketPath;
	string	m_localSocketPath;
	string	m_workingQueueKeyPath;
};

}
