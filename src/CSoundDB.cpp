/*
 * CSoundDB.cpp
 *
 *  Created on: 18-Jan-2020
 *      Author: chitt
 */

#include "CSoundDB.h"

CSoundDB::CSoundDB() {
	// TODO Auto-generated constructor stub
	m_soundfolder[0]=0;
	m_soundfilename[0]=0;
	m_soundfileid=0;
	m_soundfs=0;
	m_soundchannels=0;
	m_soundfilescount=0;
}

CSoundDB::~CSoundDB() {
	// TODO Auto-generated destructor stub
}

int CSoundDB::getSoundFileID(){
	return m_soundfileid;
}

char* CSoundDB::getSoundFolder(){
	return m_soundfolder;
}
char* CSoundDB::getsoundFileName(){
	return m_soundfilename;
}
int CSoundDB::getSoundFs(){
	return m_soundfs;
}
int CSoundDB::getSoundChannels(){
	return m_soundchannels;
}
int CSoundDB::getSoundFilesCount(){
	return m_soundfilescount;
}
bool CSoundDB::findSoundFilesCount(){
	if(m_eState!=DB_S_CONNECTED)
			return false;

		string sSQL;
		sSQL="SELECT count(*) FROM soundfiles;";
		if(_executeSQLStmt(sSQL, "SelectCountOfSoundfiles", true))
		{
			SQLBindCol( m_hStmt, 1, SQL_C_LONG, &m_soundfilescount, 0, &m_info );
			return true;
		}
		else
			return false;
}
bool CSoundDB::selectAllSoundColumns()
{
	if(m_eState!=DB_S_CONNECTED)
		return false;

	string sSQL;
	sSQL="SELECT * FROM soundfiles;";
	if( _executeSQLStmt(sSQL, "selectAllSoundColumns ", true) )
	{
		SQLBindCol( m_hStmt, 1, SQL_C_LONG, &m_soundfileid, 0, &m_info );
		SQLBindCol( m_hStmt, 2, SQL_C_CHAR, m_soundfolder, DB_TEXTLEN, &m_info );
		SQLBindCol( m_hStmt, 3, SQL_C_CHAR, m_soundfilename, DB_TEXTLEN, &m_info );
		SQLBindCol( m_hStmt, 4, SQL_C_LONG, &m_soundfs, 0, &m_info );
		SQLBindCol( m_hStmt, 5, SQL_C_LONG, &m_soundchannels, 0, &m_info );
		return true;
	}
	else
		return false;
}

bool CSoundDB::selectSoundData(int soundfileid)
{
	if(m_eState!=DB_S_CONNECTED)
		return false;

	string sSQL;
	sSQL= "SELECT soundfolder,soundfilename,soundfs,soundchannels FROM soundfiles WHERE soundfileid=" + to_string(soundfileid);
	if( true == _executeSQLStmt(sSQL, "selectFs for given soundFileID", true))
	{

				SQLBindCol( m_hStmt, 1, SQL_C_CHAR, m_soundfolder, DB_TEXTLEN, &m_info );
				SQLBindCol( m_hStmt, 2, SQL_C_CHAR, m_soundfilename, DB_TEXTLEN, &m_info );
				SQLBindCol( m_hStmt, 3, SQL_C_LONG, &m_soundfs, 0, &m_info );
				SQLBindCol( m_hStmt, 4, SQL_C_LONG, &m_soundchannels, 0, &m_info );
				bool a=fetch();
				closeQuery();
				return a;
	}
	else
		return false;
}

bool CSoundDB::insertSound(string soundfilefolder, string soundfilename, int soundfs, int soundchannels)
{
	if(m_eState!=DB_S_CONNECTED)
		return false;

	//bool bPrintStmts=true;

	string sSQL;
	// insertdata into soundfiles
	sSQL= "INSERT INTO soundfiles (soundfolder,soundfilename,soundfs,soundchannels) VALUES (\'" + soundfilefolder + "\',\'" + soundfilename + "\', "+ to_string(soundfs) + "," + to_string(soundchannels) +");";
	_executeSQLStmt(sSQL, "insertSoundFileData", false);

	return true;
}

