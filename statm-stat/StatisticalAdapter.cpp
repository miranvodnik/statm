
/****************************************************************/
/*      Header files                                            */
/****************************************************************/

#include <sys/types.h>		// types declarations
#include <sys/stat.h>		// file modes
#include <sys/mman.h>		// file mapping API
#include <sys/ipc.h>		// file mapping API
#include <sys/sem.h>		// file mapping API
#include <dirent.h>		// directory entries
#include <regex.h>		// regular expressions
#include <grp.h>		// group info
#include <pwd.h>		// user info
#include <unistd.h>		// standard library
#include <sqlxdbd.h>
#include <sqlext.h>

#include "StatisticalAdapter.h"	// general adapter
#include "StatisticalAdapterThread.h"	// general adapter
#include "StatmErrApi.h"
#include "StatmSemval.h"

static	const char*	cscfTypeName[] =
{
	"P-CSCF",
	"I-CSCF",
	"S-CSCF",
	"TAS",
	"MRC",
	"AGCF",
	"MGCF",
	"BGCF",
	"NonImsAGCF",
	"E-CSCF",
	"IMS_iAGCF",
	"ImsIAD",
	"SCCAS",
	"ImobGW",
	"RGCF",
	"TAGCF",
};

/****************************************************************/
/*      Private functions                                       */
/****************************************************************/

static	int	CheckPath (const char *, const char *);

/****************************************************************/
/* Function:    CheckPath ()                                    */
/* In:          path - directory path                           */
/*              delimiter - possible path delimiters            */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      0 - path created                                */
/*              -1 - cannot create directory path               */
/* Description: function creates missing segment of specified   */
/*              path. Relative paths are created relative to the*/
/*              current working directory                       */
/****************************************************************/

static	int	CheckPath (const char *path, const char *delimiter)
{
	char	pcwd[PATH_MAX+1];
	char	*p, *q;

	string	dname;

	if (*path != '/')
		dname = ".";

	umask (0);

	strncpy (pcwd, path, PATH_MAX);
	for (p = pcwd; NULL != (q = strsep (&p, delimiter)); )
	{
		char	*dn;
		struct	stat	sb;

		dname += '/';
		dname += q;

		if (stat ((dn = (char*) dname.c_str()), &sb) < 0)
		{
			statmErrReport (SC_STATM, SC_APL, "Try to check path '%s'", dn);
			if (errno != ENOENT)
			{
				stringstream	str;
				const char	*msg;

				str << "Check Path: stat (" << dn << ") failed, errno = " << errno << ends;
				msg = str.str().c_str();
				statmErrReport (SC_STATM, SC_ERR, msg);
				return	-1;
			}

			if (mkdir (dn, 0777) < 0)
			{
				stringstream	str;
				const char	*msg;

				str << "Check Path: path '" << dn << "' does not exist" << ends;
				msg = str.str().c_str();
				statmErrReport (SC_STATM, SC_ERR, msg);
				return	-1;
			}
			statmErrReport (SC_STATM, SC_APL, "Path checked '%s'", dn);
		}
		else	if (!S_ISDIR (sb.st_mode))
		{
			stringstream	str;
			const char	*msg;

			str << "Check Path: path (" << dn
				<< ") exists, but is not dir, stat mode = ("
				<< sb.st_mode << ")" << ends;
			msg = str.str().c_str();
			statmErrReport (SC_STATM, SC_ERR, msg);
			return	-1;
		}
	}

	return	0;
}

StatisticalAdapter::StatisticalAdapter(unsigned short adapterId, const char* statDirName, const char* destDirName, const char* owner, const char* group, const char* dperm, const char* fperm)
	: m_adapterId (adapterId), m_statDirName (statDirName), m_dstDirName (destDirName), m_owner (owner), m_group (group), m_dperm (dperm), m_fperm (fperm)
{
        union semval sv;
        sv.value = 1;
	char*	env;

	m_fileIndex = 1;
	m_sync = semget (IPC_PRIVATE, 1, IPC_CREAT);
	semctl (m_sync, 0, SETVAL, sv);

	m_notifyFD = 0;
	m_owid = (uid_t) -1;
	m_grid = (gid_t) -1;
	m_dperms = 0775;
	m_fperms = 0660;

	m_preserveHistory = false;
	if ((env = getenv ("STATM_PRESERVE_STAT_HISTORY")) != 0)
	{
		m_preserveHistory = (strcasecmp (env, "true") == 0);
	}
	m_generateXml = true;
	if ((env = getenv ("STATM_GENERATE_STAT_XML")) != 0)
	{
		m_generateXml = (strcasecmp (env, "false") != 0);
	}

	m_link = 0;
	_Init ();
}

