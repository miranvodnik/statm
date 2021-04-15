
#pragma once

/****************************************************************/
/*          UNIX - SCCS  VERSION DESCRIPTION                   	*/
/****************************************************************/
/*static char  unixid_QGNJ[] = "%W%	%D%	GeneralAdapter.h";*/

/****************************************************************/
/*      Header files                                            */
/****************************************************************/

#include <stdlib.h>
#include "StatisticalAdapter.h"

/****************************************************************/
/*      External functions                                      */
/****************************************************************/

class StatisticalCounterInfo
{
public:
	StatisticalCounterInfo (u_int statGroup, u_int statCounter, char* description, char* statCounterName, char*imsElement, char* comment) :
		m_statGroup (statGroup), m_statCounter (statCounter), m_description (description), m_statCounterName (statCounterName), m_imsElement (imsElement), m_comment (comment) {}
public:
	inline u_int	statGroup () { return m_statGroup; }
	inline u_int	statCounter () { return m_statCounter; }
	inline const char*	description () { return m_description.c_str(); }
	inline const char*	statCounterName () { return m_statCounterName.c_str(); }
	inline const char*	imsElement () { return m_imsElement.c_str(); }
	inline const char*	comment () { return m_comment.c_str(); }
private:
	u_int	m_statGroup;
	u_int	m_statCounter;
	string	m_description;
	string	m_statCounterName;
	string	m_imsElement;
	string	m_comment;
};

typedef	pair < u_int, u_int >	cntrkey;

class cmpstatcntr : public less < cntrkey >
{
public:
	bool operator () (const cntrkey& x, const cntrkey& y)
	{
		return	(x.first < y.first) || ((x.first == y.first) && (x.second < y.second));
	}
};

class StatisticalAdapterThread
{
public:
	typedef map < cntrkey, StatisticalCounterInfo*, cmpstatcntr >	cntrset;
	typedef map < unsigned short, StatisticalAdapter* >	salist;
	typedef map < unsigned short, StatisticalAdapterArg* >	arglist;
	StatisticalAdapterThread () {}
	virtual ~StatisticalAdapterThread () {}


public:
	inline static void Init () { g_StatisticalAdapterThread->_Init (); }
	inline static void Release () { g_StatisticalAdapterThread->_Release (); }
	inline static const char* FetchCounterName (cntrkey key) { return g_StatisticalAdapterThread->_FetchCounterName (key); }
	inline static const char* FetchCounterDescription (cntrkey key) { return g_StatisticalAdapterThread->_FetchCounterDescription (key); }
	inline static const char* FetchCounterIms (cntrkey key) { return g_StatisticalAdapterThread->_FetchCounterIms (key); }
	inline static const char* FetchCounterComment (cntrkey key) { return g_StatisticalAdapterThread->_FetchCounterComment (key); }
	inline static salist& get_salist () { return g_StatisticalAdapterThread->_get_salist (); }
	inline static string get_xmlCfgFileName () { return m_xmlCfgFileName; }

private:
	void _Init ();
	void _Release ();
	int ReadDBSettings();
	int ReadFSSettings();
	int ReadXMLSettings ();
	int	ReadStatCountersInfo ();
	const char* _FetchCounterName (cntrkey key);
	const char* _FetchCounterDescription (cntrkey key);
	const char* _FetchCounterIms (cntrkey key);
	const char* _FetchCounterComment (cntrkey key);
	inline salist& _get_salist () { return m_salist; }

private:
	static StatisticalAdapterThread* g_StatisticalAdapterThread;
	cntrset	m_cntrset;
	salist m_salist;

	static string m_xmlCfgFileName;
};

typedef StatisticalAdapterThread::salist salist;
