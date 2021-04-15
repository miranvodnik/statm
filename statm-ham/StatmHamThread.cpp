/*
 * StatmHamThread.cpp
 *
 *  Created on: 27. okt. 2016
 *      Author: miran
 */

#include <stdlib.h>
#include <strings.h>
#include "StatmHamThread.h"
#include "StatmErrApi.h"

namespace statm_daemon
{

/*! @brief create HAM thread object
 *
 */
StatmHamThread::StatmHamThread ()
{
#if defined(STATM_HAM)
	m_clnConn = KKIPC_CommPointIdDummy;
	m_hamConn = KKIPC_CommPointIdDummy;
	m_udata = NULL;
	m_registration = EV_START;
	m_workerReady = false;
	m_workerFailed = false;
#endif	// STATM_HAM
	m_useHam = true;
}

/*! @brief release HAM thread object
 *
 */
StatmHamThread::~StatmHamThread ()
{
	Cleanup ();
}

int StatmHamThread::SendHamMsg (HamMsgCode code, HamErrorCode err)
{
	return	0;
}

/*! @brief HAM communication loop
 *
 */
void	StatmHamThread::HAMLoop ()
{
	char*	appName = (char*) "STATM";
	int pSrvId = 0;

	if (!m_useHam)
		throw	HamDontUseHam;

#if defined(STATM_HAM)
	int	status;

	statmErrReport (SC_STATM, SC_APL, "STATM HAM thread: kkipcOpenCommPoint");
	if ((m_clnConn = kkipcOpenCommPoint ((char *)"APPL_TO_HAM", (char *)"STATM", KKIPC_Client, (void**) &m_udata)) == KKIPC_CommPointIdDummy)
	{
		statmErrReport (SC_STATM, SC_ERR, "STATM HAM thread: kkipcOpenCommPoint() failed");
		throw	HamOpenCommPointError;
	}
	SendHamMsg (HamOpenCommPoint, HamNoError);

	statmErrReport (SC_STATM, SC_APL, "STATM HAM thread: kkipcConnectTo");
	if ((m_hamConn = kkipcConnectTo ((char *)"APPL_TO_HAM", m_clnConn)) == KKIPC_CommPointIdDummy)
	{
		statmErrReport (SC_STATM, SC_ERR, "STATM HAM thread: kkipcConnectTo() failed");
		throw	HamConnectError;
	}
	SendHamMsg (HamOpenPeerPoint, HamNoError);

	statmErrReport (SC_STATM, SC_APL, "STATM HAM thread: haSendRegistrationReq");
	if (haSendRegistrationReq (m_clnConn, m_hamConn, &m_udata->regReq, appName) == HAM_ERROR)
	{
		statmErrReport (SC_STATM, SC_ERR, "STATM HAM thread: haSendRegistrationReq() failed");
		throw	HamSendRegistrationError;
	}
	SendHamMsg (HamSendRegReq, HamNoError);

	while (1)
	{
		KKIPC_T_CommPointId	conn;
		KKIPC_T_CommType	ctype;
		unsigned long		len;

		statmErrReport (SC_STATM, SC_APL, "STATM HAM thread: kkipcReceive");
		if (kkipcReceive (m_clnConn, &conn, &ctype, (void**) &m_udata, &len) != KKIPC_OK)
		{
			statmErrReport (SC_STATM, SC_ERR, "STATM HAM thread: kkipcReceive() failed");
			continue;
		}

		if (ctype != KKIPC_COMMTYPE_HA)
			continue;
		HA_T_IpcMessage	*msg = (HA_T_IpcMessage*) m_udata;
		switch (msg->fHeader.fEventName)
		{
		case EV_REG_OK:
			statmErrReport (SC_STATM, SC_APL, "STATM HAM thread: EV_REG_OK");
			m_registration = EV_REG_OK;
			if (haReceiveRegistrationOk (&m_udata->regOk, len, &m_appId) == HAM_ERROR)
			{
				statmErrReport (SC_STATM, SC_ERR, "STATM HAM thread: haReceiveRegistrationOk() failed");
				throw	HamReceiveRegOkError;
			}
			SendHamMsg (HamRecvRegOk, HamNoError);
			break;
		case EV_REG_FAIL:
			statmErrReport (SC_STATM, SC_APL, "STATM HAM thread: EV_REG_FAIL");
			m_registration = EV_REG_FAIL;
			if (haReceiveRegistrationFail (&m_udata->regFail, len) == HAM_ERROR)
			{
				statmErrReport (SC_STATM, SC_ERR, "STATM HAM thread: haReceiveRegistrationFail() failed");
				throw	HamReceiveRegFailError;
			}
			SendHamMsg (HamRecvRegFail, HamNoError);
			break;
		case EV_APP_TO_ACTIVE:
			statmErrReport (SC_STATM, SC_APL, "STATM HAM thread: EV_APP_TO_ACTIVE");
			if (haReceiveAppToActive (&m_udata->toActive, len) == HAM_ERROR)
			{
				statmErrReport (SC_STATM, SC_ERR, "STATM HAM thread: haReceiveAppToActive() failed");
				throw	HamReceiveToActiveError;
			}
			while (!m_workerReady)
			{
				statmErrReport (SC_STATM, SC_APL, "STATM HAM thread: waiting statm worker to become ready, active state");
				sleep (1);
			}
			if (m_workerFailed)
				status = haSendAppStateFail (m_clnConn, conn, &m_udata->stateFail, m_appId);
			else
				status = haSendAppStateOk (m_clnConn, conn, &m_udata->stateOk, m_appId);
			if (status == (int) HAM_ERROR)
			{
				statmErrReport (SC_STATM, SC_ERR, "STATM HAM thread: haSendAppStateOk() failed");
				throw	HamSendAppStateError;
			}
			SendHamMsg (HamRecvAppToActive, HamNoError);
			break;
		case EV_APP_TO_SB:
			statmErrReport (SC_STATM, SC_APL, "STATM HAM thread: EV_APP_TO_SB");
			if (haReceiveAppToStandby (&m_udata->toStandby, len) == HAM_ERROR)
			{
				statmErrReport (SC_STATM, SC_ERR, "STATM HAM thread: haReceiveAppToStandby() failed");
				throw	HamReceiveToStandbyError;
			}
			while (!m_workerReady)
			{
				statmErrReport (SC_STATM, SC_APL, "STATM HAM thread: waiting statm worker to become ready, stand-by state");
				sleep (1);
			}
			if (m_workerFailed)
				status = haSendAppStateFail (m_clnConn, conn, &m_udata->stateFail, m_appId);
			else
				status = haSendAppStateOk (m_clnConn, conn, &m_udata->stateOk, m_appId);
			if (status == (int) HAM_ERROR)
			{
				statmErrReport (SC_STATM, SC_ERR, "STATM HAM thread: haSendAppStateOk() failed");
				throw	HamSendAppStateError;
			}
			SendHamMsg (HamRecvAppToStandby, HamNoError);
			break;
		case EV_APP_TO_BUSY:
			statmErrReport (SC_STATM, SC_APL, "STATM HAM thread: EV_APP_TO_BUSY");
			if (haReceiveAppToBusy (&m_udata->toBusy, len, &pSrvId) == HAM_ERROR)
			{
				statmErrReport (SC_STATM, SC_ERR, "STATM HAM thread: haReceiveAppToBusy() failed");
				throw	HamReceiveToBusyError;
			}
			while (!m_workerReady)
			{
				statmErrReport (SC_STATM, SC_APL, "STATM HAM thread: waiting statm worker to become ready, busy state");
				sleep (1);
			}
			if (m_workerFailed)
				status = haSendAppUstateFail (m_clnConn, conn, &m_udata->ustateFail, m_appId);
			else
				status = haSendAppUstateOk (m_clnConn, conn, &m_udata->ustateOk, m_appId);
			if (status == (int) HAM_ERROR)
			{
				statmErrReport (SC_STATM, SC_ERR, "STATM HAM thread: haSendAppUstateOk() failed");
				throw	HamSendAppUstateError;
			}
			SendHamMsg (HamRecvAppToBusy, HamNoError);
			break;
		case EV_APP_TO_IDLE:
			statmErrReport (SC_STATM, SC_APL, "STATM HAM thread: EV_APP_TO_IDLE");
			if (haReceiveAppToIdle (&m_udata->toIdle, len, &pSrvId) == HAM_ERROR)
			{
				statmErrReport (SC_STATM, SC_ERR, "STATM HAM thread: haReceiveAppToIdle() failed");
				throw	HamReceiveToIdleError;
			}
			while (!m_workerReady)
			{
				statmErrReport (SC_STATM, SC_APL, "STATM HAM thread: waiting statm worker to become ready, idle state");
				sleep (1);
			}
			if (m_workerFailed)
				status = haSendAppUstateFail (m_clnConn, conn, &m_udata->ustateFail, m_appId);
			else
				status = haSendAppUstateOk (m_clnConn, conn, &m_udata->ustateOk, m_appId);
			if (status == (int) HAM_ERROR)
			{
				statmErrReport (SC_STATM, SC_ERR, "STATM HAM thread: haSendAppUstateOk() failed");
				throw	HamSendAppUstateError;
			}
			SendHamMsg (HamRecvAppToIdle, HamNoError);
			break;
		default:
			statmErrReport (SC_STATM, SC_WRN, "STATM HAM thread: unknown message type = %d",
				msg->fHeader.fEventName);
			SendHamMsg (HamRecvUnknown, HamNoError);
			break;
		}
	}
#endif	// STATM_HAM
}

/*! @brief Function closes all connections established with HAM
 *
 */

void	StatmHamThread::Cleanup ()
{
#if defined(STATM_HAM)
	if (m_hamConn != KKIPC_CommPointIdDummy)
		kkipcCloseConnection (m_clnConn, m_hamConn);
	if (m_clnConn != KKIPC_CommPointIdDummy)
		kkipcCloseMyCommunicationPoint (m_clnConn);
	m_clnConn = KKIPC_CommPointIdDummy;
	m_hamConn = KKIPC_CommPointIdDummy;
#endif	// STATM_HAM
}

/*! @brief This function performs initialization of controlling
 *  thread Initialization is successful if controlling message
 *  can be attached.
 *
 *  @return 0 always
 *
 */

int	StatmHamThread::InitInstance (void)
{
	char	*env;

	if ((env = getenv ("STATM_USE_HAM")) != NULL)
	{
		if (strcasecmp (env, "true") != 0)
			m_useHam = false;
	}

	return	0;
}

/*! @brief working function for HAM thread
 *
 *  It connects to HAM, performs registration and handles
 *  other messages reported by HAM. In case of error it terminates
 *  telling to the controlling thread that it should continue
 *  without HAM support
 *
 *  @return 0 always
 */

int	StatmHamThread::Run (void)
{

	statmErrReport (SC_STATM, SC_ERR, "STATM-controller: HAM thread started, TID = %d", getTid());

	pthread_cleanup_push (ThreadCleanup, this);
	while (1)
	{
		try
		{
			HAMLoop ();
		}
		catch (HamErrorCode err)
		{
			Cleanup ();
			SendHamMsg (HamExceptionCode, err);
		}
		break;
	}
	pthread_cleanup_pop (0);

	statmErrReport (SC_STATM, SC_ERR, "STATM-controller: HAM thread terminated, TID = %d", getTid());

	return	0;
}

/*! @brief HAM thread cancellation routine
 *
 */
void StatmHamThread::ThreadCleanup ()
{
	statmErrReport (SC_STATM, SC_ERR, "HAM thread canceled, TID = %d", getTid ());
}

} /* namespace statm_daemon */