/****************************************************************/
/* Destructor:  ~StatisticalAdapter                                */
/* Description: does nothing                                    */
/****************************************************************/
StatisticalAdapter::~StatisticalAdapter()
{
        union semval sv;
        sv.value = 0;
	m_notifyFD = 0;
	semctl (m_sync, 0, IPC_RMID, sv);
	delete m_link;
}

void StatisticalAdapter::lock ()
{
	struct	sembuf	sb;

	sb.sem_num = 0;
	sb.sem_op = -1;
	sb.sem_flg = SEM_UNDO;
	semop (m_sync, &sb, 1);
}

void StatisticalAdapter::unlock ()
{
	struct	sembuf	sb;

	sb.sem_num = 0;
	sb.sem_op = 1;
	sb.sem_flg = SEM_UNDO;
	semop (m_sync, &sb, 1);
}

/****************************************************************/
/* Function:    StatisticalAdapter::Init ()                        */
/* In:          /                                               */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: initializes general adapter according to the    */
/*              data given by invocation environment            */
/****************************************************************/

void	StatisticalAdapter::_Init (void)
{
	if (!m_group.empty())
	{
		struct	group	*grp = getgrnam ((char*) m_group.c_str());
		if (grp == NULL)
		{
			statmErrReport (SC_STATM, SC_ERR, "Cannot determine group '%s' info", m_group.c_str());
		}
		else
		{
			m_grid = grp->gr_gid;
		}
	}
	else	if (!m_dstDirName.empty())
	{
		struct	stat	sb;
		if (stat (m_dstDirName.c_str(), &sb) < 0)
		{
			statmErrReport (SC_STATM, SC_ERR, "Cannot stat '%s', errno = %d", m_dstDirName.c_str(), errno);
		}
		else
		{
			char	str[32];
			m_grid = sb.st_gid;
			sprintf (str, "%d", m_grid);
			m_group = str;
		}
	}
	if (!m_owner.empty())
	{
		struct	passwd	*pwd = getpwnam ((char*) m_owner.c_str());
		if (pwd == NULL)
		{
			statmErrReport (SC_STATM, SC_ERR, "Cannot determine owner '%s' info", m_owner.c_str());
		}
		else
		{
			m_owid = pwd->pw_uid;
		}
	}
	else	if (!m_dstDirName.empty())
	{
		struct	stat	sb;
		if (stat (m_dstDirName.c_str(), &sb) < 0)
		{
			statmErrReport (SC_STATM, SC_ERR, "Cannot stat '%s', errno = %d", m_dstDirName.c_str(), errno);
		}
		else
		{
			char	str[32];
			m_owid = sb.st_uid;
			sprintf (str, "%d", m_owid);
			m_owner = str;
		}
	}
	if (!m_dperm.empty())
	{
		char	*endptr = NULL;
		m_dperms = strtol ((char*) m_dperm.c_str(), &endptr, 8);
		if (endptr == NULL || *endptr != '\0')
		{
			statmErrReport (SC_STATM, SC_ERR, "Cannot determine directory permissions '%s' info", m_dperm.c_str());
			m_dperms = 0775;
		}
	}
	if (!m_fperm.empty())
	{
		char	*endptr = NULL;
		m_fperms = strtol ((char*) m_fperm.c_str(), &endptr, 8);
		if (endptr == NULL || *endptr != '\0')
		{
			statmErrReport (SC_STATM, SC_ERR, "Cannot determine file permissions '%s' info", m_dperm.c_str());
			m_fperms = 0660;
		}
	}
	if (!m_statDirName.empty())
	{
		CheckPath (m_statDirName.c_str(), "/\\");
	}
	if (!m_dstDirName.empty() && (CheckPath (m_dstDirName.c_str(), "/\\") == 0))
	{
		if (chown ((char*) m_dstDirName.c_str(), m_owid, m_grid) < 0)
		{
			const char	*owner = "UNSPECIFIED_OWNER";
			const char	*group = "UNSPECIFIED_GROUP";

			if (!m_owner.empty())
				owner = (char*) m_owner.c_str();
			if (!m_group.empty())
				group = (char*) m_group.c_str();
			statmErrReport (SC_STATM, SC_ERR, "chown (%s, %s, %s) failed, errno = %d", m_dstDirName.c_str(), owner, group, errno);
		}
		if (chmod ((char*) m_dstDirName.c_str(), m_dperms) < 0)
		{
			statmErrReport (SC_STATM, SC_ERR, "chmod (%s, %d) failed, errno = %d", m_dstDirName.c_str(), m_dperms, errno);
		}
	}
}

