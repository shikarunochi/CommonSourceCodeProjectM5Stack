/*
	SHARP MZ-700 Emulator 'EmuZ-700'
	SHARP MZ-800 Emulator 'EmuZ-800'
	SHARP MZ-1500 Emulator 'EmuZ-1500'

	Author : Takeda.Toshiya
	Date   : 2008.06.05 -

	M5Stack version.
	modified by shikarunochi 2019.05.01 -

	[ memory ]
*/

#include "memory.h"
#include "../i8253.h"
#include "../i8255.h"
#if defined(_MZ800)
#include "../z80pio.h"
#endif

#define EVENT_TEMPO		0
#define EVENT_BLINK		1
#define EVENT_HBLANK		2
#define EVENT_HSYNC_S		3
#define EVENT_HSYNC_E		4
#if defined(_MZ1500)
#define EVENT_HBLANK_PCG	5
#endif

#define MEM_BANK_MON_L		0x01
#define MEM_BANK_MON_H		0x02
#if defined(_MZ800)
#define MEM_BANK_CGROM_R	0x04
#define MEM_BANK_CGROM_W	0x08
#define MEM_BANK_CGROM		(MEM_BANK_CGROM_R | MEM_BANK_CGROM_W)
#define MEM_BANK_VRAM		0x10
#endif
#if defined(_MZ800) || defined(_MZ1500)
#define MEM_BANK_PCG		0x20
#endif

#if defined(_MZ800)
#define MZ700_MODE	(dmd & 8)
#endif

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 11, eb = (e) >> 11; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x800 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x800 * (i - sb); \
		} \
	} \
}

#define MEM_BANK_MON	1

#if defined(_MZ800)
#define IPL_FILE_NAME	"MZ700IPL.ROM"
#define EXT_FILE_NAME	"MZ800IPL.ROM"
#else
#define IPL_FILE_NAME	"IPL.ROM"
#define EXT_FILE_NAME	"EXT.ROM"
#endif

