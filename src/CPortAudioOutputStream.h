#ifndef CAUDIOOUTPUTSTREAM_H_
#define CAUDIOOUTPUTSTREAM_H_

#include <iostream>
#include <stdlib.h>

#include "portaudio.h"

enum CAUDIO_STREAM_STATE{AUDIOSTREAM_S_READY,AUDIOSTREAM_S_NOT_READY, AUDIOSTREAM_S_PLAYING};

class CPortAudioOutputStream
{
	CAUDIO_STREAM_STATE AudioStreamState;
	PaStreamParameters m_outputParameters; 	// desired configuration of the stream
	PaStream* m_stream;					 	// we'll get the address of the stream from Pa_OpenStream
	PaError m_error;						// PortAudio specific error values
	float* m_databuf;
	long m_framesPerBlock;

public:
	CPortAudioOutputStream();
	virtual ~CPortAudioOutputStream();
	void open(long samplerate,int channelcnt, int blockcnt, float* databuf, long sbufsize);
	void close();
	void start();
	void resume();
	void stop();
	void pause();
};

#endif
