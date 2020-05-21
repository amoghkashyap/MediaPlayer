// SGSE-Labor
// CDatabaseRahmen.cpp
// Framework-Klasse f�r den Zugriff von C++-Programmen �ber ODBC 
// auf einfache Datenbanken
// 
////////////////////////////////////////////////////////////////////////////////
// Header-Dateien
#include <iostream>	
#include <iomanip>
#include <stdlib.h>
#include <stdio.h>
using namespace std;

#include "CDatabase.h"		

////////////////////////////////////////////////////////////////////////////////
// CDatabase-Rahmen-Klasse
// Konstruktor
CDatabase::CDatabase()
{
	m_hDBEnv=NULL;				
	m_hDBC=NULL;					
	m_hStmt=NULL;					
	m_info=0;
	m_eState=DB_S_NOTREADY;
	m_bPrint=true;
}

CDatabase::~CDatabase()
{
	close();
}

// �ffnet die Datenbank
bool CDatabase::open(const char* pDB, const char* pUsr, const char* pPwd)
{
	if(m_eState==DB_S_CONNECTED)	// Zustand pr�fen
		return true;

	m_eState=DB_S_CONNECTED;		// Zustand testweise auf connected setzen
	// Environment-Handle holen
	SQLRETURN ret= SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_hDBEnv);
	if(SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret){
		_showSQLError( "open (environment handle)", ret, m_hDBEnv, SQL_HANDLE_ENV);
		close();
		return false;
	}
	// minimale ODBC-Version setzen
	ret = SQLSetEnvAttr( m_hDBEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	if(SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret){
		_showSQLError( "open (set ODBC-Version)", ret, m_hDBEnv, SQL_HANDLE_ENV);
		close();
		return false;
	}

	// Connection-Handle holen
	ret = SQLAllocHandle(SQL_HANDLE_DBC, m_hDBEnv, &m_hDBC);
	if(SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret){
		_showSQLError( "open (connection handle)", ret, m_hDBEnv, SQL_HANDLE_ENV);
		close();
		return false;
	}

	// verbinden
	ret = SQLConnect(m_hDBC, (SQLCHAR*)pDB, SQL_NTS, (SQLCHAR*) pUsr, SQL_NTS, (SQLCHAR*) pPwd, SQL_NTS);
	if(SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret){
		_showSQLError( "open (connect)", ret, m_hDBC, SQL_HANDLE_DBC);
		close();
		return false;
	}

	// Statement-Handle holen
	ret = SQLAllocHandle(SQL_HANDLE_STMT, m_hDBC, &m_hStmt);
	if(SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret){
		_showSQLError( "open (statement handle)", ret, m_hDBC, SQL_HANDLE_DBC);
		close();
		return false;
	}
	return true;
}

// gibt alle Handles frei und schlie�t die Verbindung zur Datenbank
void CDatabase::close()
{
	if(m_eState==DB_S_NOTREADY) // Zustand pr�fen
		return;

	if(m_hStmt != NULL){
		SQLFreeHandle(SQL_HANDLE_STMT, m_hStmt);
		m_hStmt=NULL;
	}
	if(m_eState == DB_S_CONNECTED || m_eState == DB_S_FETCH){
		SQLDisconnect(m_hDBC);
	}
	if(m_hDBC != NULL){
		SQLFreeHandle(SQL_HANDLE_DBC, m_hDBC);
		m_hDBC=NULL;
	}
	if(m_hDBEnv != NULL){
		SQLFreeHandle(SQL_HANDLE_ENV, m_hDBEnv);
		m_hDBEnv=NULL;
	}
	m_eState=DB_S_NOTREADY;
	return;
}

// holt einen Datensatz aus der Datenbank
// zuvor muss eine doSelectAction erfolgreich ausgef�hrt worden sein, die die passenden Attribute 
// der Dataset-Klasse mit BindCol an die Attribute der Dataset-Klasse bindet und das DB-Objekt 
// in den Zustand DBFETCH �berf�hrt (sonst stehen keine Datens�tze zur Abholung bereit)
// jeder erfolgreiche fetch-Aufruf �bertr�gt die Daten eines Datensatzes in die gebundenen Attribute 
// des CDataset-Objektes
bool CDatabase::fetch()
{
	if(m_eState!=DB_S_FETCH)		
		return false;

	SQLRETURN ret;
	ret = SQLFetch( m_hStmt);	//Daten holen
	if ((ret == SQL_SUCCESS) || (ret == SQL_SUCCESS_WITH_INFO)) // Daten geholt
		return true;
	else if(ret == SQL_NO_DATA)	// keine Daten da
		return false;
	else						// Fehler
	{
		_showSQLError( "FetchQuery", ret, m_hStmt, SQL_HANDLE_STMT);
		return false;
	}
}

// Zum Abschluss einer Datenbank-Aktion muss das Statement-Handle
// wieder in den Ausgangszustand versetzt werden
void CDatabase::closeQuery( void )
{
	SQLFreeStmt( m_hStmt, SQL_UNBIND);	// Bindungen aufheben
	SQLFreeStmt( m_hStmt, SQL_CLOSE);	// Statement-Handle zur�cksetzen
	m_eState=DB_S_CONNECTED;			// DB-Verbindung ist bereit f�r n�chste DB-Aktion 
}