void MEMORY::initialize()
{
	wdmy = (uint8_t *)ps_malloc(0x800);
	rdmy = (uint8_t *)ps_malloc(0x800);
	
	ipl = (uint8_t *)ps_malloc(0x1000);	// IPL 4KB
#if defined(_MZ800) || defined(_MZ1500)
	ext = (uint8_t *)ps_malloc(0x1800);	// MZ-1500 EXT 6KB
	pcg = (uint8_t *)ps_malloc(0x6000);	// MZ-1500 PCG 8KB * 3
#endif
	font = (uint8_t *)ps_malloc(0x1000);	// CGROM 4KB
	ram = (uint8_t *)ps_malloc(0x10000);	// Main RAM 64KB
	vram = (uint8_t *)ps_malloc(0x1000);	// MZ-700/1500 VRAM 4KB
	
	screen = (uint8_t *)ps_malloc(200 * 320);


	// init memory
	memset(ipl, 0xff, 0x1000);
	memset(ram, 0, 0x10000);
	memset(vram, 0, 0x1000);
#if defined(_MZ700) || defined(_MZ1500)
	memset(vram + 0x800, 0x71, 0x400);
#endif
#if defined(_MZ800) || defined(_MZ1500)
	memset(ext, 0xff, 0x1800);
#endif
#if defined(_MZ1500)
	memset(pcg, 0, 0x6000);
#endif
	memset(font, 0, 0x1000);
	memset(rdmy, 0xff, 0x800);
	
	// load rom images
	FILEIO* fio = new FILEIO();
#if defined(_MZ1500)
	if(fio->Fopen(create_local_path(_T("MZ-1500.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, 0x1000, 1);
		fio->Fread(ext, 0x1800, 1);
		fio->Fclose();
	}else{
#endif
	if(fio->Fopen(create_local_path(_T(IPL_FILE_NAME)), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, 0x1000, 1);
		fio->Fclose();
	}
#if defined(_MZ800) || defined(_MZ1500)
	if(fio->Fopen(create_local_path(_T(EXT_FILE_NAME)), FILEIO_READ_BINARY)) {
		fio->Fread(ext, 0x1800, 1);
		fio->Fclose();
	}
#endif
#if defined(_MZ1500)
	}
#endif

	if(fio->Fopen(create_local_path(_T("FONT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(font, 0x1000, 1);
		fio->Fclose();
	}
	delete fio;
	
#if defined(_MZ700)
	// init PCG-700
	memset(pcg, 0, sizeof(pcg));
	memcpy(pcg + 0x000, font + 0x000, 0x400);
	memcpy(pcg + 0x800, font + 0x800, 0x400);
#endif
	
	// init memory map
	SET_BANK(0x0000, 0xffff, ram, ram);
	
	// create pc palette
#if defined(_MZ800)
	update_config();
#else
	for(int i = 0; i < 8; i++) {
		palette_pc[i] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);
	}
#endif
	
	// register event
	register_vline_event(this);
	register_event_by_clock(this, EVENT_TEMPO, CPU_CLOCKS / 64, true, NULL);	// 32hz * 2
	register_event_by_clock(this, EVENT_BLINK, CPU_CLOCKS / 3, true, NULL);	// 1.5hz * 2
}


void MEMORY::reset()
{
#if defined(_MZ800)
	// check dip-switch
	is_mz800 = (config.boot_mode == 0);
#endif
	
	// reset memory map
	mem_bank = MEM_BANK_MON_L | MEM_BANK_MON_H;
#if defined(_MZ800)
	// TODO: check initial params
	wf = rf = 0x01;
	dmd = 0x00;
	vram_addr_top = 0x9fff;
#elif defined(_MZ1500)
	pcg_bank = 0;
#endif
	update_map_low();
	update_map_middle();
	update_map_high();
	
	// reset crtc
#if defined(_MZ800)
	sof = 0;
	sw = 125;
	ssa = 0;
	sea = 125;
#endif
	blink = tempo = false;
	vblank = vsync = true;
	hblank = hsync = true;
#if defined(_MZ700) || defined(_MZ1500)
	hblank_vram = true;
#if defined(_MZ1500)
	hblank_pcg = true;
#endif
#endif
	
#if defined(_MZ700)
	// reset PCG-700
	pcg_data = pcg_addr = 0;
	pcg_ctrl = 0xff;
#endif
	
	// reset palette
#if defined(_MZ800)
	palette_sw = 0;
	for(int i = 0; i < 4; i++) {
		palette[i] = i;
	}
	for(int i = 0; i < 16; i++) {
		palette16[i] = i;
	}
#elif defined(_MZ1500)
	for(int i = 0; i < 8; i++) {
		palette[i] = i;
	}
#endif
	
	// motor is always rotating...
	d_pio->write_signal(SIG_I8255_PORT_C, 0xff, 0x10);
}

#if defined(_MZ800)
void MEMORY::update_config()
{
	if(config.monitor_type == 0) {
		// color
		for(int i = 0; i < 8; i++) {
			palette_pc[i] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);
		}
		for(int i = 0; i < 16; i++) {
			int val = (i & 8) ? 255 : 127;
			palette_mz800_pc[i] = RGB_COLOR((i & 2) ? val : 0, (i & 4) ? val : 0, (i & 1) ? val : 0);
		}
	} else {
		// monochrome
		for(int i = 0; i < 8; i++) {
			palette_pc[i] = RGB_COLOR(255 * i / 7, 255 * i / 7, 255 * i / 7);
		}
		for(int i = 0; i < 16; i++) {
			palette_mz800_pc[i] = RGB_COLOR(255 * i / 15, 255 * i / 15, 255 * i / 15);
		}
	}
	palette_mz800_pc[8] = palette_mz800_pc[7];
}
#endif

void MEMORY::event_vline(int v, int clock)
{
	// vblank / vsync
	set_vblank(v >= 200);
#if defined(_MZ800)
	vsync = (v >= 240 && v <= 242);
#else
	vsync = (v >= 221 && v <= 223);
#endif
	
	// hblank / hsync
	set_hblank(false);
#if defined(_MZ800)
	register_event_by_clock(this, EVENT_HBLANK, 128, false, NULL);	// PAL 50Hz
	register_event_by_clock(this, EVENT_HSYNC_S, 161, false, NULL);
	register_event_by_clock(this, EVENT_HSYNC_E, 177, false, NULL);
#else
	register_event_by_clock(this, EVENT_HBLANK, 165, false, NULL);	// NTSC 60Hz
//	register_event_by_clock(this, EVENT_HSYNC_S, 180, false, NULL);
//	register_event_by_clock(this, EVENT_HSYNC_E, 194, false, NULL);
#endif
	
#if defined(_MZ700) || defined(_MZ1500)
	// memory wait for vram
	hblank_vram = false;
#if defined(_MZ1500)
	// memory wait for pcg
	register_event_by_clock(this, EVENT_HBLANK_PCG, 170, false, NULL);
	hblank_pcg = false;
#endif
#endif
	
	// draw one line
	if(v < 200) {
		draw_line(v);
	}
}

void MEMORY::event_callback(int event_id, int err)
{
	if(event_id == EVENT_TEMPO) {
		// 32KHz
		tempo = !tempo;
	} else if(event_id == EVENT_BLINK) {
		// 556 OUT (1.5KHz) -> 8255:PC6
		d_pio->write_signal(SIG_I8255_PORT_C, (blink = !blink) ? 0xff : 0, 0x40);
	} else if(event_id == EVENT_HBLANK) {
		set_hblank(true);
#if defined(_MZ700) || defined(_MZ1500)
		if(hblank_vram) {
			// wait because vram is accessed
			d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 0);
		}
		hblank_vram = true;
#endif
	} else if(event_id == EVENT_HSYNC_S) {
		hsync = true;
	} else if(event_id == EVENT_HSYNC_E) {
		hsync = false;
#if defined(_MZ1500)
	} else if(event_id == EVENT_HBLANK_PCG) {
		if(hblank_pcg) {
			// wait because pcg is accessed
			d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 0);
		}
		hblank_pcg = true;
#endif
	}
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
#if defined(_MZ800)
	// MZ-800
	if(MZ700_MODE) {
		if(0xe000 <= addr && addr <= 0xe00f && (mem_bank & MEM_BANK_MON_H)) {
			// memory mapped i/o
			switch(addr & 0x0f) {
			case 0: case 1: case 2: case 3:
				d_pio->write_io8(addr & 3, data);
				break;
			case 4: case 5: case 6: case 7:
				d_pit->write_io8(addr & 3, data);
				break;
			case 8:
				// 8253 gate0
//				d_pit->write_signal(SIG_I8253_GATE_0, data, 1);
				break;
			}
			return;
		}
	} else {
		if(0x8000 <= addr && addr <= vram_addr_top && (mem_bank & MEM_BANK_VRAM)) {
			addr = vram_addr(addr & 0x3fff);
			int page;
			switch(wf & 0xe0) {
			case 0x00:	// single write
				page = (dmd & 4) ? (wf & 5) : wf;
				for(int i = 0, bit = 1; i < 4; i++, bit <<= 1, addr += 0x2000) {
					if(page & bit) {
						vram[addr] = data;
					}
				}
				break;
			case 0x20:	// exor
				page = (dmd & 4) ? (wf & 5) : wf;
				for(int i = 0, bit = 1; i < 4; i++, bit <<= 1, addr += 0x2000) {
					if(page & bit) {
						vram[addr] ^= data;
					}
				}
				break;
			case 0x40:	// or
				page = (dmd & 4) ? (wf & 5) : wf;
				for(int i = 0, bit = 1; i < 4; i++, bit <<= 1, addr += 0x2000) {
					if(page & bit) {
						vram[addr] |= data;
					}
				}
				break;
			case 0x60:	// reset
				page = (dmd & 4) ? (wf & 5) : wf;
				for(int i = 0, bit = 1; i < 4; i++, bit <<= 1, addr += 0x2000) {
					if(page & bit) {
						vram[addr] &= ~data;
					}
				}
				break;
			case 0x80:	// replace
			case 0xa0:
				page = vram_page_mask(wf);
				for(int i = 0, bit = 1; i < 4; i++, bit <<= 1, addr += 0x2000) {
					if(page & bit) {
						if(wf & bit) {
							vram[addr] = data;
						} else {
							vram[addr] = 0;
						}
					}
				}
				break;
			case 0xc0:	// pset
			case 0xe0:
				page = vram_page_mask(wf);
				for(int i = 0, bit = 1; i < 4; i++, bit <<= 1, addr += 0x2000) {
					if(page & bit) {
						if(wf & bit) {
							vram[addr] |= data;
						} else {
							vram[addr] &= ~data;
						}
					}
				}
				break;
			}
			return;
		}
	}
#else
	// MZ-700/1500
#if defined(_MZ1500)
	if(mem_bank & MEM_BANK_PCG) {
		if(0xd000 <= addr && addr <= 0xefff) {
			// pcg wait
			if(!hblank_pcg) {
				d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
				hblank_pcg = true;
			}
		}
	} else {
#endif
		if(mem_bank & MEM_BANK_MON_H) {
			if(0xd000 <= addr && addr <= 0xdfff) {
				// vram wait
				if(!hblank_vram) {
					d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
					hblank_vram = true;
				}
			} else if(0xe000 <= addr && addr <= 0xe00f) {
				// memory mapped i/o
				switch(addr & 0x0f) {
				case 0: case 1: case 2: case 3:
					d_pio->write_io8(addr & 3, data);
					break;
				case 4: case 5: case 6: case 7:
					d_pit->write_io8(addr & 3, data);
					break;
				case 8:
					// 8253 gate0
					d_pit->write_signal(SIG_I8253_GATE_0, data, 1);
					break;
				}
				return;
#if defined(_MZ700)
			} else if(addr == 0xe010) {
				pcg_data = data;
				return;
			} else if(addr == 0xe011) {
				pcg_addr = data;
				return;
			} else if(addr == 0xe012) {
				if(!(pcg_ctrl & 0x10) && (data & 0x10)) {
					int offset = pcg_addr | ((data & 3) << 8);
					offset |= (data & 4) ? 0xc00 : 0x400;
					pcg[offset] = (data & 0x20) ? font[offset] : pcg_data;
				}
				pcg_ctrl = data;
				return;
#endif
			}
		}
#if defined(_MZ1500)
	}
#endif
#endif
	wbank[addr >> 11][addr & 0x7ff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffff;
#if defined(_MZ800)
	// MZ-800
	if(MZ700_MODE) {
		if(0xe000 <= addr && addr <= 0xe00f && (mem_bank & MEM_BANK_MON_H)) {
			// memory mapped i/o
			switch(addr & 0x0f) {
			case 0: case 1: case 2: case 3:
				return d_pio->read_io8(addr & 3);
			case 4: case 5: case 6: case 7:
				return d_pit->read_io8(addr & 3);
			case 8:
				return (hblank ? 0 : 0x80) | (tempo ? 1 : 0) | 0x7e;
			}
			return 0xff;
		}
	} else {
		if(0x8000 <= addr && addr <= vram_addr_top && (mem_bank & MEM_BANK_VRAM)) {
			addr = vram_addr(addr & 0x3fff);
			if(rf & 0x80) {
				int page = vram_page_mask(rf);
				uint32_t result = 0xff;
				for(int bit2 = 1; bit2 <= 0x80; bit2 <<= 1) {
					uint32_t addr2 = addr;
					for(int i = 0, bit = 1; i < 4; i++, bit <<= 1, addr2 += 0x2000) {
						if((page & bit) && (vram[addr2] & bit2) != ((rf & bit) ? bit2 : 0)) {
							result &= ~bit2;
							break;
						}
					}
				}
				return result;
			} else {
				int page = vram_page_mask(rf) & rf;
				for(int i = 0, bit = 1; i < 4; i++, bit <<= 1, addr += 0x2000) {
					if(page & bit) {
						return vram[addr];
					}
				}
			}
			return 0xff;
		}
	}
#else
	// MZ-700/1500
#if defined(_MZ1500)
	if(mem_bank & MEM_BANK_PCG) {
		if(0xd000 <= addr && addr <= 0xefff) {
			// pcg wait
			if(!hblank_pcg) {
				d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
				hblank_pcg = true;
			}
		}
	} else {
#endif
		if(mem_bank & MEM_BANK_MON_H) {
			if(0xd000 <= addr && addr <= 0xdfff) {
				// vram wait
				if(!hblank_vram) {
					d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
					hblank_vram = true;
				}
			} else if(0xe000 <= addr && addr <= 0xe00f) {
				// memory mapped i/o
				switch(addr & 0x0f) {
				case 0: case 1: case 2: case 3:
					return d_pio->read_io8(addr & 3);
				case 4: case 5: case 6: case 7:
					return d_pit->read_io8(addr & 3);
				case 8:
					return (hblank ? 0 : 0x80) | (tempo ? 1 : 0) | 0x7e;
				}
				return 0xff;
			}
		}
#if defined(_MZ1500)
	}
#endif
#endif
	return rbank[addr >> 11][addr & 0x7ff];
}

void MEMORY::write_data8w(uint32_t addr, uint32_t data, int* wait)
{
	*wait = ((mem_bank & MEM_BANK_MON_L) && addr < 0x1000) ? 1 : 0;
	write_data8(addr, data);
}

uint32_t MEMORY::read_data8w(uint32_t addr, int* wait)
{
	*wait = ((mem_bank & MEM_BANK_MON_L) && addr < 0x1000) ? 1 : 0;
	return read_data8(addr);
}

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
#if defined(_MZ800)
	case 0xcc:
		wf = data;
		break;
	case 0xcd:
		rf = data;
		break;
	case 0xce:
		vram_addr_top = (data & 4) ? 0xbfff : 0x9fff;
		dmd = data;
		update_map_middle();
		break;
	case 0xcf:
		switch(addr & 0x7ff) {
		case 0x1cf:
			sof = (sof & 0xf00) | data;
			break;
		case 0x2cf:
			sof = (sof & 0x0ff) | ((data & 3) << 8);
			break;
		case 0x3cf:
			sw = data & 0x7f;
			break;
		case 0x4cf:
			ssa = data & 0x7f;
			break;
		case 0x5cf:
			sea = data & 0x7f;
			break;
		}
		break;
#endif
	case 0xe0:
		mem_bank &= ~MEM_BANK_MON_L;
#if defined(_MZ800)
		mem_bank &= ~MEM_BANK_CGROM;
#endif
		update_map_low();
		update_map_middle();
		break;
	case 0xe1:
		mem_bank &= ~MEM_BANK_MON_H;
		update_map_high();
		break;
	case 0xe2:
		mem_bank |= MEM_BANK_MON_L;
		update_map_low();
		break;
	case 0xe3:
		mem_bank |= MEM_BANK_MON_H;
		update_map_high();
		break;
	case 0xe4:
		mem_bank |= MEM_BANK_MON_L | MEM_BANK_MON_H;
#if defined(_MZ800)
		mem_bank &= ~MEM_BANK_CGROM_R;
		mem_bank |= MEM_BANK_CGROM_W | MEM_BANK_VRAM;
#elif defined(_MZ1500)
		mem_bank &= ~MEM_BANK_PCG;
#endif
		update_map_low();
		update_map_middle();
		update_map_high();
		break;
#if defined(_MZ800) || defined(_MZ1500)
	case 0xe5:
		mem_bank |= MEM_BANK_PCG;
#if defined(_MZ1500)
		pcg_bank = data;
#endif
		update_map_high();
		break;
	case 0xe6:
		mem_bank &= ~MEM_BANK_PCG;
		update_map_high();
		break;
#endif
#if defined(_MZ800)
	case 0xf0:
		if(data & 0x40) {
			palette_sw = (data & 3) << 2;
		} else {
			palette[(data >> 4) & 3] = data & 0x0f;
		}
		for(int i = 0; i < 16; i++) {
			palette16[i] = ((i & 0x0c) == palette_sw) ? palette[i & 3] : i;
		}
		break;
#elif defined(_MZ1500)
	case 0xf0:
		priority = data;
		break;
	case 0xf1:
		palette[(data >> 4) & 7] = data & 7;
		break;
#endif
	}
}

#if defined(_MZ800)
uint32_t MEMORY::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0xce:
		return (hblank ? 0 : 0x80) | (vblank ? 0 : 0x40) | (hsync ? 0 : 0x20) | (vsync ? 0 : 0x10) | (is_mz800 ? 2 : 0) | (tempo ? 1 : 0) | 0x0c;
	case 0xe0:
		mem_bank &= ~MEM_BANK_CGROM_W;
		mem_bank |= MEM_BANK_CGROM_R | MEM_BANK_VRAM;
		update_map_middle();
		break;
	case 0xe1:
		mem_bank &= ~(MEM_BANK_CGROM | MEM_BANK_VRAM);
		update_map_middle();
		break;
	}
	return 0xff;
}
#endif

