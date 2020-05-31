/*
	CASIO FP-1100 Emulator 'eFP-1100'

	Author : Takeda.Toshiya
	Date   : 2010.06.18-

	M5Stack version.
	modified by shikarunochi 2020.05.24 -
	
	[ ram pack ]
*/

#include "rampack.h"

void RAMPACK::initialize()
{
	ram = (uint8_t*)ps_malloc(0x4000);
	memset(ram, 0, 0x4000);
	modified = false;
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("RAMPACK%d.BIN"), index), FILEIO_READ_BINARY)) {
		fio->Fread(ram, 0x4000, 1);
		fio->Fclose();
	}
	delete fio;
}

void RAMPACK::release()
{
	if(modified) {
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(create_local_path(_T("RAMPACK%d.BIN"), index), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(ram, 0x4000, 1);
			fio->Fclose();
		}
		delete fio;
	}
}

void RAMPACK::write_io8(uint32_t addr, uint32_t data)
{
	if(addr < 0x4000) {
		if(ram[addr] != data) {
			ram[addr] = data;
			modified = true;
		}
	}
}

uint32_t RAMPACK::read_io8(uint32_t addr)
{
	if(addr < 0x4000) {
		return ram[addr];
	} else if(0xff00 <= addr && addr < 0xff80) {
		return 0x01; // device id
	}
	return 0xff;
}

#define STATE_VERSION	1

bool RAMPACK::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram, 0x4000, 1);
	state_fio->StateValue(modified);
	return true;
}

