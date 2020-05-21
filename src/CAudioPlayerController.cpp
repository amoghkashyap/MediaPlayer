/*
 * CAudioPlayerController.cpp
 *
 *  Created on: 09.01.2020
 *      Author: Wirth
 */
////////////////////////////////////////////////////////////////////////////////
// Header
#define USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>			// functions to scan files in folders (used in Lab04prep_DBAdminInsert)
using namespace std;

#include "CASDDException.h"


#include "CFile.h"
#include "CSoundFile.h"

#include "CFilterDB.h"
#include "CFilter.h"

#include "CUserInterface.h"
#include "CAudioPlayerController.h"

CAudioPlayerController::CAudioPlayerController() {
	m_pSFile=NULL;		// association with 1 or 0 CSoundFile-objects
	m_pFilter=NULL;		// association with 1 or 0 CFilter-objects
	m_ui=NULL;			// association with 1 or 0 CUserInterface-objects
}

CAudioPlayerController::~CAudioPlayerController() {
	if(m_pSFile)delete m_pSFile;
	if(m_pFilter)delete m_pFilter;
}

void CAudioPlayerController::run(CUserInterface* pui) {
	// if an exception has been thrown by init, the user is not able to use the player
	// therefore the program is terminated (unrecoverable error)
	try
	{
		m_ui=pui;	// set the current user interface (given by the parameter)
		init();		// initialize the components of the controller, if possible and necessary
	}
	catch(CASDDException& e)
	{
		string eMsg="Error from: ";
		m_ui->showMessage(eMsg+e.getSrcAsString()+""+e.getErrorText());
		return;
	}

	//////////////////////////////////////////////////
	// main menue of the player
	// todo: Add further menu items, the corresponding cases and method calls to the following code
	// note: the last item of the menu must be empty (see CUserInterfaceCmdIOW code)
	string mainMenue[]={"select sound","select filter","select Amplitude Scaling","play the sound","manage Sound Collection","manage filter collection", "terminate player", ""};
	while(1)
	{
		// if an exception will be thrown by one of the methods, the main menu will be shown
		// after an error message has been displayed. The user may decide, what to do (recoverable error)
		// For instance, if the user selects a filter without having selected a sound file before, an error
		// message tells the user to select a sound file. He/She may decide to do this and then select a filter
		// once more. In this case, the error condition is eliminated and the program may continue regularly.
		try
		{
			// display the menu and get the user's choice
			int selitem=m_ui->getListSelection(mainMenue);
			// process the user's choice by calling the appropriate CAudioPlayerControllerMethod
			switch(selitem)
			{
			case 0: chooseSoundFile();break;
			case 1: chooseFilter();break;
			case 2: chooseAmplitudeScaling();break;
			case 3: play();break;
			case 4: manageSoundCollection();break;
			case 5: manageFilterCollection();break;
			default: return;
			}
		}
		catch(CASDDException& e)
		{
			string eMsg="Error from: ";
			m_ui->showMessage(eMsg+e.getSrcAsString()+""+e.getErrorText());
		}
	}
}

void CAudioPlayerController::init()
{
	m_ui->init();
	// no printing - the controller is the only object which may initiate
	// printing via the view object (MVC design)
	m_filterColl.allowPrint(false);
	if(false == m_filterColl.open("AudioFilterCollectionDB","root"))
		throw CASDDException(SRC_Database,-1,m_filterColl.getSQLErrorMsg());
	///////////////////
	// todo: initialize other attributes if necessary
		// toDo add the other parts of the controller (see UML Class diagram)
		 m_soundColl.allowPrint(false);
		 if(false == m_soundColl.open("SoundCollectionDB","root"))
		 		throw CASDDException(SRC_Database,-1,m_soundColl.getSQLErrorMsg());

}