// Zustand anzeigen
string CDatabase::showState( void )
{
	string statemsg="current state: ";
	switch(m_eState)
	{
	case DB_S_NOTREADY:
		statemsg+= "DB not ready."; break;
	case DB_S_CONNECTED:
		statemsg+= "DB is open."; break;
	case DB_S_FETCH:
		statemsg+= "Performing database action."; break;
	}
	if(m_bPrint)cout << statemsg << endl;
	return statemsg;
}

bool CDatabase::_executeSQLStmt(string sSQLStmt, string errText, bool isSelectStmt)
{
	if(m_bPrint)cout << sSQLStmt << endl;

	SQLRETURN ret;
	ret = SQLExecDirect( m_hStmt, (SQLCHAR*)sSQLStmt.c_str(), SQL_NTS);
	if ((ret == SQL_SUCCESS) || (ret == SQL_SUCCESS_WITH_INFO))
	{
		if(isSelectStmt)
		{
			m_eState=DB_S_FETCH;
			return true;
		}
		else
		{
			closeQuery();
			return true;
		}
	}
	else
	{
		_showSQLError( errText.c_str(), ret, m_hStmt, SQL_HANDLE_STMT);
		closeQuery();
		return false;
	}
}


//**
// private Methoden
//**
int CDatabase::_getNativeErrorCode(SQLRETURN rc, SQLHANDLE hndl, int type)
{
	SQLCHAR SqlState[6], Msg[ SQL_MAX_MESSAGE_LENGTH];
	SQLINTEGER NativeError=-1;
	SQLSMALLINT i=1, MsgLen;

	if (rc == SQL_ERROR)
	{
		/*
		SQLRETURN SQLGetDiagRec(
		SQLSMALLINT     HandleType,		// (in) SQL_HANDLE_ENV, SQL_HANDLE_DBC, SQL_HANDLE_STMT, SQL_HANDLE_DESC
		SQLHANDLE       Handle,			// (in) Handle des Typs
		SQLSMALLINT     RecNumber,		// (in) Nummer des Diagnostik-Datensatzes (bei 1 beginnend)
		SQLCHAR *       SQLState,		// (out) Code f�r die Fehlerursache
		SQLINTEGER *    NativeErrorPtr,	// (out) Fehlercode
		SQLCHAR *       MessageText,	// (out) Fehlertext-Puffer
		SQLSMALLINT     BufferLength,	// (in)  max. Fehlertext-L�nge
		SQLSMALLINT *   TextLengthPtr);	// (out) tats�chliche Fehlertext-L�nge
		*/
		while (SQLGetDiagRec( type, hndl, i, SqlState, &NativeError,
				Msg, sizeof(Msg), &MsgLen) == SQL_NO_DATA) i++;
	}
	return NativeError;
}


// gibt die zu einem Fehlercode passende Fehlermeldung auf dem Bildschirm aus
// infotxt ... freier Text, der z.B. den Namen der Methode enth�lt, in der der Fehler aufgetreten ist
// rc ........ Fehlercode
// hndl ...... Handle, bei dessen Benutzung der Fehler aufgetreten ist
// type ...... Typ des Handle
void CDatabase::_showSQLError( const char* infotxt, SQLRETURN rc, SQLHANDLE hndl, int type)
{
	SQLCHAR SqlState[6], Msg[ SQL_MAX_MESSAGE_LENGTH];
	SQLINTEGER NativeError;
	SQLSMALLINT i, MsgLen;

	if (rc == SQL_ERROR)
	{
		i = 1;
		/*
		SQLRETURN SQLGetDiagRec(
		SQLSMALLINT     HandleType,		// (in) SQL_HANDLE_ENV, SQL_HANDLE_DBC, SQL_HANDLE_STMT, SQL_HANDLE_DESC
		SQLHANDLE       Handle,			// (in) Handle des Typs
		SQLSMALLINT     RecNumber,		// (in) Nummer des Diagnostik-Datensatzes (bei 1 beginnend)
		SQLCHAR *       SQLState,		// (out) Code f�r die Fehlerursache
		SQLINTEGER *    NativeErrorPtr,	// (out) Fehlercode
		SQLCHAR *       MessageText,	// (out) Fehlertext-Puffer
		SQLSMALLINT     BufferLength,	// (in)  max. Fehlertext-L�nge
		SQLSMALLINT *   TextLengthPtr);	// (out) tats�chliche Fehlertext-L�nge
		*/
		while (SQLGetDiagRec( type, hndl, i, SqlState, &NativeError,
				Msg, sizeof(Msg), &MsgLen) != SQL_NO_DATA)
		{
			m_lasterrmsg=string(infotxt) + "  -- SQL ERROR: " + string((char*)SqlState) + " " + to_string(NativeError) + " " + string((char*)Msg);
			if(m_bPrint)cout << m_lasterrmsg << endl;
			i++;
		}
	}
	else
	{
		if( rc == SQL_INVALID_HANDLE)
		{
			m_lasterrmsg=string(infotxt) + " -- invalid Handle " + to_string((int)((size_t)hndl));
			if(m_bPrint)cout << m_lasterrmsg << endl;
		}
	}
}

string CDatabase::getSQLErrorMsg() {
	return m_lasterrmsg;
}

void CDatabase::allowPrint(bool bPrint) {
	m_bPrint=bPrint;
}