void MEMORY::set_vblank(bool val)
{
	if(vblank != val) {
		// VBLANK -> 8255:PC7
		d_pio->write_signal(SIG_I8255_PORT_C, val ? 0 : 0xff, 0x80);
#if defined(_MZ800)
		// VBLANK -> Z80PIO:PA5
		d_pio_int->write_signal(SIG_Z80PIO_PORT_A, val ? 0 : 0xff, 0x20);
#endif
		vblank = val;
	}
}

void MEMORY::set_hblank(bool val)
{
	if(hblank != val) {
		hblank = val;
	}
}

void MEMORY::update_map_low()
{
	if(mem_bank & MEM_BANK_MON_L) {
		SET_BANK(0x0000, 0x0fff, wdmy, ipl);
	} else {
		SET_BANK(0x0000, 0x0fff, ram, ram);
	}
}

void MEMORY::update_map_middle()
{
#if defined(_MZ800)
	if(MZ700_MODE) {
		if(mem_bank & MEM_BANK_CGROM_R) {
			SET_BANK(0x1000, 0x1fff, wdmy, font);
			SET_BANK(0xc000, 0xcfff, vram + 0x2000, vram + 0x2000);
		} else {
			SET_BANK(0x1000, 0x1fff, ram + 0x1000, ram + 0x1000);
			SET_BANK(0xc000, 0xcfff, ram + 0xc000, ram + 0xc000);
		}
	} else {
		if(mem_bank & MEM_BANK_CGROM) {
			SET_BANK(0x1000, 0x1fff, wdmy, font);
		} else {
			SET_BANK(0x1000, 0x1fff, ram + 0x1000, ram + 0x1000);
		}
		SET_BANK(0xc000, 0xcfff, ram + 0xc000, ram + 0xc000);
	}
#endif
}