void CAudioPlayerController::chooseFilter()
{
	if(!m_pSFile) // a sound file must have been created by the chooseSoundFile method before
	{
		m_ui->showMessage("Error from selectFilter: No sound file. Select sound file before filter!");
		return;
	}
	// get the sampling rate from the current sound file
	int fs=m_pSFile->getSampleRate();
	cout<<"hello"<<fs<<endl;
	/////////////////////////////////////
	// list the appropriate filters for the sound
	int numflt=m_filterColl.selectNumFilters(fs); 	// get the number of appropriate filter files
	if(numflt)										// if there are filters that fit
	{
		// prepare a string array for the user interface, that will contain  a menu with the selection of filters
		// there is place for an additional entry for an unfiltered sound and an empty string
		string* pFlt=new string[numflt+2];
		// prepare an integer array for the corresponding filter IDs to pass them to the user interface as well
		// there is place for -1 (unfiltered sound)
		int* pFIDs=new int[numflt+1];

		// select the appropriate filters
		m_filterColl.selectFilters(fs);

		for(int i=0; i < numflt; i++)
		{
			m_filterColl.fetch();	// get a record of filter data
			// instead to print the filter data, the will be inserted into the string array and the filter ID array
			pFIDs[i]= m_filterColl.getFilterID();
			pFlt[i]= m_filterColl.getFilterType() + "/" + m_filterColl.getFilterSubType()
				   + ", order=" + to_string(m_filterColl.getOrder()) + "/delay="
				   + to_string(m_filterColl.getDelay()) + "s]: "+ m_filterColl.getFilterInfo();
		}
		m_filterColl.closeQuery();

		// add the last menu entry for the choice of an unfiltered sound
		pFIDs[numflt]=-1;
		pFlt[numflt]="-1 [unfiltered sound]";

		// pass the arrays to the user interface and wait for the user's input
		// if the user provides a filterID which is not in pFIDs, the method returns
		// CUI_UNKNOWN
		int fid=m_ui->getListSelection(pFlt,pFIDs);

		// destroy the arrays
		delete[]pFlt;
		delete[]pFIDs;

		/////////////////////////////////////
		// create a filter according to the user's choice
		if(fid != CUI_UNKNOWN)
		{
			if(fid>=0)		// the user has chosen a filter from the filter collection
			{
				// get the filter's data
				if(true == m_filterColl.selectFilterData(fid))
				{
					// if there was a filter object from a preceding choice of the user, delete this
					if(m_pFilter)delete m_pFilter;
					// create filter
					m_pFilter=new CFilter(m_filterColl.getACoeffs(), m_filterColl.getBCoeffs(), m_filterColl.getOrder(), m_filterColl.getDelay(), m_pSFile->getSampleRate(), m_pSFile->getNumChannels());
				}
				else
				{
					// wrong ID (may only accidently happen - logical error in the program?)
					m_ui->showMessage("Error from selectFilter: No filter data available! Did not change filter. ");
				}
			}
			else	// the user has chosen not to filter the sound
			{
				if(m_pFilter)	// if there was a filter object from a preceding choice of the user
				{
					delete m_pFilter;	// ... delete this
					m_pFilter=NULL;		// currently we have no filter
					m_ui->showMessage("Message from selectFilter: Filter removed. ");
				}
			}
		}
		else
			m_ui->showMessage("Error from selectFilter: Invalid filter selection! Play unfiltered sound. ");
	}
	else
		m_ui->showMessage("Error from selectFilter: No filter available! Play unfiltered sound. ");
}