bool StatisticalAdapter::SetArg (StatisticalAdapterArg* p_arg)
{
	bool result = true;
	string argName = p_arg->argName();
	string argValue = p_arg->argValue();

	if (argName == "PreserveStatHistory")
	{
		m_preserveHistory = (strcasecmp (argValue.c_str(), "true") == 0);
		result = true;
	}
	else if (argName == "GenerateStatXML")
	{
		m_generateXml = (strcasecmp (argValue.c_str(), "true") == 0);
		result = true;
	}
	else
		result = false;

	return result;
}

/****************************************************************/
/* Function:    StatisticalAdapter::Collect ()                     */
/* In:          immediate - immediatelly collecting?            */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: function collects general adapter data after    */
/*              being requested                                 */
/****************************************************************/

void	StatisticalAdapter::_Collect (bool immediate, time_t t)
{
	DIR*	dir;

	statmErrReport (SC_STATM, SC_APL, "Collect (%d)", immediate);

	if ((dir = opendir (m_statDirName.c_str())) == 0)
	{
		statmErrReport (SC_STATM, SC_ERR, "Collect: cannot open dir, name = '%s'", m_statDirName.c_str());
		return;
	}

	struct dirent	entry;
	struct dirent*	rentry = 0;

	while (readdir_r (dir, &entry, &rentry) == 0)
	{
		if (rentry == 0)
			break;
		string	srcf = m_statDirName + '/' + entry.d_name;
		struct stat	sb;
		if (stat (srcf.c_str(), &sb) < 0)
			continue;
		if (S_ISREG (sb.st_mode) == 0)
			continue;
		_DoCollect (entry.d_name, 0, t);
	}

	closedir (dir);
}

/****************************************************************/
/* Function:    StatisticalAdapter::DoCollect ()                   */
/* In:          ind - invoke callback or not                    */
/* Out:         /                                               */
/* In/Out:      /                                               */
/* Return:      /                                               */
/* Description: Unconditionally collect general adapter data    */
/****************************************************************/

void	StatisticalAdapter::_DoCollect (const char* fileName, int depth, time_t t)
{
	if (this == 0)
		return;

	int	i = 0;
	unsigned long	index = m_fileIndex;
	const char* pathName = m_statDirName.c_str();

	statmErrReport (SC_STATM, SC_ERR, "DoCollect ()");
	string	fnam;
	string	tfnam;
	string	dstf;

	string	srcf = m_statDirName + '/' + fileName;

	if ((m_generateXml == false) || (fileName == 0))
	{
		statmErrReport (SC_STATM, SC_ERR,
				"Cannot collect statistical data, generate = %d, path = %s, name = %s",
				m_generateXml, pathName, (fileName == 0) ? "NULL" : fileName);
	}
	else
	{
		while (1)
		{
			if (access (srcf.c_str (), F_OK) == 0)
				break;
			if (errno == EINTR)
				continue;
			statmErrReport (SC_STATM, SC_ERR, "Cannot access %s/%s, errno = %s", pathName, fileName, strerror (errno));
			return;
		}

		while (1)
		{
			dstf = m_dstDirName + '/' + fileName + ".xml";

			if (access (dstf.c_str (), F_OK) != 0)
				break;
			statmErrReport (SC_STATM, SC_ERR, "XML file = %s already exists", dstf.c_str ());
			return;
		}

		CollectStatData (srcf, dstf);

		for (i = 0; i < 10; ++i)
		{
			if (chown ((char*) dstf.c_str (), m_owid, m_grid) < 0)
			{
				const char	*owner = "UNSPECIFIED_OWNER";
				const char	*group = "UNSPECIFIED_GROUP";

				if (!m_owner.empty ())
					owner = (char*) m_owner.c_str ();
				if (!m_group.empty ())
					group = (char*) m_group.c_str ();
				statmErrReport (SC_STATM, SC_ERR, "chown (%s, %s, %s) failed, errno = %d", dstf.c_str (), owner, group, errno);
			}
			if (chmod ((char*) dstf.c_str (), m_fperms) < 0)
			{
				statmErrReport (SC_STATM, SC_ERR, "chmod (%s, %d) failed, errno = %d", dstf.c_str (), m_fperms, errno);
			}
			break;
		}

		if (i >= 10)
		{
			m_fileIndex = index;
			statmErrReport (SC_STATM, SC_ERR, "Cannot rename cummulative data file (%s/%s to %s)", pathName, fileName, fnam.c_str ());
		}
	}

	m_link->_DoCollect(fileName, depth + 1, t);

	if ((depth == 0) && (m_preserveHistory == false) && (unlink (srcf.c_str()) != 0))
			statmErrReport (SC_STATM, SC_ERR, "cannot unlink file '%s', reason: %s", srcf.c_str(), strerror (errno));
}