void MEMORY::update_map_high()
{
#if defined(_MZ800)
	// MZ-800
	if(MZ700_MODE) {
		if(mem_bank & MEM_BANK_PCG) {
			SET_BANK(0xd000, 0xffff, wdmy, rdmy);
		} else if(mem_bank & MEM_BANK_MON_H) {
			SET_BANK(0xd000, 0xdfff, vram + 0x3000, vram + 0x3000);
			SET_BANK(0xe000, 0xffff, wdmy, ext);
		} else {
			SET_BANK(0xd000, 0xffff, ram + 0xd000, ram + 0xd000);
		}
	} else {
		SET_BANK(0xd000, 0xdfff, ram + 0xd000, ram + 0xd000);
		if(mem_bank & MEM_BANK_PCG) {
			SET_BANK(0xe000, 0xffff, wdmy, rdmy);
		} else if(mem_bank & MEM_BANK_MON_H) {
			SET_BANK(0xe000, 0xffff, wdmy, ext);
		} else {
			SET_BANK(0xe000, 0xffff, ram + 0xe000, ram + 0xe000);
		}
	}
#else
	// MZ-700/1500
#if defined(_MZ1500)
	if(mem_bank & MEM_BANK_PCG) {
		if(pcg_bank & 3) {
			uint8_t *bank = pcg + ((pcg_bank & 3) - 1) * 0x2000;
			SET_BANK(0xd000, 0xefff, bank, bank);
		} else {
			SET_BANK(0xd000, 0xdfff, wdmy, font);	// read only
			SET_BANK(0xe000, 0xefff, wdmy, font);
		}
		SET_BANK(0xf000, 0xffff, wdmy, rdmy);
	} else {
#endif
		if(mem_bank & MEM_BANK_MON_H) {
			SET_BANK(0xd000, 0xdfff, vram, vram);
#if defined(_MZ1500)
			SET_BANK(0xe000, 0xe7ff, wdmy, rdmy);
			SET_BANK(0xe800, 0xffff, wdmy, ext );
#else
			SET_BANK(0xe000, 0xffff, wdmy, rdmy);
#endif
		} else {
			SET_BANK(0xd000, 0xffff, ram + 0xd000, ram + 0xd000);
		}
#if defined(_MZ1500)
	}
#endif
#endif
}

