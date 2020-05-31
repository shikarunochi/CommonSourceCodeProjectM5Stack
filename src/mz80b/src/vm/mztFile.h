/*
	Author : shikarunochi
	Date   : 2019.05.04-

	[ mztFile ]
*/

#ifndef _MZTFILE_H_
#define _MZTFILE_H_

#include "vm.h"
#include "../emu.h"
#include "datarec.h"

#define MZT_TAPEQUEUE_SIZE 1024 * 10
#define MZT_TAPBUFFERSIZE 512

#define MZ80B_LONG_PULSE 16
#define MZ80B_SHORT_PULSE 8

typedef struct {
	bool isHigh;
	uint8_t count;//0~255
} tapeBuffer_t;

typedef struct{
	uint8_t type;
	int count;
} mztFormatData_t;

enum{
	MZT_SIGNAL_0 = 0,
	MZT_SIGNAL_1,
	MZT_BIT_HIGH,
	MZT_BIT_LOW,
	MZT_HEADER,
	MZT_BODY,
};

class MZTFILE
{
private:
	uint16_t size;
	uint16_t offs;
	uint16_t execAddr;
	uint8_t header[128];
	uint8_t tapeBuffer[MZT_TAPBUFFERSIZE];

	FILEIO *play_fio;

	tapeBuffer_t bufferQueue[MZT_TAPEQUEUE_SIZE];
	int inQueuePosition;
	int outQueuePosition;
	int bufferCounter;
	int signalCounter;
    int firstCount;
    int secondCount;
	int tapePosition;
	int tapeLoadPhase;
	int highCount;

	int lastHeaderPosition;

	bool addCheckSumFlg;

	int adjustCount;
	int beforeTapeLoadPhase;

	int addByte(uint8_t byteData);
public:
	MZTFILE() {
	}
	~MZTFILE() {}
	
	void initialize(FILEIO *parent_play_fio);
	uint8_t nextBuffer();
	int getTapePosition();
	int getTapeSize();
	int getTapePercent();
	void addHigh();
	void addLow();
};
#endif