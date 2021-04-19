
/****************************************************************/
/*      Header files                                            */
/****************************************************************/

#include <sys/select.h>		// types declarations
#include <sys/ioctl.h>
#include <sys/types.h>		// types declarations
#include <sys/stat.h>		// file modes
#include <sys/mman.h>		// file mapping API
#include <sys/inotify.h>	// file system notifications
#include <dirent.h>		// directory entries
#include <regex.h>		// regular expressions
#include <grp.h>		// group info
#include <pwd.h>		// user info
#include <sqlxdbd.h>
#include <sqlext.h>
#include <unistd.h>		// standard library
#include <stddef.h>		// standard library
#include <libxml/SAX2.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "StatisticalAdapter.h"	// general adapter
#include "StatisticalAdapterThread.h"	// general adapter
#include "StatmErrApi.h"

/****************************************************************/
/*      Private data                                            */
/****************************************************************/

string StatisticalAdapterThread::m_xmlCfgFileName = "";
string StatisticalAdapterThread::m_inputCatalogName = "";
StatisticalAdapterThread* StatisticalAdapterThread::g_StatisticalAdapterThread = new StatisticalAdapterThread ();

void StatisticalAdapterThread::_Init ()
{
	char* env = getenv ("STATM_CONFIG");

	if (env != 0)
	{
		if (strcasecmp (env, "xml") == 0)
			ReadXMLSettings();
		else if (strcasecmp(env, "db") == 0)
			ReadDBSettings();
		else if (strcasecmp(env, "fs") == 0)
			ReadFSSettings();
	}
	else
		ReadFSSettings();
	ReadStatCountersInfo();
}

void StatisticalAdapterThread::_Release ()
{
	for (cntrset::iterator cit = m_cntrset.begin(); cit != m_cntrset.end(); ++cit)
		delete cit->second;
	m_cntrset.clear();
	for (salist::iterator sit = m_salist.begin(); sit != m_salist.end(); ++sit)
		delete sit->second;
	m_salist.clear();
	statmErrReport(SC_STATM, SC_ERR, "Statistical adapters deleted");
}

static xmlChar* get_node_text (xmlNodePtr node)
{
	xmlChar* content = 0;
	for (xmlNodePtr child = node->children; child != 0; child = child->next)
	{
		if (child->type != XML_TEXT_NODE)
			continue;
		content = child->content;
		break;
	}
	return content;
}

static xmlChar* get_config_value (xmlXPathContextPtr xmlCtx, xmlChar* tabName, xmlChar* indexName, xmlChar* indexValue, xmlChar* cfgName)
{
	xmlChar* content = (xmlChar*) "";

	stringstream path;
	path << "//" << tabName << "[" << indexName << "='" << indexValue << "']/" << cfgName;
	xmlXPathObjectPtr obj = xmlXPathEvalExpression ((xmlChar*) path.str().c_str (), xmlCtx);

	xmlNodeSetPtr nodes = obj->nodesetval;
	if (nodes->nodeNr < 1)
		return content;
	return get_node_text (nodes->nodeTab[0]);
}

