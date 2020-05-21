#include <iostream>
#include <stdlib.h>
using namespace std;

#include "portaudio.h"
#include "CPortAudioOutputStream.h"


CPortAudioOutputStream::CPortAudioOutputStream()
{
	// TODO Auto-generated constructor stub
	AudioStreamState=AUDIOSTREAM_S_NOT_READY;
	m_stream=NULL;
	m_error=0;
	m_databuf=NULL;
	m_framesPerBlock=0;
}

CPortAudioOutputStream::~CPortAudioOutputStream()
{
	// TODO Auto-generated destructor stub
	close();
}

void CPortAudioOutputStream::open(long samplerate, int channelcnt, int blockcnt, float* databuf, long sbufsize)
{
	long m_samplerate=samplerate;
	int m_channelcnt=channelcnt;
	int m_blockcnt=blockcnt;
	m_databuf=databuf;
	m_framesPerBlock=m_samplerate/m_blockcnt; 	// frames per buffer (a frame includes all samples that have to be played simultaneously)
	long m_sbufsize=sbufsize;


	m_error = Pa_Initialize();						// init PortAudio API

	if(m_error==paNoError)
	{
		// the computers default output device should work
		m_outputParameters.device = Pa_GetDefaultOutputDevice();

		if (m_outputParameters.device != paNoDevice)
		{
			// set appropriate output parameters
			m_outputParameters.channelCount = m_channelcnt;	// channels
			m_outputParameters.sampleFormat = paFloat32;  // 32 bit floating point output
			//Default latency values for robust non-interactive applications (e.g. playing sound files)
			//(is ignored ??)
			m_outputParameters.suggestedLatency = Pa_GetDeviceInfo( m_outputParameters.device )->defaultHighOutputLatency;
			m_outputParameters.hostApiSpecificStreamInfo = NULL;	// not necessary
			m_error = Pa_OpenStream(&m_stream, NULL, /* no input */&m_outputParameters, m_samplerate, m_framesPerBlock,
								paClipOff,	/* we won't output out of range samples so don't bother clipping them */
								NULL, 		/* no callback, use blocking API */
								NULL ); 	/* no callback, so no callback userData */

			if( m_error == paNoError )
			{
				AudioStreamState=AUDIOSTREAM_S_READY;
				const PaStreamInfo* pStreamInfo= Pa_GetStreamInfo(m_stream);
				cout << " Stream properties ";
				cout  << " output latency= " << pStreamInfo->outputLatency;
				cout << " sample rate= " << pStreamInfo->sampleRate << endl;
			}
			m_error= Pa_StartStream(m_stream);
		}
	}
}
void CPortAudioOutputStream::close()
{
	if(AudioStreamState!=AUDIOSTREAM_S_NOT_READY){
		Pa_CloseStream(m_stream);
		Pa_Terminate(); // terminate the API only if you have initialized it successfully
	}
}


void CPortAudioOutputStream::start()
{
	long framesPerBlock=m_framesPerBlock; 	// frames per buffer (a frame includes all samples that have to be played simultaneously)
	float* sbufBlock=m_databuf;	// pointer to the next block to play
	m_error = Pa_WriteStream( m_stream, m_databuf, framesPerBlock);
	if( m_error != paNoError )
	{
		Pa_CloseStream(m_stream);
		AudioStreamState=AUDIOSTREAM_S_READY;
	}
}

void CPortAudioOutputStream::resume()
{
	start();
}

void CPortAudioOutputStream::stop()
{
	if(AudioStreamState==AUDIOSTREAM_S_PLAYING)
	{
		Pa_StopStream(m_stream);
		AudioStreamState=AUDIOSTREAM_S_READY;
	}
}
void CPortAudioOutputStream::pause()
{
	stop();
}
