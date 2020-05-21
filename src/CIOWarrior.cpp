/*
 * CIOWarrior.cpp
 *
 *  Created on: 20.09.2019
 *      Author: Wirth
 */
/**
 * \file CIOWarrior.cpp
 *
 * \brief implementation of the IOWarrior control class (performs output only)
 */
#include <iostream>
using namespace std;

#include "CIOWarrior.h"

CIOWarrior::CIOWarrior()
{
	m_handle=NULL;
	m_lastError=IOW_E_OK;
	m_state=IOW_S_NOTREADY;
}

CIOWarrior::~CIOWarrior()
{
	close();
}

bool CIOWarrior::open()
{
	// evaluate state
	if(m_state == IOW_S_READY)
		return true;

	// call the API open function
	m_handle=IowKitOpenDevice();
	if(m_handle == NULL)
	{
		m_lastError=IOW_E_NODEVICE;	// set error value
		return false;
	}
	// call the API get product ID function
	if(IowKitGetProductId(m_handle) != IOWKIT_PRODUCT_ID_IOW40)
	{
		// set error value
		m_lastError=IOW_E_INVALIDDEVICE;	// set error value
		// close the device
		IowKitCloseDevice(m_handle);
		return false;
	}
	// try to set all IOWarrior port pins to 1
	IOWKIT40_IO_REPORT myreport;
	myreport.Value=0xffffffff;
	if( IowKitWrite(m_handle, IOW_PIPE_IO_PINS, (PCHAR)&myreport, IOWKIT40_IO_REPORT_SIZE)!=IOWKIT40_IO_REPORT_SIZE)
	{
		IowKitCloseDevice(m_handle);
		m_lastError=IOW_E_WRITEERROR;
		return false;
	}
	// set device state to ready
	m_state=IOW_S_READY;
	// set error to ok
	m_lastError=IOW_E_OK;
	return true;
}

void CIOWarrior::close()
{
	if(m_state == IOW_S_NOTREADY)
		return;
	IowKitCloseDevice(m_handle);
	m_state = IOW_S_NOTREADY;
}


bool CIOWarrior::write(unsigned char data)
{
	// check the state
	if(m_state != IOW_S_READY)
	{
		m_lastError=IOW_E_DEVICENOTREADY;
		return false;
	}
	// define a report structure variable
	IOWKIT40_IO_REPORT myreport;
	myreport.ReportID=0; 				// plain io ID
	myreport.Value = 0xFFFFFFFF;
	myreport.Bytes[3]=~data;			// 0-bit switches LED on, 1-bit off

	int res=IowKitWrite(m_handle, IOW_PIPE_IO_PINS, (char*)&myreport, sizeof(myreport));
	if(res != sizeof(myreport))
	{
		m_lastError=IOW_E_WRITEERROR;
		close();
		return false;
	}
	return true;
}

void CIOWarrior::printData(unsigned char cByte)
{
	for(int i=0;i<8;i++)
	{
		cByte & 0x80 ? cout << '1' : cout << '0';
		cByte <<= 1;
	}
	cout << '\r';
}

void CIOWarrior::printState()
{
	switch(m_state)
	{
	case IOW_S_NOTREADY: cout << "device is not ready" << endl; break;
	case IOW_S_READY: cout << "device is ready" << endl; break;
	default: cout << "Unknown State" << endl; break;
	}
	switch(m_lastError)
	{
	case IOW_E_NODEVICE: cout << "Error: no device" << endl; break;
	case IOW_E_INVALIDDEVICE: cout << "Error: Invalid device" << endl; break;
	case IOW_E_WRITEERROR: cout << "Error: could not write" << endl; break;
	case IOW_E_DEVICENOTREADY: cout << "Error: please open the device first" << endl; break;
	case IOW_E_OK: cout << "No Error :-)" << endl; break;
	default: cout << "Unknown Error" << endl; break;
	}
}

IOW_STATES CIOWarrior::getState()
{
	return m_state;
}

bool CIOWarrior::keyPressed()
{
	if(m_state != IOW_S_READY)
		{
			m_lastError=IOW_E_DEVICENOTREADY;
			return false;
		}

		IOWKIT40_IO_REPORT report;

		// Read IO pins of IO-Warrior 40

		ULONG res;
		res=IowKitReadNonBlocking(m_handle, IOW_PIPE_IO_PINS, (char*)&report, IOWKIT40_IO_REPORT_SIZE);
		if(res==0)
		{
			//state of the switch has not changed.. hence no report.. should return false
			return false;
		}
		if (report.Bytes[0] == 0xff) return false;
		while (report.Bytes[0] == 0xfe)
		{
			IowKitReadNonBlocking(m_handle, IOW_PIPE_IO_PINS, (char*)&report, IOWKIT40_IO_REPORT_SIZE);
			if(res == 0)
				return false;
		}
		return true;
}

