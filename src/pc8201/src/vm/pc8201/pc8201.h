/*
	NEC PC-8201 Emulator 'ePC-8201'

	Author : Takeda.Toshiya
	Date   : 2009.03.31-

	M5Stack version.
	modified by shikarunochi 2019.04.15 -
	
	[ virtual machine ]
*/

#ifndef _PC8201_H_
#define _PC8201_H_

#ifdef _PC8201A
#define DEVICE_NAME		"NEC PC-8201A"
#define CONFIG_NAME		"pc8201a"
#else
#define DEVICE_NAME		"NEC PC-8201"
#define CONFIG_NAME		"pc8201"
#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		64
#define CPU_CLOCKS		2457600
#define SCREEN_WIDTH		240
#define SCREEN_HEIGHT		64
#define HAS_I8085

// device informations for win32
#define USE_TAPE		1
#define USE_KEY_LOCKED
//#define USE_AUTO_KEY		5
//#define USE_AUTO_KEY_RELEASE	6
//#define USE_AUTO_KEY_CAPS
//#define USE_SOUND_VOLUME	3
//#define USE_DEBUGGER
//#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep"), _T("CMT (Signal)"), _T("Noise (CMT)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class I8080;
class I8155;
class IO;
class PCM1BIT;
class UPD1990A;

class CMT;
class KEYBOARD;
class LCD;
class MEMORY;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	I8080* cpu;
	I8155* pio;
	IO* io;
	PCM1BIT* pcm;
	UPD1990A* rtc;
	
	CMT* cmt;
	KEYBOARD* keyboard;
	LCD* lcd;
	MEMORY* memory;
	
public:
	// ----------------------------------------
	// initialize
	// ----------------------------------------
	
	VM(EMU* parent_emu);
	~VM();
	
	// ----------------------------------------
	// for emulation class
	// ----------------------------------------
	
	// drive virtual machine
	void reset();
	void run();
	double get_frame_rate()
	{
		return FRAMES_PER_SEC;
	}
	
#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index);
#endif
	
	// draw screen
	void draw_screen();
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16_t* create_sound(int* extra_frames);
	int get_sound_buffer_ptr();
#ifdef USE_SOUND_VOLUME
	void set_sound_device_volume(int ch, int decibel_l, int decibel_r);
#endif
	
	// notify key
	void key_down(int code, bool repeat);
	void key_up(int code);
	bool get_caps_locked();
	bool get_kana_locked();
	
	// user interface
	void play_tape(int drv, const _TCHAR* file_path);
	void rec_tape(int drv, const _TCHAR* file_path);
	void close_tape(int drv);
	bool is_tape_inserted(int drv);
	bool is_tape_playing(int drv);
	bool is_tape_recording(int drv);
	int get_tape_position(int drv);
	const _TCHAR* get_tape_message(int drv);
	void push_play(int drv);
	void push_stop(int drv);
	void push_fast_forward(int drv);
	void push_fast_rewind(int drv);
	void push_apss_forward(int drv) {}
	void push_apss_rewind(int drv) {}
	bool is_frame_skippable();
	
	void update_config();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// ----------------------------------------
	// for each device
	// ----------------------------------------
	
	// devices
	DEVICE* get_device(int id);
//	DEVICE* dummy;
//	DEVICE* first_device;
//	DEVICE* last_device;
};

#endif