void CAudioPlayerController::manageFilterCollection()
{
	// user input for filter file path
	string fltfolder;
	m_ui->showMessage("Enter filter file path: ");
	fltfolder=m_ui->getUserInputPath();

	//////////////////////////////////////////
	// Code from Lab04prep_DBAdminInsert and Lab04prep_insertFilterTest
	// iterates through the folder that the user entered and inserts all
	// the filters it finds in the folder (reading txt files)
	 dirent* entry;
	 DIR* dp;
	 string fltfile;

	 dp = opendir(fltfolder.c_str());
	 if (dp == NULL)
	 {
		 m_ui->showMessage("Could not open filter file folder.");
		 return;
	 }

	 while((entry = readdir(dp)))
	 {
		 fltfile=entry->d_name;
		 m_ui->showMessage("Filter file to insert into the database: " + fltfolder+fltfile+":");


		 if(fltfile.rfind(".txt")!=string::npos)// txt file?
		 {
			const int rbufsize=100;					// assume not more than 100 different sampling frequencies
			char readbuf[rbufsize];

			// get all sampling frequencies contained in the file
			int numFs=rbufsize;
			int fsbuf[rbufsize];
			CFilterFile::getFs(fltfolder+fltfile,fsbuf,numFs);

			for(int i=0; i < numFs;i++)	// iterate through all found fs
			{
				CFilterFile ff(fsbuf[i], (fltfolder+fltfile).c_str(), FILE_READ); // create a file object for a certain fs
				ff.open();
				if(ff.read(readbuf,rbufsize))	// read information about the filter with the fs
				{
					// send information to the user interface to display it
					string fileinfo = "Inserting filter file: " + ff.getFilterType() + "/" + ff.getFilterSubType() + " filter [order=" + to_string(ff.getOrder())
							        + ", delay=" + to_string(ff.getDelay()) + "s, fs=" + to_string(fsbuf[i]) + "Hz] " + ff.getFilterInfo();
					m_ui->showMessage(fileinfo);

					// insert the filter into the filter collection database
					if( false == m_filterColl.insertFilter(ff.getFilterType(),ff.getFilterSubType(),
									  fsbuf[i],ff.getOrder(),1000.*ff.getDelay(),ff.getFilterInfo(),
									  ff.getBCoeffs(),ff.getNumBCoeffs(),ff.getACoeffs(),ff.getNumACoeffs()))
						m_ui->showMessage(m_filterColl.getSQLErrorMsg()); // if error, let the user interface show the error message
				}
				else
					m_ui->showMessage("No coefficients available for fs=" + to_string(fsbuf[i]) + "Hz");
				ff.close();
			}
		 }
		 else
			 m_ui->showMessage(" irrelevant file of other type or directory");
	 }
	 closedir(dp);
}

void CAudioPlayerController::chooseAmplitudeScaling(){
	string scalingMenu[]={"Linear Scaling","Logarithmic Scaling",""};
	try
	{
		// display the menu and get the user's choice
		int selitem=m_ui->getListSelection(scalingMenu);
		// process the user's choice by calling the appropriate CAudioPlayerControllerMethod
		switch(selitem)
		{
		case 0: m_ui->setAmplitudeScaling(SCALING_MODE_LIN);break;
		case 1: m_ui->setAmplitudeScaling(SCALING_MODE_LOG);break;
		default: m_ui->showMessage("Choosen Wrong input.Please try again"); return;
		}
		m_ui->showMessage("You have succesfully selected the Amplitude scale for your Player ");
	}
	catch(CASDDException& e)
	{
		string eMsg="Error from: ";
		m_ui->showMessage(eMsg+e.getSrcAsString()+""+e.getErrorText());
	}

}
void CAudioPlayerController::chooseSoundFile(){
	m_ui->showMessage("please select a sound file to get its frequency");
	m_soundColl.findSoundFilesCount();
	m_soundColl.fetch();
	int numsnd=m_soundColl.getSoundFilesCount();// get the number of sound files
	m_soundColl.closeQuery();
	if(numsnd)	// if there are sound files
	{
		// prepare a string array for the user interface, that will contain  a menu with the selection of a sound file
		// there is place for an additional entry for an empty string
		string* pSnd=new string[numsnd+1];
		// prepare an integer array for the corresponding sound file IDs to pass them to the user interface as well
		int* pSIDs=new int[numsnd];
		// select all the sound files
		m_soundColl.selectAllSoundColumns();
		for(int i=0; i < numsnd; i++)
		{
			m_soundColl.fetch();	// get a record of sound files
			// instead to print the sound files data, we will be inserting into the string array and the sound ID array
			pSIDs[i]= m_soundColl.getSoundFileID();
			pSnd[i]=to_string(m_soundColl.getSoundFileID()) + "/" + m_soundColl.getsoundFileName();;
		}
		m_soundColl.closeQuery();
		// pass the arrays to the user interface and wait for the user's input
		// if the user provides a soundFileID which is not in pSIDs, the method returns
		// CUI_UNKNOWN
		int fid=m_ui->getListSelection(pSnd,pSIDs);
		// destroy the arrays
		delete[]pSnd;
		delete[]pSIDs;
		if(fid!=CUI_UNKNOWN){
			m_soundColl.selectSoundData(fid);
			string sndfolder=m_soundColl.getSoundFolder();
			string sndfile=m_soundColl.getsoundFileName();
			string fsMsg= "selected sound is : " +sndfile + " and its Frequency is : " + to_string(m_soundColl.getSoundFs());
			m_ui->showMessage(fsMsg);
			string sndfile_r=sndfolder+sndfile;
			if(m_pSFile)delete m_pSFile;
			// create Sound Object
			m_pSFile=new CSoundFile(sndfile_r.c_str(),FILE_READ);
			m_pSFile->open();
			m_ui->showMessage("You can now select a filter or play the Audio");
		}
		else
			m_ui->showMessage("You have given an wrong input which doesn't exist. Please try once again by choosing a valid input");
	}
	else{
		m_ui->showMessage("No Sound Files Available");
	}
}
void CAudioPlayerController::manageSoundCollection(){
	m_ui->showMessage("method->manageSoundCollection : You are inside this !. ");
	m_ui-> showMessage("Please give the path of the folder where the sound files exist : ");
	string sndfolder=m_ui->getUserInputPath();
	m_ui->showMessage("path given by user is:"+sndfolder);
	dirent* entry;
	DIR* dp;
	string sndfile;
	dp = opendir(sndfolder.c_str());
	if (dp == NULL) throw CASDDException(SRC_File,-1,"Could not open filter file folder.");
	try
	{
		while((entry = readdir(dp)))
		{
			sndfile=entry->d_name;
			m_ui-> showMessage(sndfile + ":");
			if(sndfile.rfind(".wav")!=string::npos)
			{
				m_ui->showMessage(" sound file to insert into the database.");
				string soundfolder = sndfolder.c_str()+sndfile;
				m_ui->showMessage(soundfolder);
				m_pSFile=new CSoundFile(soundfolder.c_str(),FILE_READ);
				m_pSFile->open();
				m_soundColl.insertSound(sndfolder ,sndfile.c_str(),m_pSFile->getSampleRate(),m_pSFile->getNumChannels());

			}
			else
				m_ui -> showMessage( " irrelevant file of other type or directory");
		}
		m_ui -> showMessage("opeartion successfully completed");
		closedir(dp);
	}
	catch(CASDDException& e)
	{
		e.print();
	}
}