static	int	readInt (const u_char* ptr)
{
	int	result = 0;
	for (int i = 0; i < 4; ++i)
	{
		result <<= 8;
		result += ptr[i];
	}
	return	result;
}

static	int	readCharInt (const char* ptr, int len)
{
	int	result = 0;
	for (; len > 0; --len)
	{
		result *= 10;
		result += *ptr++ - '0';
	}
	return	result;
}

int	StatisticalAdapter::GetImsCscfType (int nodeid)
{
	static	SQLHENV		henv = SQL_NULL_HANDLE;
	static	SQLHDBC		hdbc = SQL_NULL_HANDLE;
	static	SQLHSTMT	hstm = SQL_NULL_HANDLE;
	static	long		ims_cscf_type = 0;			/* IMS CSCF Type	*/
	static	int	nid;
	RETCODE	r;

	return 0;

	if (henv == SQL_NULL_HANDLE)
	{
		statmErrReport (SC_STATM, SC_ERR, "HENV = SQL_NULL_HANDLE");
		if ((r = SQLAllocEnv (&henv)) != SQL_SUCCESS)
			return	-1;
	}

	if (hdbc == SQL_NULL_HANDLE)
	{
		statmErrReport (SC_STATM, SC_ERR, "HDBC = SQL_NULL_HANDLE");
		if ((r = SQLAllocConnect (henv, &hdbc)) != SQL_SUCCESS)
			return	-1;

		char*	dbName = getenv ("STATM_SOLID_CONNECTION_STRING");
		char*	dbUser = getenv ("STATM_SOLID_DB_USERNAME");
		char*	dbPassword = getenv ("STATM_SOLID_DB_PASSWORD");
		char*	dbCatalog = getenv ("STATM_SOLID_DB_CATALOG");

		if ((dbName == 0) || (dbUser == 0) || (dbPassword == 0) || (dbCatalog == 0))
			return	-1;

		if ((r = SQLConnect (hdbc,
				(SQLCHAR *) dbName,
				SQL_NTS,
				(SQLCHAR *) dbUser,
				SQL_NTS,
				(SQLCHAR *) dbPassword,
				SQL_NTS )) != SQL_SUCCESS)
			return	-1;
	}

	nid = nodeid;
	char* ssql = "SELECT ims_cscf_type FROM ims_global WHERE nodeid = ?";
	while (hstm == SQL_NULL_HANDLE)
	{
		statmErrReport (SC_STATM, SC_ERR, "HSTM = SQL_NULL_HANDLE");
		if ((r = SQLAllocStmt (hdbc, &hstm)) != SQL_SUCCESS)
		{
			hstm = SQL_NULL_HANDLE;
			break;
		}
		if ((r = SQLPrepare (hstm, (SQLCHAR *)ssql, SQL_NTS)) != SQL_SUCCESS)
		{
			SQLFreeStmt (hstm, 0);
			hstm = SQL_NULL_HANDLE;
			break;
		}
		if ((r = SQLBindParameter (hstm, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &nid, 0, NULL)) != SQL_SUCCESS)
		{
			SQLFreeStmt (hstm, 0);
			hstm = SQL_NULL_HANDLE;
			break;
		}
		if ((r = SQLBindCol (hstm, 1, SQL_C_SLONG, &ims_cscf_type, sizeof(ims_cscf_type), NULL)) != SQL_SUCCESS)
		{
			SQLFreeStmt (hstm, 0);
			hstm = SQL_NULL_HANDLE;
			break;
		}
		break;
	}

	if ((hstm == SQL_NULL_HANDLE) ||
		((r = SQLExecute (hstm)) != SQL_SUCCESS) ||
		((r = SQLFetch(hstm)) != SQL_SUCCESS))
	{
		ims_cscf_type = 0;
	}
	return	ims_cscf_type;
}

