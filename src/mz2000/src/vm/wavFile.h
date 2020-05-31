/*
	Author : shikarunochi
	Date   : 2020.05.23-

	[ wavFile ]
*/

#ifndef _WAVFILE_H_
#define _WAVFILE_H_

#include "vm.h"
#include "../emu.h"
#include "datarec.h"

#define TAPBUFFERSIZE (1024 * 10)

class WAVFILE
{
private:

	FILEIO *play_fio;
	int size;
	uint8_t *tapeBuffer;

	int bufferPosition;
	bool prevSignal;
	int16_t maxSample;
	int16_t minSample;
	int tapePosition;

	int16_t sampleSignal;
    int16_t preCheckSampleSignal;
    int16_t nextCheckSampleSignal;

	int readDataCount;

public:
	WAVFILE() {
		tapeBuffer = NULL;
	}
	~WAVFILE() {}
	
	void initialize(FILEIO *parent_play_fio);
	uint8_t nextBuffer();
	int getTapePercent();
	int getTapeSize();
	int getTapePosition();
};
#endif