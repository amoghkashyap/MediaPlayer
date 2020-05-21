/*
 * CAudioPlayerController.h
 *
 *  Created on: 09.01.2020
 *      Author: Wirth
 */

#ifndef SRC_CAUDIOPLAYERCONTROLLER_H_
#define SRC_CAUDIOPLAYERCONTROLLER_H_
#include "CFile.h"
#include "CSoundFile.h"

#include "CFilterDB.h"
#include "CFilter.h"
#include "CUserInterface.h"
#include "CPortAudioOutputStream.h"
#include "CSoundDB.h"

class CAudioPlayerController {
private:

	CUserInterface* m_ui;
	CFilterDB m_filterColl;
	CFilter* m_pFilter;

	CSoundFile* m_pSFile;
	// toDo add the other parts of the controller (see UML Class diagram)
	CSoundDB m_soundColl;
	CPortAudioOutputStream m_audiostream;

public:
	CAudioPlayerController();
	~CAudioPlayerController();
	void run(CUserInterface* pui);

private:
	void init();
	void chooseFilter();
	void manageFilterCollection();
	// toDo add the other methods of the controller (see UML class diagram)
	void manageSoundCollection();
	void play();
	void chooseSoundFile();
	void chooseAmplitudeScaling();
};

#endif /* SRC_CAUDIOPLAYERCONTROLLER_H_ */