void StatisticalAdapter::CollectStatData (string srcFileName, string dstFileName)
{
	u_char	field [4 * 33];
	u_char	gname [4 * 7];
	FILE*	src;
	FILE*	dst;
	size_t	size;

	statmErrReport (SC_STATM, SC_ERR, "CollectStatData (%s, %s)", srcFileName.c_str(), dstFileName.c_str());
	const char*	srcFName = srcFileName.c_str();
	if ((size = strlen (srcFName)) < 12)
	{
		statmErrReport (SC_STATM, SC_ERR, "cannot deduce statistical measurement time from '%s', file name too short", srcFileName.c_str());
		return;
	}

	time_t	now = time (0);
	struct tm	ltime;

	gmtime_r (&now, &ltime);

	size -= 12;
	ltime.tm_year = readCharInt (srcFName + size, 4) - 1900;
	size += 4;
	ltime.tm_mon = readCharInt (srcFName + size, 2) - 1;
	size += 2;
	ltime.tm_mday = readCharInt (srcFName + size, 2);
	size += 2;
	ltime.tm_hour = readCharInt (srcFName + size, 2);
	size += 2;
	ltime.tm_min = readCharInt (srcFName + size, 2);
	size += 2;

	mktime (&ltime);
	ltime.tm_sec = -ltime.tm_gmtoff;

	int tzHour = ltime.tm_gmtoff / 3600;
	int tzMin = (ltime.tm_gmtoff / 60) % 60;

	if (mktime (&ltime) < 0)
	{
		statmErrReport (SC_STATM, SC_ERR, "cannot deduce statistical measurement time from '%s', mktime() failed, errno = %d", srcFileName.c_str(), errno);
		return;
	}

	if ((src = fopen (srcFileName.c_str(), "rb")) == 0)
	{
		statmErrReport (SC_STATM, SC_ERR, "fopen (%s, 'br') failed, errno = %d", srcFileName.c_str(), errno);
		return;
	}

	if ((dst = fopen (dstFileName.c_str(), "w")) == 0)
	{
		fclose (src);
		statmErrReport (SC_STATM, SC_ERR, "fopen (%s, 'wr') failed, errno = %d", dstFileName.c_str(), errno);
		return;
	}

	char*	nodeid = getenv ("NODEID");
	if (nodeid == 0)
		nodeid = (char*) "--unknown--";

	int	cscfType = GetImsCscfType(atoi (nodeid));
	stringstream	cscfTypeNames;
	bool	empty = true;

	for (int i = 0, flags = syn_cv_cscf_type_p; flags <= syn_cv_cscf_type_tagcf; flags <<= 1, ++i)
	{
		if ((cscfType & flags) == 0)
			continue;
		if (!empty)
			cscfTypeNames << ",";
		empty = false;
		cscfTypeNames << cscfTypeName[i];
	}
	if (empty)
		cscfTypeNames << "unknown";
	cscfTypeNames << ends;

	fprintf (dst, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

	fprintf (dst, "<measCollecFile xmlns=\"http://www.3gpp.org/ftp/specs/archive/32_series/32.435#measCollec\" ");
	fprintf (dst, "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ");
	fprintf (dst, "xsi:schemaLocation=\"http://www.3gpp.org/ftp/specs/archive/32_series/32.435#measCollec\">\n");

	fprintf (dst, "\t<fileHeader fileFormatVersion=\"1.0\" vendorName=\"Iskratel\" dnPrefix=\"%s\">\n", "");
	fprintf (dst, "\t\t<fileSender elementType=\"%s\" localDn=\"%s\"/>\n", cscfTypeNames.str().c_str(), nodeid);
	fprintf (dst, "\t\t<measCollec beginTime=\"%04d-%02d-%02dT%02d:%02d:00%+03d:%02d\"/>\n",
		ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday, ltime.tm_hour, ltime.tm_min, tzHour, tzMin);
	fprintf (dst, "\t</fileHeader>\n");

	fprintf (dst, "\t<measData>\n");

	char	hostname[132];
	gethostname(hostname, 128);
	fprintf (dst, "\t\t<managedElement localDn=\"%s\"/>\n", nodeid);

	while ((size = fread (field, 1, sizeof field, src)) > 0)
	{
		if (size != sizeof field)
		{
			statmErrReport (SC_STATM, SC_ERR, "size of file '%s' is not multiple of statistical record size", srcFileName.c_str());
			break;
		}

		stringstream	statValues;
		stringstream	statNames;

		int	statGroup = readInt (field);
		int	statGType = readInt (field + 4);

		if ((statGroup & (1 << 16)) != 0)
		{
			if ((size = fread (gname, 1, sizeof gname, src)) != sizeof gname)
			{
				statmErrReport (SC_STATM, SC_ERR, "size of file '%s' is not multiple of statistical record size", srcFileName.c_str());
				break;
			}
			gname [sizeof gname - 1] = 0;
		}

		const char*	typeName = StatisticalAdapterThread::FetchCounterName(cntrkey (statGType, (u_int) -1));

		if (typeName != 0)
		{
			fprintf (dst, "\t\t\t<measInfo measInfoId=\"%s\">\n", typeName);
			fprintf (dst, "\t\t\t<job jobId=\"%s\"/>\n", typeName);
		}
		else
		{
			fprintf (dst, "\t\t\t<measInfo measInfoId=\"%d\">\n", statGType);
			fprintf (dst, "\t\t\t<job jobId=\"%d\"/>\n", statGType);
		}
		fprintf (dst, "\t\t\t<granPeriod endTime=\"%04d-%02d-%02dT%02d:%02d:00%+03d:%02d\" duration=\"%s\"/>\n",
			ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday, ltime.tm_hour, ltime.tm_min, tzHour, tzMin, "PT900S");
		fprintf (dst, "\t\t\t<repPeriod duration=\"%s\"/>\n", "PT1800S");

		const u_char*	ptr = field + 8;
		bool	nameCount = false;
		for (int i = 0; i < 31; ++i, ptr += 4)
		{
			int	val = readInt (ptr);
			const char*	statName = StatisticalAdapterThread::FetchCounterName(cntrkey (statGType, i + 1));
			if (statName == 0)
				continue;
			statValues << val << ' ';
			if (nameCount)
				statNames << ' ';
			else
				nameCount = true;
			statNames << statName;
		}

		statNames << ends;
		statValues << ends;

		fprintf (dst, "\t\t\t<measTypes>%s</measTypes>\n", statNames.str().c_str());
		if ((statGroup & (1 << 16)) != 0)
			fprintf (dst, "\t\t\t<measValue measObjLdn=\"%s\">\n", gname);
		else
			fprintf (dst, "\t\t\t<measValue measObjLdn=\"%d\">\n", statGroup - 1000);
		fprintf (dst, "\t\t\t\t<measResults>%s</measResults>\n", statValues.str().c_str());
		fprintf (dst, "\t\t\t</measValue>\n");
		fprintf (dst, "\t\t\t</measInfo>\n");
	}

	ltime.tm_min += 15;
	mktime (&ltime);

	fprintf (dst, "\t</measData>\n");

	fprintf (dst, "\t<fileFooter>\n");
	fprintf (dst, "\t<measCollec endTime=\"%04d-%02d-%02dT%02d:%02d:00%+03d:%02d\"/>\n",
		ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday, ltime.tm_hour, ltime.tm_min, tzHour, tzMin);
	fprintf (dst, "\t</fileFooter>\n");

	fprintf (dst, "</measCollecFile>\n");

	fclose (src);
	fclose (dst);
}