#if defined(_MZ800)
int MEMORY::vram_page_mask(uint8_t f)
{
	switch(dmd & 7) {
	case 0:	// 320x200,4col
	case 1:
		return (f & 0x10) ? (4 + 8) : (1 + 2);
	case 2:	// 320x200,16col
		return (1 + 2 + 4 + 8);
	case 4:	// 640x200,2col
	case 5:
		return (f & 0x10) ? 4 : 1;
	case 6:	// 640x200,4col
		return (1 + 4);
	}
	return 0;
}

int MEMORY::vram_addr(int addr)
{
	if(dmd & 4) {
		// 640x200
		if(ssa * 128 <= addr && addr < sea * 128) {
			addr += sof * 16;
			if(addr >= sea * 128) {
				addr -= sw * 128;
			}
		}
	} else {
		// 320x200
		if(ssa * 64 <= addr && addr < sea * 64) {
			addr += sof * 8;
			if(addr >= sea * 64) {
				addr -= sw * 64;
			}
		}
	}
	return addr;
}
#endif

#if defined(_MZ700) || defined(_MZ1500)
void MEMORY::draw_line(int v)
{
	int ptr = 40 * (v >> 3);
#if defined(_MZ700)
	bool pcg_active = ((config.dipswitch & 1) && !(pcg_ctrl & 8));
#endif
	for(int x = 0; x < 320; x += 8) {
		uint8_t attr = vram[ptr | 0x800];
#if defined(_MZ1500)
		uint8_t pcg_attr = vram[ptr | 0xc00];
#endif
		uint16_t code = (vram[ptr] << 3) | ((attr & 0x80) << 4);
		uint8_t col_b = attr & 7;
		uint8_t col_f = (attr >> 4) & 7;
#if defined(_MZ700)
		uint8_t pat_t = pcg_active ? pcg[code | (v & 7)] : font[code | (v & 7)];
#else
		uint8_t pat_t = font[code | (v & 7)];
#endif
		uint8_t* dest = screen + v * 320 + x;

#if defined(_MZ1500)
		if((priority & 1) && (pcg_attr & 8)) {
			uint16_t pcg_code = (vram[ptr | 0x400] << 3) | ((pcg_attr & 0xc0) << 5);
			uint8_t pcg_dot[8];
			uint8_t pat_b = pcg[pcg_code | (v & 7) | 0x0000];
			uint8_t pat_r = pcg[pcg_code | (v & 7) | 0x2000];
			uint8_t pat_g = pcg[pcg_code | (v & 7) | 0x4000];
			pcg_dot[0] = ((pat_b & 0x80) >> 7) | ((pat_r & 0x80) >> 6) | ((pat_g & 0x80) >> 5);
			pcg_dot[1] = ((pat_b & 0x40) >> 6) | ((pat_r & 0x40) >> 5) | ((pat_g & 0x40) >> 4);
			pcg_dot[2] = ((pat_b & 0x20) >> 5) | ((pat_r & 0x20) >> 4) | ((pat_g & 0x20) >> 3);
			pcg_dot[3] = ((pat_b & 0x10) >> 4) | ((pat_r & 0x10) >> 3) | ((pat_g & 0x10) >> 2);
			pcg_dot[4] = ((pat_b & 0x08) >> 3) | ((pat_r & 0x08) >> 2) | ((pat_g & 0x08) >> 1);
			pcg_dot[5] = ((pat_b & 0x04) >> 2) | ((pat_r & 0x04) >> 1) | ((pat_g & 0x04) >> 0);
			pcg_dot[6] = ((pat_b & 0x02) >> 1) | ((pat_r & 0x02) >> 0) | ((pat_g & 0x02) << 1);
			pcg_dot[7] = ((pat_b & 0x01) >> 0) | ((pat_r & 0x01) << 1) | ((pat_g & 0x01) << 2);
			
			if(priority & 2) {
				// pcg > text
				dest[0] = pcg_dot[0] ? pcg_dot[0] : (pat_t & 0x80) ? col_f : col_b;
				dest[1] = pcg_dot[1] ? pcg_dot[1] : (pat_t & 0x40) ? col_f : col_b;
				dest[2] = pcg_dot[2] ? pcg_dot[2] : (pat_t & 0x20) ? col_f : col_b;
				dest[3] = pcg_dot[3] ? pcg_dot[3] : (pat_t & 0x10) ? col_f : col_b;
				dest[4] = pcg_dot[4] ? pcg_dot[4] : (pat_t & 0x08) ? col_f : col_b;
				dest[5] = pcg_dot[5] ? pcg_dot[5] : (pat_t & 0x04) ? col_f : col_b;
				dest[6] = pcg_dot[6] ? pcg_dot[6] : (pat_t & 0x02) ? col_f : col_b;
				dest[7] = pcg_dot[7] ? pcg_dot[7] : (pat_t & 0x01) ? col_f : col_b;
			} else {
				// text_fore > pcg > text_back
				dest[0] = (pat_t & 0x80) ? col_f : pcg_dot[0] ? pcg_dot[0] : col_b;
				dest[1] = (pat_t & 0x40) ? col_f : pcg_dot[1] ? pcg_dot[1] : col_b;
				dest[2] = (pat_t & 0x20) ? col_f : pcg_dot[2] ? pcg_dot[2] : col_b;
				dest[3] = (pat_t & 0x10) ? col_f : pcg_dot[3] ? pcg_dot[3] : col_b;
				dest[4] = (pat_t & 0x08) ? col_f : pcg_dot[4] ? pcg_dot[4] : col_b;
				dest[5] = (pat_t & 0x04) ? col_f : pcg_dot[5] ? pcg_dot[5] : col_b;
				dest[6] = (pat_t & 0x02) ? col_f : pcg_dot[6] ? pcg_dot[6] : col_b;
				dest[7] = (pat_t & 0x01) ? col_f : pcg_dot[7] ? pcg_dot[7] : col_b;
			}
		} else {
#endif
			// text only
			dest[0] = (pat_t & 0x80) ? col_f : col_b;
			dest[1] = (pat_t & 0x40) ? col_f : col_b;
			dest[2] = (pat_t & 0x20) ? col_f : col_b;
			dest[3] = (pat_t & 0x10) ? col_f : col_b;
			dest[4] = (pat_t & 0x08) ? col_f : col_b;
			dest[5] = (pat_t & 0x04) ? col_f : col_b;
			dest[6] = (pat_t & 0x02) ? col_f : col_b;
			dest[7] = (pat_t & 0x01) ? col_f : col_b;
#if defined(_MZ1500)
		}
#endif
		ptr++;
	}
}