int StatisticalAdapterThread::ReadXMLSettings ()
{
	int result = EXIT_FAILURE;

	char* xmlCfgFileName = getenv ("STATM_XML_CONFIG_FILE_NAME");
	if (xmlCfgFileName == 0)
		return 0;

	m_xmlCfgFileName = xmlCfgFileName;
	statmErrReport (SC_STATM, SC_ERR, "Reading XML config file '%s'", xmlCfgFileName);

	xmlDocPtr xmlDoc = 0;
	xmlXPathContextPtr xmlCtx = 0;
	xmlDoc = xmlReadFile (xmlCfgFileName, 0, 0);

	xmlCtx = xmlXPathNewContext (xmlDoc);
	if (xmlCtx == 0)
	{
		return	-1;
	}

	{
		stringstream pathExpr;
		pathExpr << "//stat_adapter/adapter_id";
		xmlXPathObjectPtr obj = xmlXPathEvalExpression ((xmlChar*) pathExpr.str().c_str (), xmlCtx);
		if (obj == 0)
		{
			return -1;
		}

		xmlNodeSetPtr nodes = obj->nodesetval;
		int nodenr = (nodes != 0) ? nodes->nodeNr : 0;

		for (int nr = 0; nr < nodenr; ++nr)
		{
			xmlNodePtr node = nodes->nodeTab [nr];
			if (node == 0)
				continue;
			xmlChar* adapterId = get_node_text (node);
			if (adapterId == 0)
				continue;

			xmlChar* srcDirName = get_config_value (xmlCtx, (xmlChar*) "stat_adapter", (xmlChar*) "adapter_id", adapterId, (xmlChar*) "stat_dir_name");
			xmlChar* dstDirName = get_config_value (xmlCtx, (xmlChar*) "stat_adapter", (xmlChar*) "adapter_id", adapterId, (xmlChar*) "dest_dir_name");
			xmlChar* ownerName = get_config_value (xmlCtx, (xmlChar*) "stat_adapter", (xmlChar*) "adapter_id", adapterId, (xmlChar*) "owner_name");
			xmlChar* groupName = get_config_value (xmlCtx, (xmlChar*) "stat_adapter", (xmlChar*) "adapter_id", adapterId, (xmlChar*) "group_name");
			xmlChar* fperm = get_config_value (xmlCtx, (xmlChar*) "stat_adapter", (xmlChar*) "adapter_id", adapterId, (xmlChar*) "dir_permissions");
			xmlChar* dperm = get_config_value (xmlCtx, (xmlChar*) "stat_adapter", (xmlChar*) "adapter_id", adapterId, (xmlChar*) "file_permissions");

			cout << "stat adapter (" << adapterId << ", " << srcDirName << ", " << dstDirName << ", " << ownerName << ", " << groupName << ", " << fperm << ", " << dperm << ")" << endl;

			unsigned short adapter_id = atoi ((const char*) adapterId);
			salist::iterator sit = m_salist.find (adapter_id);
			if (sit == m_salist.end())
			{
				StatisticalAdapter* p_StatisticalAdapter = new StatisticalAdapter
					(
						adapter_id,
						(const char*) srcDirName,
						(const char*) dstDirName,
						(const char*) ownerName,
						(const char*) groupName,
						(const char*) fperm,
						(const char*) dperm
					);
				salist::iterator ssit;
				for (ssit = m_salist.begin(); ssit != m_salist.end(); ++ssit)
				{
					if (strcmp (p_StatisticalAdapter->statDirName(), ssit->second->statDirName()) == 0)
						break;
				}
				if (ssit != m_salist.end())
				{
					p_StatisticalAdapter->link (ssit->second->link());
					ssit->second->link (p_StatisticalAdapter);
				}
				else
					m_salist[adapter_id] = p_StatisticalAdapter;
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter added, id = %d, bin-dir = %s, xml-dir = %s, owner = %s, group = %s, fperm = %s, dperm = %s",
						adapter_id, srcDirName, dstDirName, ownerName, groupName, fperm, dperm);

				pathExpr.str("");
				pathExpr << "//stat_adapter_arg[adapter_id='" << adapterId << "']/arg_id";
				string path = pathExpr.str();
				obj = xmlXPathEvalExpression ((xmlChar*) pathExpr.str().c_str (), xmlCtx);
				if (obj == 0)
				{
					return -1;
				}

				xmlNodeSetPtr argnodes = obj->nodesetval;
				int argnr = (argnodes != 0) ? argnodes->nodeNr : 0;

				for (int anr = 0; anr < argnr; ++anr)	// stat_adapter table
				{
					xmlNodePtr node = argnodes->nodeTab [anr];
					if (node == 0)
						continue;
					xmlChar* argId = get_node_text (node);
					if (argId == 0)
						continue;

					xmlChar* argName  = get_config_value (xmlCtx, (xmlChar*) "stat_adapter_arg", (xmlChar*) "arg_id", argId, (xmlChar*) "arg_name");
					xmlChar* argValue = get_config_value (xmlCtx, (xmlChar*) "stat_adapter_arg", (xmlChar*) "arg_id", argId, (xmlChar*) "arg_value");

					cout << "stat adapter_arg (" << argName << ", " << argValue << ")" << endl;

					unsigned short	arg_id = atoi ((const char*) argId);
					StatisticalAdapterArg* p_StatisticalAdapterArg = new StatisticalAdapterArg
						(
							arg_id,
							adapter_id,
							(const char*) argName,
							(const char*) argValue
						);
					p_StatisticalAdapter->SetArg (p_StatisticalAdapterArg);
					delete p_StatisticalAdapterArg;
					statmErrReport (SC_STATM, SC_ERR, "Set statistical adapter (id = %d) argument (%s, %s)", adapter_id, argName, argValue);
				}
			}
			else
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter already exists, id = %d", adapter_id);
			}
		}
	}

	if (xmlDoc != 0)
	xmlFreeDoc (xmlDoc);

	return 0;

	return result;
}

