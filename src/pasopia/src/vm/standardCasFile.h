/*
	Author : shikarunochi
	Date   : 2020.05.25-

	[ standardCasFile ]
*/

#ifndef _STANDARDCASFILE_H_
#define _STANDARDCASFILE_H_

#include "vm.h"
#include "../emu.h"
#include "datarec.h"

#define STANDARDCAS_BUFFERSIZE (1024 * 10)

class STANDARDCASFILE
{
private:

	FILEIO *play_fio;
	int size;
	uint8_t *tapeBuffer;

	int bufferPosition;
	int tapePosition;
	
	int dataCount;
	int checkCount;
	uint8_t currentData;
public:
	STANDARDCASFILE() {
		tapeBuffer = NULL;
	}
	~STANDARDCASFILE() {}
	
	void initialize(FILEIO *parent_play_fio);
	uint8_t nextBuffer();
	int getTapePercent();
	int getTapeSize();
	int getTapePosition();
};
#endif