void CAudioPlayerController::play(){
	if(m_pSFile!=NULL)
	{
			float* sbuf;
			float* sbuf1;
			bool button=true;
						// configure sample buffer

						sf_count_t framesPerBlock=(int)(m_pSFile->getSampleRate()/8); // 1s/8=125ms per block
						if((m_filterColl.getOrder()==0) && (m_pFilter!=NULL))
							framesPerBlock=(int)(2*m_pFilter->getOrder()); // For Delay Filter
						sf_count_t sblockSize=m_pSFile->getNumChannels()*framesPerBlock; // total number of samples per block
						sbuf=new float[sblockSize];
						sbuf1=new float[sblockSize];
						sf_count_t readSize=m_pSFile->read(sbuf,framesPerBlock);
						if(m_pFilter != NULL)
							m_audiostream.open(m_pSFile->getSampleRate(),m_pSFile->getNumChannels(),8,sbuf1,readSize);
						else
							m_audiostream.open(m_pSFile->getSampleRate(),m_pSFile->getNumChannels(),8,sbuf,readSize);
						m_ui->showMessage("Press button or enter key to start, pause and resume play back");
													while(true) //loop to test pause for max times
													{
														m_ui->wait4Key(button);
														//button pressed... start playing audio  and catch next button press.
															m_ui->showMessage("Press button to stop again");
															button=false;
														while(button==false) // loop till next button press
														{
													//playing audio file
													while( framesPerBlock == readSize)
													{
														if(m_pFilter != NULL)
															m_pFilter->filter(sbuf,sbuf1,framesPerBlock);
														m_audiostream.start();
														m_ui->showAmplitude(sbuf, sblockSize);
														readSize=m_pSFile->read(sbuf, framesPerBlock);
														//m_ui->wait4Key(button);
														button=m_ui->wait4Key(button);
														if(button==true)
														{
															m_ui->showMessage("Press button to resume!!");
															break;
														}
													}
													m_audiostream.pause();
													if(framesPerBlock != readSize)break; //break button loop
												}
												if(framesPerBlock != readSize)break;//break for loop
											}
						delete[]sbuf;
						delete[]sbuf1;
						//	m_audiostream.close();
						m_pSFile->rewind();
}
	else
		m_ui->showMessage("No sound file is available to play.. pLease choose a sound file");
}