int StatisticalAdapterThread::ReadDBSettings ()
{
	static const char* stat_adapter_query = "select nodeid, adapter_id, stat_dir_name, dest_dir_name, owner_name, group_name, dir_permissions, file_permissions from stat_adapter where nodeid=?";
	static const char* stat_arg_query     = "select nodeid, arg_id, adapter_id, arg_name, arg_value from stat_adapter_arg where nodeid=? and adapter_id=?";

	static SQLHENV	env = SQL_NULL_HENV;
	static SQLHDBC	dbc = SQL_NULL_HDBC;
	static SQLHSTMT	stat_stm = SQL_NULL_HSTMT;
	static SQLHSTMT	arg_stm = SQL_NULL_HSTMT;

	static unsigned int nodeid = 0;
	static unsigned int adapterId = 0;

	static unsigned short node_id;
	static unsigned short adapter_id;
	static char	stat_dir_name[256];
	static char	dest_dir_name[256];
	static char	owner_name[64];
	static char	group_name[64];
	static char	dir_permissions[64];
	static char	file_permissions[64];
	static unsigned short arg_id;
	static char stat_arg_name[32];
	static char stat_arg_value[32];

	bool done = false;

	while (!done)
	{
		if (nodeid == 0)
		{
			const char* nodeidStr = getenv ("NODEID");
			if (nodeidStr == 0)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot get NODEID");
				break;
			}
			nodeid = atoi (nodeidStr);
		}

		if (env == SQL_NULL_HENV)
		{
			if (SQLAllocHandle(SQL_HANDLE_ENV, 0, &env) != SQL_SUCCESS)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot allocate ENV");
				break;
			}
		}

		if (dbc == SQL_NULL_HDBC)
		{
			if (SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc) != SQL_SUCCESS)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot allocate DBC");
				break;
			}

			if (SQLConnect (dbc, (SQLCHAR*) "localhost", SQL_NTS, (SQLCHAR*) "user", SQL_NTS, (SQLCHAR*) "passwd", SQL_NTS) != SQL_SUCCESS)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot connect to data base");
				dbc = SQL_NULL_HDBC;
				break;
			}
		}

		if (stat_stm == SQL_NULL_HSTMT)
		{
			if (SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stat_stm) != SQL_SUCCESS)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot allocate STMT");
				break;
			}

			if (SQLPrepare(stat_stm, (SQLCHAR*) stat_adapter_query, SQL_NTS) != SQL_SUCCESS)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot prepare '%s'", stat_adapter_query);
				SQLFreeHandle(SQL_HANDLE_STMT, stat_stm);
				stat_stm = SQL_NULL_HSTMT;
				break;
			}

			if (SQLBindParameter(stat_stm, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &nodeid, 0, NULL) != SQL_SUCCESS)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot bind parameter NODEID");
				SQLFreeHandle(SQL_HANDLE_STMT, stat_stm);
				stat_stm = SQL_NULL_HSTMT;
				break;
			}

			node_id = -1;
			if (SQLBindCol (stat_stm, 1, SQL_C_SLONG, &node_id, sizeof(node_id), NULL) != SQL_SUCCESS)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot bind column NODEID");
				SQLFreeHandle(SQL_HANDLE_STMT, stat_stm);
				stat_stm = SQL_NULL_HSTMT;
				break;
			}

			adapter_id = -1;
			if (SQLBindCol (stat_stm, 2, SQL_C_SLONG, &adapter_id, sizeof(adapter_id), NULL) != SQL_SUCCESS)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot bind column ADAPTER_ID");
				SQLFreeHandle(SQL_HANDLE_STMT, stat_stm);
				stat_stm = SQL_NULL_HSTMT;
				break;
			}

			memset ((void*)stat_dir_name, 0, sizeof (stat_dir_name));
			if (SQLBindCol (stat_stm, 3, SQL_C_CHAR, (SQLPOINTER)stat_dir_name, sizeof(stat_dir_name), NULL) != SQL_SUCCESS)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot bind column STAT_DIR_NAME");
				SQLFreeHandle(SQL_HANDLE_STMT, stat_stm);
				stat_stm = SQL_NULL_HSTMT;
				break;
			}

			memset ((void*)dest_dir_name, 0, sizeof (dest_dir_name));
			if (SQLBindCol (stat_stm, 4, SQL_C_CHAR, (SQLPOINTER)dest_dir_name, sizeof(dest_dir_name), NULL) != SQL_SUCCESS)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot bind column DEST_DIR_NAME");
				SQLFreeHandle(SQL_HANDLE_STMT, stat_stm);
				stat_stm = SQL_NULL_HSTMT;
				break;
			}

			memset ((void*)owner_name, 0, sizeof (owner_name));
			if (SQLBindCol (stat_stm, 5, SQL_C_CHAR, (SQLPOINTER)owner_name, sizeof(owner_name), NULL) != SQL_SUCCESS)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot bind column OWNER_NAME");
				SQLFreeHandle(SQL_HANDLE_STMT, stat_stm);
				stat_stm = SQL_NULL_HSTMT;
				break;
			}

			memset ((void*)group_name, 0, sizeof (group_name));
			if (SQLBindCol (stat_stm, 6, SQL_C_CHAR, (SQLPOINTER)group_name, sizeof(group_name), NULL) != SQL_SUCCESS)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot bind column GROUP_NAME");
				SQLFreeHandle(SQL_HANDLE_STMT, stat_stm);
				stat_stm = SQL_NULL_HSTMT;
				break;
			}

			memset ((void*)dir_permissions, 0, sizeof (dir_permissions));
			if (SQLBindCol (stat_stm, 7, SQL_C_CHAR, (SQLPOINTER)dir_permissions, sizeof(dir_permissions), NULL) != SQL_SUCCESS)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot bind column DIR_PERMISSIONS");
				SQLFreeHandle(SQL_HANDLE_STMT, stat_stm);
				stat_stm = SQL_NULL_HSTMT;
				break;
			}

			memset ((void*)file_permissions, 0, sizeof (file_permissions));
			if (SQLBindCol (stat_stm, 8, SQL_C_CHAR, (SQLPOINTER)file_permissions, sizeof(file_permissions), NULL) != SQL_SUCCESS)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot bind column FILE_PERMISSIONS");
				SQLFreeHandle(SQL_HANDLE_STMT, stat_stm);
				stat_stm = SQL_NULL_HSTMT;
				break;
			}
		}


		if (arg_stm == SQL_NULL_HSTMT)
		{
			if (SQLAllocHandle(SQL_HANDLE_STMT, dbc, &arg_stm) != SQL_SUCCESS)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot allocate STMT");
				break;
			}

			if (SQLPrepare(arg_stm, (SQLCHAR*) stat_arg_query, SQL_NTS) != SQL_SUCCESS)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot prepare '%s'", stat_arg_query);
				SQLFreeHandle(SQL_HANDLE_STMT, arg_stm);
				arg_stm = SQL_NULL_HSTMT;
				break;
			}

			if (SQLBindParameter(arg_stm, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &nodeid, 0, NULL) != SQL_SUCCESS)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot bind parameter NODEID");
				SQLFreeHandle(SQL_HANDLE_STMT, arg_stm);
				arg_stm = SQL_NULL_HSTMT;
				break;
			}

			if (SQLBindParameter(arg_stm, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &adapterId, 0, NULL) != SQL_SUCCESS)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot bind parameter ADAPTER_ID");
				SQLFreeHandle(SQL_HANDLE_STMT, arg_stm);
				arg_stm = SQL_NULL_HSTMT;
				break;
			}

			node_id = -1;
			if (SQLBindCol (arg_stm, 1, SQL_C_SLONG, &node_id, sizeof(node_id), NULL) != SQL_SUCCESS)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot bind column NODEID");
				SQLFreeHandle(SQL_HANDLE_STMT, arg_stm);
				arg_stm = SQL_NULL_HSTMT;
				break;
			}

			arg_id = -1;
			if (SQLBindCol (arg_stm, 2, SQL_C_SLONG, &arg_id, sizeof(arg_id), NULL) != SQL_SUCCESS)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot bind column ARG_ID");
				SQLFreeHandle(SQL_HANDLE_STMT, arg_stm);
				arg_stm = SQL_NULL_HSTMT;
				break;
			}

			adapter_id = -1;
			if (SQLBindCol (arg_stm, 3, SQL_C_SLONG, &adapter_id, sizeof(adapter_id), NULL) != SQL_SUCCESS)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot bind column ADAPTER_ID");
				SQLFreeHandle(SQL_HANDLE_STMT, arg_stm);
				arg_stm = SQL_NULL_HSTMT;
				break;
			}

			memset ((void*)stat_arg_name, 0, sizeof (stat_arg_name));
			if (SQLBindCol (arg_stm, 4, SQL_C_CHAR, (SQLPOINTER)stat_arg_name, sizeof(stat_arg_name), NULL) != SQL_SUCCESS)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot bind column ARG_NAME");
				SQLFreeHandle(SQL_HANDLE_STMT, arg_stm);
				arg_stm = SQL_NULL_HSTMT;
				break;
			}

			memset ((void*)stat_arg_value, 0, sizeof (stat_arg_value));
			if (SQLBindCol (arg_stm, 5, SQL_C_CHAR, (SQLPOINTER)stat_arg_value, sizeof(stat_arg_value), NULL) != SQL_SUCCESS)
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot bind column ARG_VALUE");
				SQLFreeHandle(SQL_HANDLE_STMT, arg_stm);
				arg_stm = SQL_NULL_HSTMT;
				break;
			}
		}

		if (SQLExecute (stat_stm) != SQL_SUCCESS)
		{
			statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot execute '%s'", stat_adapter_query);
			SQLFreeHandle(SQL_HANDLE_STMT, stat_stm);
			stat_stm = SQL_NULL_HSTMT;
			break;
		}

		while (SQLFetch(stat_stm) == SQL_SUCCESS)
		{
			salist::iterator sit = m_salist.find (adapter_id);
			if (sit == m_salist.end())
			{
				StatisticalAdapter* p_StatisticalAdapter = new StatisticalAdapter
					(
						adapter_id,
						(const char*) stat_dir_name,
						(const char*) dest_dir_name,
						(const char*) owner_name,
						(const char*) group_name,
						(const char*) dir_permissions,
						(const char*) file_permissions
					);
				salist::iterator ssit;
				for (ssit = m_salist.begin(); ssit != m_salist.end(); ++ssit)
				{
					if (strcmp (p_StatisticalAdapter->statDirName(), ssit->second->statDirName()) == 0)
						break;
				}
				if (ssit != m_salist.end())
				{
					p_StatisticalAdapter->link (ssit->second->link());
					ssit->second->link (p_StatisticalAdapter);
				}
				else
					m_salist[adapter_id] = p_StatisticalAdapter;
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter added, id = %d, bin-dir = %s, xml-dir = %s", adapter_id, stat_dir_name, dest_dir_name);

				adapterId = adapter_id;

				if (SQLExecute (arg_stm) != SQL_SUCCESS)
				{
					statmErrReport (SC_STATM, SC_ERR, "Statistical adapter: Cannot execute '%s'", stat_arg_query);
					SQLFreeHandle(SQL_HANDLE_STMT, arg_stm);
					arg_stm = SQL_NULL_HSTMT;
					break;
				}

				while (SQLFetch(arg_stm) == SQL_SUCCESS)
				{
					StatisticalAdapterArg* p_StatisticalAdapterArg = new StatisticalAdapterArg
						(
							arg_id,
							adapter_id,
							stat_arg_name,
							stat_arg_value
						);
					p_StatisticalAdapter->SetArg (p_StatisticalAdapterArg);
					delete p_StatisticalAdapterArg;
					statmErrReport (SC_STATM, SC_ERR, "Set statistical adapter (id = %d) argument (%s, %s)", adapter_id, stat_arg_name, stat_arg_value);
				}
			}
			else
			{
				statmErrReport (SC_STATM, SC_ERR, "Statistical adapter already exists, id = %d", adapter_id);
			}
		}

		done = true;
	}

	return done ? 0 : -1;
}

