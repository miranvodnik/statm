
#include <string>
using namespace std;

/*! @brief error info object
 *
 */
class	StatmErrInfo
{
private:
	StatmErrInfo ();

	int	Connect (void);
	void	Disconnect (void);

public:

	static	inline	StatmErrInfo	*statmErrInfo (void) { return g_errInfo; }
	int	ErrReport (int module, int severity, const char* format, va_list msg);
	bool	disable (bool disable);

private:
	static	StatmErrInfo*	g_errInfo;
	int	m_logLevel;	//!< log level
	string	m_syslogSocketPath;	//!< UNIX domain socket path to access syslog-ng
	int	m_syslogSocket;	//!< UNIX domain socket to access syslog-ng
	bool	m_disabled;	//!< disable error reporting when set to true
};
