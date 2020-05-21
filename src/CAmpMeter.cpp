/*
 * CAmpMeter.cpp
 *
 *  Created on: 21.10.2019
 *      Author: Wirth
 */
#include <iostream>
//#include <stdlib.h>
#include <math.h>
using namespace std;

#include "CIOWarrior.h"
#include "CAmpMeter.h"

CAmpMeter::CAmpMeter()
{
	// todo: initialize all attributes (see UML class diagram)
	m_scmode = SCALING_MODE_LIN;
	m_scMax = 0;
	std::fill(m_thresholds, m_thresholds+8, 0);
	m_iowdev = NULL;
}

CAmpMeter::~CAmpMeter()
{

}

void CAmpMeter::init(float min, float max, SCALING_MODES scmode, int logScaleMin,CIOWarrior* iowdev)
{
	// todo: initialize the amplitude meter on the basis of the given parameter values
	// The maximum absolute value of min and max is taken for the maximum of the scale.
	// The thresholds are calculated for a LED line with 8 LEDs in dependence of the given scaling mode (scmode).
		// Linear scaling: 0 ... maximum of the scale
		// Logarithmic scaling: logScaleMin ... 0 [dB], logScaleMin must be negative
	m_iowdev = iowdev;
		m_scMax = fabs(min)<fabs(max)?fabs(max):fabs(min);
		m_scmode=scmode;
		switch (scmode)
		{
			case SCALING_MODE_LIN:
				for(int i=0;i<8;i++)
					{
						m_thresholds[i]=i*m_scMax/7;
					}
			break;
			case SCALING_MODE_LOG:
				if(logScaleMin>0) logScaleMin = -logScaleMin;
				for(int i=0;i<8;i++)
					{
						m_thresholds[i]=logScaleMin-(float)i*logScaleMin/7;
					}
			break;
		}
}

bool CAmpMeter::write(float* databuf, unsigned long databufsize)
{
	if(NULL == databuf || NULL==m_iowdev)return false;
	return write(_getValueFromBuffer(databuf, databufsize));
}

bool CAmpMeter::write(float data)
{
	// todo:
	// if an IOWarrior is connected it gets the appropriate bar pattern and writes it to the IOWarrior
	// otherwise it does nothing
		if(!m_iowdev->open())return false;
		unsigned char pat=_getBarPattern(data);
		if(!m_iowdev->write(pat))return false;
		return true;
}

void CAmpMeter::print(float data)
{
	//todo:
	// get the appropriate bar pattern
	// print the bar pattern in binary format on the screen (see CIOWarrior::printData() method for an example)
	unsigned char pat = _getBarPattern(data);
	for(int i=0;i<8;i++)
	{
		pat & 0x80 ? cout << '1' : cout << '0';
		pat <<= 1;
	}
	cout << '\r';
}

void CAmpMeter::print(float* databuf, unsigned long databufsize)
{
	if(NULL == databuf)
	{
		cout << "no data for printing amplitude bar!" << endl;
		return;
	}
	return print(_getValueFromBuffer(databuf, databufsize));
}


unsigned char CAmpMeter::_getBarPattern(float data)
{
	char pat=0;
	// todo: Calculate appropriate bar pattern pat for data. Data is a linear value in any case. The bar pattern may be used
	// for visualization on the screen or the LED line.
	// Example: pat is 0b11111111 if the absolute value of data is equal to the maximum data value (m_scMax)
	// In dependence of the scaling mode (m_scmode), the absolute value of data (linear scaling) or
	// the dB value of the absolute value of data (logarithmic scaling) has to be used for the bar pattern calculation.
	// Before calculating the dB value, the absolute value of data shall be divided by the linear scale maximum to
	// adjust the highest value to 0 dB (this is called peak normalization).
	switch (m_scmode)
		{
			case SCALING_MODE_LIN:
				for(int i=0;i<8;i++)
					{
						if(m_thresholds[i]<=fabs(data))
						{
							pat =pow(2,i+1)-1;
						}
					}
			break;
			case SCALING_MODE_LOG:
				for(int i=0;i<8;i++)
					{
						if(m_thresholds[i]<=20*log(fabs(data)/m_scMax))
						{
							pat =pow(2,i+1)-1;
						}
					}
			break;
		}
	return pat;
}


float CAmpMeter::_getValueFromBuffer(float* databuf, unsigned long databufsize)
{
	float dmax=0., d;
	unsigned long imax=0;
	for(unsigned long i=0; i < databufsize; i++)
	{
		d=fabs(databuf[i]);
		if(dmax < d)
		{
			dmax=d;
			imax=i;
		}
	}
	return databuf[imax];
}