static	int	CheckPath(const char*, const char*);

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

static	int	CheckPath(const char* path, const char* delimiter)
{
	char	pcwd[PATH_MAX + 1];
	char* p, * q;

	string	dname;

	if (*path != '/')
		dname = ".";

	umask(0);

	strncpy(pcwd, path, PATH_MAX);
	for (p = pcwd; NULL != (q = strsep(&p, delimiter)); )
	{
		char* dn;
		struct	stat	sb;

		dname += '/';
		dname += q;

		if (stat((dn = (char*)dname.c_str()), &sb) < 0)
		{
			statmErrReport(SC_STATM, SC_APL, "Try to check path '%s'", dn);
			if (errno != ENOENT)
			{
				stringstream	str;
				const char* msg;

				str << "Check Path: stat (" << dn << ") failed, errno = " << errno << ends;
				msg = str.str().c_str();
				statmErrReport(SC_STATM, SC_ERR, msg);
				return	-1;
			}

			if (mkdir(dn, 0777) < 0)
			{
				stringstream	str;
				const char* msg;

				str << "Check Path: path '" << dn << "' does not exist" << ends;
				msg = str.str().c_str();
				statmErrReport(SC_STATM, SC_ERR, msg);
				return	-1;
			}
			statmErrReport(SC_STATM, SC_APL, "Path checked '%s'", dn);
		}
		else	if (!S_ISDIR(sb.st_mode))
		{
			stringstream	str;
			const char* msg;

			str << "Check Path: path (" << dn
				<< ") exists, but is not dir, stat mode = ("
				<< sb.st_mode << ")" << ends;
			msg = str.str().c_str();
			statmErrReport(SC_STATM, SC_ERR, msg);
			return	-1;
		}
	}

	return	0;
}