void MEMORY::draw_screen()
{
	if(emu->now_waiting_in_debugger) {
		// draw lines
		for(int v = 0; v < 200; v++) {
			draw_line(v);
		}
	}
	
	// copy to real screen
	emu->set_vm_screen_lines(200);
	
	for(int y = 0; y < 200; y++) {
		scrntype_t* dest0 = emu->get_screen_buffer(y);
		//scrntype_t* dest1 = emu->get_screen_buffer(2 * y + 1);
		uint8_t* src = screen + y * 320;
		
		for(int x = 0; x < 320; x++) {
#if defined(_MZ1500)
			//dest0[x2] = dest0[x2 + 1] = palette_pc[palette[src[x] & 7]];
			dest0[x] = palette_pc[palette[src[x] & 7]];
#else
			dest0[x] = palette_pc[src[x] & 7];
#endif
		}
		//if(!config.scan_line) {
		//	my_memcpy(dest1, dest0, 640 * sizeof(scrntype_t));
		//} else {
		//	memset(dest1, 0, 640 * sizeof(scrntype_t));
		//}
	}
	emu->screen_skip_line(true);
}
#else
void MEMORY::draw_line(int v)
{
	switch(dmd & 0x0f) {
	case 0x00:	// 320x200,4col
	case 0x01:
		draw_line_320x200_2bpp(v);
		break;
	case 0x02:	// 320x200,16col
		draw_line_320x200_4bpp(v);
		break;
	case 0x04:	// 640x200,2col
	case 0x05:
		draw_line_640x200_1bpp(v);
		break;
	case 0x06:	// 640x200,4col
		draw_line_640x200_2bpp(v);
		break;
	case 0x08:	// MZ-700
		draw_line_mz700(v);
		break;
	}
}

void MEMORY::draw_line_320x200_2bpp(int v)
{
	int ofs1 = (dmd & 1) ? 0x4000 : 0;
	int ofs2 = ofs1 | 0x2000;
	int ptr = 40 * v;
	
	for(int x = 0; x < 320; x += 8) {
		int addr = vram_addr(ptr++);
		uint8_t pat1 = vram[addr | ofs1];
		uint8_t pat2 = vram[addr | ofs2];
		uint8_t* dest = &screen[v][x];
		
		dest[0] = palette[((pat1 & 0x01)     ) | ((pat2 & 0x01) << 1)];
		dest[1] = palette[((pat1 & 0x02) >> 1) | ((pat2 & 0x02)     )];
		dest[2] = palette[((pat1 & 0x04) >> 2) | ((pat2 & 0x04) >> 1)];
		dest[3] = palette[((pat1 & 0x08) >> 3) | ((pat2 & 0x08) >> 2)];
		dest[4] = palette[((pat1 & 0x10) >> 4) | ((pat2 & 0x10) >> 3)];
		dest[5] = palette[((pat1 & 0x20) >> 5) | ((pat2 & 0x20) >> 4)];
		dest[6] = palette[((pat1 & 0x40) >> 6) | ((pat2 & 0x40) >> 5)];
		dest[7] = palette[((pat1 & 0x80) >> 7) | ((pat2 & 0x80) >> 6)];
	}
}

