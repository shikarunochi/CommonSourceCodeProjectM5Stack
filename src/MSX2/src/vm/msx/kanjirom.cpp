/*
	Common Source Code Project
	MSX Series (experimental)

	Origin : Zodiac
	Author : umaiboux
	Date   : 2016.03.xx-

	M5Stack version.
	modified by shikarunochi 2019.03.21 -
	
	[ Kanji Rom ]
*/

#include "kanjirom.h"
#include "../../fileio.h"

KANJIROM::KANJIROM(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
{
	rom = (uint8_t*)ps_malloc(256*1024);
	index = 0;
	memset(rom, 0xff, 256*1024);
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("KANJI.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, 256*1024, 1);
		fio->Fclose();
	}
	delete fio;
	
	set_device_name(_T("Kanji ROM"));
}

void KANJIROM::write_io8(uint32_t addr, uint32_t data)
{
	if (0 == (addr & 1)) {
		index = (index & 0x1f800) | ((data & 0x3f) << 5);
	}
	else {
		index = (index & 0x7e0) | ((data & 0x3f) << 11);
	}
}

uint32_t KANJIROM::read_io8(uint32_t addr)
{
	if (1 == (addr & 3)) {
		return rom[index++];
	}
	else if (3 == (addr & 3)) {
		return rom[0x20000 + index++];
	}
	return 0xFF;
}