int StatisticalAdapterThread::ReadFSSettings()
{
	char* statmIn  = getenv("STATM_INPUT_DIRECTORY");
	char* statmOut = getenv("STATM_OUTPUT_DIRECTORY");

	if (statmIn == 0)
	{
		statmErrReport(SC_STATM, SC_ERR, "STATM_INPUT_DIRECTORY environment variable is not defined");
		return -1;
	}

	if (statmOut == 0)
	{
		statmErrReport(SC_STATM, SC_ERR, "STATM_OUTPUT_DIRECTORY environment variable is not defined");
		return -1;
	}

	m_inputCatalogName = statmIn;
	m_outputCatalogName = statmOut;

	CheckPath(statmIn, "/\\");
	CheckPath(statmOut, "/\\");

	DIR* inDir;
	if ((inDir = opendir(statmIn)) == 0)
	{
		statmErrReport(SC_STATM, SC_ERR, "Cannot open statistical module input directory '%s'", statmIn);
		return -1;
	}

	DIR* outDir;
	if ((outDir = opendir(statmOut)) == 0)
	{
		closedir(inDir);
		statmErrReport(SC_STATM, SC_ERR, "Cannot open statistical module output directory '%s'", statmOut);
		return -1;
	}
	closedir(outDir);

	struct dirent	entry;
	struct dirent* rentry = 0;

	int adapter_id = 0;
	while (readdir_r(inDir, &entry, &rentry) == 0)
	{
		if (rentry == 0)
			break;

		if (strcmp(entry.d_name, ".") == 0)
			continue;

		if (strcmp(entry.d_name, "..") == 0)
			continue;

		string	srcf = statmIn;
		srcf += '/';
		srcf += entry.d_name;

		string	dstf = statmOut;
		dstf += '/';
		dstf += entry.d_name;

		struct stat	sb;
		if (stat(srcf.c_str(), &sb) < 0)
			continue;
		if (S_ISDIR(sb.st_mode) == 0)
			continue;
		CheckPath(dstf.c_str(), "/\\");
		StatisticalAdapter* p_StatisticalAdapter = new StatisticalAdapter
		(
			++adapter_id,
			(const char*)srcf.c_str(),
			(const char*)dstf.c_str(),
			(const char*)"owner",
			(const char*)"group",
			(const char*)"755",
			(const char*)"644"
		);
		m_salist[adapter_id] = p_StatisticalAdapter;
		statmErrReport(SC_STATM, SC_ERR, "Statistical adapter added, id = %d, bin-dir = %s, xml-dir = %s", adapter_id, srcf.c_str(), dstf.c_str());
	}
	closedir(inDir);

	return 0;
}

