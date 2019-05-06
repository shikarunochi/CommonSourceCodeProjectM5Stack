/*
	Author : shikarunochi
	Date   : 2019.05.04-

	[ tapFile ]
*/

#ifndef _TAPFILE_H_
#define _TAPFILE_H_

#include "vm.h"
#include "../emu.h"
#include "datarec.h"

#define TAPBUFFERSIZE (1024 * 10)

class TAPFILE
{
private:

	FILEIO *play_fio;
	int size;
	uint8_t *tapeBuffer;

	int bufferPosition;
	int bitPosition;
	int tapePosition;

public:
	TAPFILE() {
		tapeBuffer = NULL;
	}
	~TAPFILE() {}
	
	void initialize(FILEIO *parent_play_fio);
	uint8_t nextBuffer();
	int getTapePercent();
	int getTapeSize();
	int getTapePosition();
};
#endif