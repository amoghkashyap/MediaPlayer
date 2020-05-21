/*
 * CSoundDB.h
 *
 *  Created on: 18-Jan-2020
 *      Author: chitt
 */

#ifndef CSOUNDDB_H_
#define CSOUNDDB_H_

#include <string>
using namespace std;
#include "CDatabase.h"

class CSoundDB : public CDatabase {
private:
		char m_soundfolder[DB_TEXTLEN];		// folder where sound file exist
		char m_soundfilename[DB_TEXTLEN];	// name of sound file
		int m_soundfileid;				// sound file ID from Data base
		int m_soundfs;				//sampling frequency of sound file
		int m_soundchannels;				// number of channels of a sound file
		int m_soundfilescount;
public:
	CSoundDB();
	virtual ~CSoundDB();
			int getSoundChannels();			// get number of sound channels
			int getSoundFs();				// sound file sampling frequency
			char* getsoundFileName();		// gets sound file name
			char* getSoundFolder();			// gets folder name of sound file
			int getSoundFileID();			// sound file ID in DB
			bool selectAllSoundColumns();
			bool selectSoundData(int soundfileid);
			bool insertSound(string soundfilefolder, string soundfilename, int soundfs, int soundchannels);
			bool findSoundFilesCount();
			int getSoundFilesCount();

};

#endif /* CSOUNDDB_H_ */