int	StatisticalAdapterThread::ReadStatCountersInfo ()
{
	char*	cntrFname = getenv ("STATM_STAT_CNTR_INFO");

	if (cntrFname == 0)
	{
		statmErrReport (SC_STATM, SC_ERR, "Cannot read statistical counters info, ENV variable STATM_STAT_CNTR_INFO not found");
		return	-1;
	}

	FILE*	cntrFd = fopen (cntrFname, "r");

	if (cntrFd == 0)
	{
		statmErrReport (SC_STATM, SC_ERR, "Cannot open statistical counters info file '%s', errno = %d", cntrFname, errno);
		return	-1;
	}

	int	statGroup = -1;
	char	line[4100];
	if (fgets (line, 4096, cntrFd) != 0)
	while (fgets (line, 4096, cntrFd) != 0)
	{
		int	statCounter = -1;
		int	dataCounter = 0;
		int	fieldCounter = 0;
		char*	fields[6];
		fields[fieldCounter] = line;
		line[4096] = 0;
		for (char* readPtr = line; (*readPtr != 0) && (*readPtr != '\n'); ++readPtr)
		{
			if (*readPtr == '\t')
			{
				*readPtr = 0;
				if (*fields[fieldCounter] != 0)
					++dataCounter;
				if (++fieldCounter < 6)
					fields[fieldCounter] = readPtr + 1;
				else
					--fieldCounter;
			}
		}
		if (dataCounter == 0)
		{
			statGroup = -1;
			continue;
		}
		if ((*fields[0] == 0) && (*fields[3] == 0))
			continue;
		if (*fields[0] != 0)
		{
			char*	ptr;
			if ((ptr = strstr (fields[0], " = ")) != 0)
				statGroup = atoi (ptr + 3);
			else if ((ptr = strstr (fields[0], " =")) != 0)
				statGroup = atoi (ptr + 2);
			else if ((ptr = strstr (fields[0], "= ")) != 0)
				statGroup = atoi (ptr + 2);
			else if ((ptr = strstr (fields[0], "=")) != 0)
				statGroup = atoi (ptr + 1);
			m_cntrset[cntrkey (statGroup, (u_int) -1)] = new StatisticalCounterInfo (statGroup, (u_int) -1, (char*) "", fields[1], (char*) "", (char*) "");
		}
		if (*fields[3] != 0)
		{
			char*	ptr;
			if ((ptr = strstr (fields[2], " = ")) != 0)
				statCounter = atoi (ptr + 3);
			else if ((ptr = strstr (fields[2], " =")) != 0)
				statCounter = atoi (ptr + 2);
			else if ((ptr = strstr (fields[2], "= ")) != 0)
				statCounter = atoi (ptr + 2);
			else if ((ptr = strstr (fields[2], "=")) != 0)
				statCounter = atoi (ptr + 1);
		}

		if ((statGroup < 0) || (statCounter < 0))
			continue;
		if (m_cntrset.find (cntrkey (statGroup, statCounter)) != m_cntrset.end())
			continue;
		m_cntrset[cntrkey (statGroup, statCounter)] = new StatisticalCounterInfo (statGroup, statCounter, fields[1], fields[3], fields[4], fields[5]);
	}

	fclose (cntrFd);
	return	0;

}

const char* StatisticalAdapterThread::_FetchCounterName (cntrkey key)
{
	cntrset::iterator	it = m_cntrset.find(key);
	return	(it == m_cntrset.end()) ? 0 : it->second->statCounterName ();
}

const char* StatisticalAdapterThread::_FetchCounterDescription (cntrkey key)
{
	cntrset::iterator	it = m_cntrset.find(key);
	return	(it == m_cntrset.end()) ? 0 : it->second->description ();
}

const char* StatisticalAdapterThread::_FetchCounterIms (cntrkey key)
{
	cntrset::iterator	it = m_cntrset.find(key);
	return	(it == m_cntrset.end()) ? 0 : it->second->imsElement ();
}

const char* StatisticalAdapterThread::_FetchCounterComment (cntrkey key)
{
	cntrset::iterator	it = m_cntrset.find(key);
	return	(it == m_cntrset.end()) ? 0 : it->second->comment ();
}