void MEMORY::draw_line_320x200_4bpp(int v)
{
	int ptr = 40 * v;
	
	for(int x = 0; x < 320; x += 8) {
		int addr = vram_addr(ptr++);
		uint8_t pat1 = vram[addr         ];
		uint8_t pat2 = vram[addr | 0x2000];
		uint8_t pat3 = vram[addr | 0x4000];
		uint8_t pat4 = vram[addr | 0x6000];
		uint8_t* dest = &screen[v][x];
		
		dest[0] = palette16[((pat1 & 0x01)     ) | ((pat2 & 0x01) << 1) | ((pat3 & 0x01) << 2) | ((pat4 & 0x01) << 3)];
		dest[1] = palette16[((pat1 & 0x02) >> 1) | ((pat2 & 0x02)     ) | ((pat3 & 0x02) << 1) | ((pat4 & 0x02) << 2)];
		dest[2] = palette16[((pat1 & 0x04) >> 2) | ((pat2 & 0x04) >> 1) | ((pat3 & 0x04)     ) | ((pat4 & 0x04) << 1)];
		dest[3] = palette16[((pat1 & 0x08) >> 3) | ((pat2 & 0x08) >> 2) | ((pat3 & 0x08) >> 1) | ((pat4 & 0x08)     )];
		dest[4] = palette16[((pat1 & 0x10) >> 4) | ((pat2 & 0x10) >> 3) | ((pat3 & 0x10) >> 2) | ((pat4 & 0x10) >> 1)];
		dest[5] = palette16[((pat1 & 0x20) >> 5) | ((pat2 & 0x20) >> 4) | ((pat3 & 0x20) >> 3) | ((pat4 & 0x20) >> 2)];
		dest[6] = palette16[((pat1 & 0x40) >> 6) | ((pat2 & 0x40) >> 5) | ((pat3 & 0x40) >> 4) | ((pat4 & 0x40) >> 3)];
		dest[7] = palette16[((pat1 & 0x80) >> 7) | ((pat2 & 0x80) >> 6) | ((pat3 & 0x80) >> 5) | ((pat4 & 0x80) >> 4)];
	}
}

void MEMORY::draw_line_640x200_1bpp(int v)
{
	int ofs = (dmd & 1) ? 0x4000 : 0;
	int ptr = 80 * v;
	
	for(int x = 0; x < 640; x += 8) {
		int addr = vram_addr(ptr++);
		uint8_t pat = vram[addr | ofs];
		uint8_t* dest = &screen[v][x];
		
		dest[0] = palette[(pat & 0x01)     ];
		dest[1] = palette[(pat & 0x02) >> 1];
		dest[2] = palette[(pat & 0x04) >> 2];
		dest[3] = palette[(pat & 0x08) >> 3];
		dest[4] = palette[(pat & 0x10) >> 4];
		dest[5] = palette[(pat & 0x20) >> 5];
		dest[6] = palette[(pat & 0x40) >> 6];
		dest[7] = palette[(pat & 0x80) >> 7];
	}
}

void MEMORY::draw_line_640x200_2bpp(int v)
{
	int ptr = 80 * v;
	
	for(int x = 0; x < 640; x += 8) {
		int addr = vram_addr(ptr++);
		uint8_t pat1 = vram[addr         ];
		uint8_t pat2 = vram[addr | 0x4000];
		uint8_t* dest = &screen[v][x];
		
		dest[0] = palette[((pat1 & 0x01)     ) | ((pat2 & 0x01) << 1)];
		dest[1] = palette[((pat1 & 0x02) >> 1) | ((pat2 & 0x02)     )];
		dest[2] = palette[((pat1 & 0x04) >> 2) | ((pat2 & 0x04) >> 1)];
		dest[3] = palette[((pat1 & 0x08) >> 3) | ((pat2 & 0x08) >> 2)];
		dest[4] = palette[((pat1 & 0x10) >> 4) | ((pat2 & 0x10) >> 3)];
		dest[5] = palette[((pat1 & 0x20) >> 5) | ((pat2 & 0x20) >> 4)];
		dest[6] = palette[((pat1 & 0x40) >> 6) | ((pat2 & 0x40) >> 5)];
		dest[7] = palette[((pat1 & 0x80) >> 7) | ((pat2 & 0x80) >> 6)];
	}
}

void MEMORY::draw_line_mz700(int v)
{
	int ptr = (40 * (v >> 3)) | 0x3000;
	
	for(int x = 0; x < 320; x += 8) {
		uint8_t attr = vram[ptr | 0x800];
		uint16_t code = (vram[ptr] << 3) | ((attr & 0x80) << 4);
		uint8_t col_b = attr & 7;
		uint8_t col_f = (attr >> 4) & 7;
		uint8_t pat_t = vram[code | (v & 7) | 0x2000];
		uint8_t* dest = &screen[v][x];
		
		// text only
		dest[0] = (pat_t & 0x01) ? col_f : col_b;
		dest[1] = (pat_t & 0x02) ? col_f : col_b;
		dest[2] = (pat_t & 0x04) ? col_f : col_b;
		dest[3] = (pat_t & 0x08) ? col_f : col_b;
		dest[4] = (pat_t & 0x10) ? col_f : col_b;
		dest[5] = (pat_t & 0x20) ? col_f : col_b;
		dest[6] = (pat_t & 0x40) ? col_f : col_b;
		dest[7] = (pat_t & 0x80) ? col_f : col_b;
		ptr++;
	}
}

