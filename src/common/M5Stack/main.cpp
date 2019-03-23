/*
	Skelton for retropc emulator

	Win32 Version
	Author : Takeda.Toshiya
	Date   : 2006.08.18 -
	[ win32 main ]

	modified by @shikarunochi 2019.03
*/

#include "windows.h"
#include "../emu.h"
#include "../fifo.h"
#include "../fileio.h"

// emulation core
EMU* emu;

// ----------------------------------------------------------------------------
// Emulator main
// ----------------------------------------------------------------------------

//int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szCmdLine, int iCmdShow)
int emuMain()
{
	config.cpu_power = 1;
	config.full_speed = true;
	
	Serial.println("emuMainStart!");
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setCursor(0, 0);
	M5.Lcd.println(String(DEVICE_NAME) + " Emulator for M5Stack FIRE");
    M5.Lcd.println("Version." + String(__DATE__) + "." + String(__TIME__));
    delay(2000);
	M5.Lcd.fillScreen(TFT_BLACK);
	
	load_config("");
	
	// initialize emulation core
    emu=new EMU();
    
	// main loop
	int total_frames = 0, draw_frames = 0, skip_frames = 0;
	DWORD next_time = 0;
	bool prev_skip = false;
	DWORD update_fps_time = 0;
	DWORD update_status_bar_time = 0;
	DWORD disable_screen_saver_time = 0;
	//MSG msg;

	while(1) {
		if(emu){
			// drive machine
			int run_frames = emu->run();
			total_frames += run_frames;
			
			// timing controls
			int sleep_period = 0;
			bool now_skip = (config.full_speed || emu->is_frame_skippable()) && !emu->is_video_recording() && !emu->is_sound_recording();
			
			if((prev_skip && !now_skip) || next_time == 0) {
				next_time = millis();
			}
			if(!now_skip) {
				static int accum = 0;
				accum += emu->get_frame_interval();
				int interval = accum >> 10;
				accum -= interval << 10;
				next_time += interval;
			}
			prev_skip = now_skip;
			
			if(next_time > millis()) {
				// update window if enough time
				draw_frames += emu->draw_screen();
				skip_frames = 0;
				
				// sleep 1 frame priod if need
				DWORD current_time = millis();
				if((int)(next_time - current_time) >= 10) {
					sleep_period = next_time - current_time;
				}
			} else if(++skip_frames > 2 /* (int)emu->get_frame_rate()*/) {
				// update window at least once per 1 sec in virtual machine time
				draw_frames += emu->draw_screen();
				skip_frames = 0;
				next_time = millis();
			}
			if(sleep_period == 0){
				sleep_period = 1;
			}
			delay(sleep_period);
		}
	}
	return 0;
}

