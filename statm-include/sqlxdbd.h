/*
 * sqlxdbd.h
 *
 *  Created on: 4. feb. 2016
 *      Author: miran
 */

#pragma once

#include <sqlunix.h>

#if !defined(__xdbdimplementation)

#define	SQLAllocConnect(EnvironmentHandle,ConnectionHandle)	\
	SQLAllocHandle(SQL_HANDLE_DBC,EnvironmentHandle,ConnectionHandle)

#define	SQLAllocEnv(EnvironmentHandle)	\
	SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,EnvironmentHandle)

#define	SQLAllocStmt(ConnectionHandle,StatementHandle)	\
	SQLAllocHandle(SQL_HANDLE_STMT,ConnectionHandle,StatementHandle)

#define	SQLBindParameter(StatementHandle,ParameterNumber,fParamType,ValueType,ParameterType,LengthPrecision,ParameterScale,ParameterValue,cbValueMax,StrLen_or_Ind)	\
	SQLBindParam(StatementHandle,ParameterNumber,ValueType,ParameterType,LengthPrecision,ParameterScale,ParameterValue,StrLen_or_Ind)

#define	SQLFreeConnect(ConnectionHandle)	\
	SQLFreeHandle(SQL_HANDLE_DBC,ConnectionHandle)

#define	SQLFreeEnv(EnvironmentHandle)	\
	SQLFreeHandle(SQL_HANDLE_ENV,EnvironmentHandle)

#define	SQLFreeStmt(StatementHandle,Option)	\
	SQLFreeHandle(SQL_HANDLE_STMT,StatementHandle)

#define SQLGetConnectOption(ConnectionHandle,Option,Value)	\
	SQLGetConnectAttr(ConnectionHandle,Attribute,Value,0,0)

#define SQLGetStmtOption(StatementHandle,Option,Value)	\
	SQLGetStmtAttr(StatementHandle,Option,Value,0,0)

#define	SQLSetConnectOption(ConnectionHandle,Option,Value)	\
	SQLSetConnectAttr(ConnectionHandle,Option,Value,0)

#define SQLSetParam(StatementHandle,ParameterNumber,ValueType,ParameterType,LengthPrecision,ParameterScale,ParameterValue,StrLen_or_Ind)	\
	SQLBindParam(StatementHandle,ParameterNumber,ValueType,ParameterType,LengthPrecision,ParameterScale,ParameterValue,StrLen_or_Ind)

#define SQLSetStmtOption(StatementHandle,Option,Value)	\
	SQLSetStmtAttr(StatementHandle,Option,Value,0)

#endif