void MEMORY::draw_screen()
{
	if(emu->now_waiting_in_debugger) {
		// draw lines
		for(int v = 0; v < 200; v++) {
			draw_line(v);
		}
	}
	
	// copy to real screen
	emu->set_vm_screen_lines(200);
	
	for(int y = 0; y < 200; y++) {
		scrntype_t* dest0 = emu->get_screen_buffer(2 * y);
		scrntype_t* dest1 = emu->get_screen_buffer(2 * y + 1);
		uint8_t* src = screen[y];
		
		if(dmd & 8) {
			// MZ-700 mode
			for(int x = 0, x2 = 0; x < 320; x++, x2 += 2) {
				dest0[x2] = dest0[x2 + 1] = palette_pc[src[x] & 7];
			}
		} else if(dmd & 4) {
			// 640x200
			for(int x = 0; x < 640; x++) {
				dest0[x] = palette_mz800_pc[src[x] & 15];
			}
		} else {
			// 320x200
			for(int x = 0, x2 = 0; x < 320; x++, x2 += 2) {
				dest0[x2] = dest0[x2 + 1] = palette_mz800_pc[src[x] & 15];
			}
		}
		if(!config.scan_line) {
			my_memcpy(dest1, dest0, 640 * sizeof(scrntype_t));
		} else {
			memset(dest1, 0, 640 * sizeof(scrntype_t));
		}
	}
	emu->screen_skip_line(true);
}
#endif

#define STATE_VERSION	2

bool MEMORY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
#if defined(_MZ700)
	state_fio->StateArray(pcg + 0x400, 0x400, 1);
	state_fio->StateArray(pcg + 0xc00, 0x400, 1);
#elif defined(_MZ1500)
	state_fio->StateArray(pcg, 0x6000, 1);
#endif
	state_fio->StateArray(ram, 0x10000, 1);
	state_fio->StateArray(vram, 0x1000, 1);
	state_fio->StateValue(mem_bank);
#if defined(_MZ700)
	state_fio->StateValue(pcg_data);
	state_fio->StateValue(pcg_addr);
	state_fio->StateValue(pcg_ctrl);
#elif defined(_MZ800)
	state_fio->StateValue(wf);
	state_fio->StateValue(rf);
	state_fio->StateValue(dmd);
	state_fio->StateValue(vram_addr_top);
	state_fio->StateValue(is_mz800);
#elif defined(_MZ1500)
	state_fio->StateValue(pcg_bank);
#endif
#if defined(_MZ800)
	state_fio->StateValue(sof);
	state_fio->StateValue(sw);
	state_fio->StateValue(ssa);
	state_fio->StateValue(sea);
	state_fio->StateValue(palette_sw);
	state_fio->StateArray(palette, sizeof(palette), 1);
	state_fio->StateArray(palette16, sizeof(palette16), 1);
#elif defined(_MZ1500)
	state_fio->StateValue(priority);
	state_fio->StateArray(palette, sizeof(palette), 1);
#endif
	state_fio->StateValue(blink);
	state_fio->StateValue(tempo);
	state_fio->StateValue(hblank);
	state_fio->StateValue(hsync);
	state_fio->StateValue(vblank);
	state_fio->StateValue(vsync);
#if defined(_MZ700) || defined(_MZ1500)
	state_fio->StateValue(hblank_vram);
#endif
#if defined(_MZ1500)
	state_fio->StateValue(hblank_pcg);
#endif
#if defined(_MZ800)
	state_fio->StateArray(palette_mz800_pc, sizeof(palette_mz800_pc), 1);
#endif
	state_fio->StateArray(palette_pc, sizeof(palette_pc), 1);
	
	// post process
	if(loading) {
		update_map_low();
		update_map_middle();
		update_map_high();
	}
	return true;
}

//From version 2010-09-29
void MEMORY::open_mzt(const _TCHAR* filename)
{
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(filename, FILEIO_READ_BINARY)) {
		// init memory (???)
		memcpy(ram, ipl, (0x1000));
		
		mem_bank &= ~MEM_BANK_MON;
		update_map_low();
		
		// load mzt file
//		fio->Fseek(0, FILEIO_SEEK_END);
//		int remain = fio->Ftell();
//		fio->Fseek(0, FILEIO_SEEK_SET);
//		bool first_block = true;
		
//		while(remain >= 128) {
			uint8_t header[128];
			fio->Fread(header, sizeof(header), 1);
//			remain -= 128;
			
			int size = header[0x12] | (header[0x13] << 8);
			int offs = header[0x14] | (header[0x15] << 8);
			int addr = header[0x16] | (header[0x17] << 8);
			Serial.printf("Read MZT to RAM[size:%X][offs:%X][addr:%X]\n",size,offs,addr);
//			if(first_block) {
//				first_block = false;
				vm->set_pc(addr);
//			}
//			if(remain >= size) {
				fio->Fread(ram + offs, size, 1);
//			}
//			remain -= size;
			
			// patch
			if(header[0x40] == 'P' && header[0x41] == 'A' && header[0x42] == 'T' && header[0x43] == ':') {
				int patch_ofs = 0x44;
				for(; patch_ofs < 0x80; ) {
					uint16_t patch_addr = header[patch_ofs] | (header[patch_ofs + 1] << 8);
					patch_ofs += 2;
					if(patch_addr == 0xffff) {
						break;
					}
					int patch_len = header[patch_ofs++];
					for(int i = 0; i < patch_len; i++) {
						ram[patch_addr + i] = header[patch_ofs++];
					}
				}
			}
//		}
	}
	fio->Fclose();
}

