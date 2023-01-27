/*
	Skelton for retropc emulator

	Win32 Version
	Author : Takeda.Toshiya
	Date   : 2006.08.18 -
	[ M5Stack main ]

	modified by @shikarunochi 2019.03
*/
#pragma GCC optimize("O3")
#include "windows.h"
#include "../emu.h"
#include "../fifo.h"
#include "../fileio.h"
#include "osd.h"

//https://gist.github.com/lovyan03/7e0cd8362d7252124ae281526d5eaac1
lgfx::LGFX_SPI<LGFX_Config> lcd;  // M5Stack本体内蔵LCD用
lgfx::LGFX_SPI<LGFX_Config> exLcd; // 外付けST7789用
lgfx::Panel_ILI9342 panel;
lgfx::Panel_ST7789 exPanel;

// emulation core
EMU* emu;

// ----------------------------------------------------------------------------
// Emulator main
// ----------------------------------------------------------------------------

//int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szCmdLine, int iCmdShow)
int emuMain()
{
	M5.Speaker.begin(); 
	M5.Speaker.mute();
	
	config.cpu_power = 1;
	config.full_speed = true;
	
	Serial.println("emuMainStart!");
	//lcd.init();
	//initLGFXLcd();
    lcd.fillScreen(TFT_BLACK);
    lcd.setCursor(0, 0);
	lcd.println(String(DEVICE_NAME) + " Emulator for M5Stack FIRE");
    lcd.println("Version." + getEmulatorVersion());
    delay(2000);
	lcd.fillScreen(TFT_BLACK);
	
	load_config("");
	
#if !defined(_M5Core2)	
	if(digitalRead(BUTTON_C_PIN) == 0) {
    	Serial.println("Alt File Path");
     	lcd.println("Alt File Path");
     	delay(2000);
     	strcpy(config.filePathSuffix,"alt");
        lcd.fillScreen(TFT_BLACK);
		delay(1000);
  	}
#endif
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
			} else if(++skip_frames > 1){//(int)emu->get_frame_rate()) {
				// update window at least once per 1 sec in virtual machine time
				draw_frames += emu->draw_screen();
				skip_frames = 0;
				next_time = millis();
			}
			if(sleep_period == 0){
				//sleep_period = 1;
				delayMicroseconds(1);
			}else{
				delayMicroseconds(1);
				//delay(sleep_period);
			}
		}
	}
	return 0;
}

String getEmulatorVersion(){
	return  String(__DATE__) + "." + String(__TIME__);
}
	
void initLGFXLcd(){
  // panelはM5Stack本体LCDの設定をする
  panel.freq_write = 40000000;
  panel.freq_fill  = 40000000;
  panel.freq_read  = 16000000;
  panel.spi_mode = 0;
  panel.spi_mode_read = 0;
  panel.spi_read = true;
  panel.spi_3wire = true;
  panel.spi_cs = 14;
  panel.spi_dc = 27;
  panel.gpio_rst = 33;
  panel.gpio_bl  = 32;
  panel.pwm_ch_bl = 7;
  panel.backlight_level = true;
  panel.rgb_order = false;

  pinMode(panel.gpio_rst, INPUT); // M5StackのRSTピンを読取り設定にする(TNかIPSか判定に使用するため)

#ifdef USE_240X240LCD
  // exPanelはCS無しST7789 (240x240)の設定をする
  exPanel.freq_write = 80000000; // 80MHz設定(配線の状況次第では40MHzに下げる必要あり)
  //exPanel.freq_write = 40000000; // 80MHz設定(配線の状況次第では40MHzに下げる必要あり)
  exPanel.freq_fill  = 80000000;
  exPanel.freq_read  = 20000000;
  exPanel.spi_mode = 0;        // SPIモードは0を指定する
  exPanel.spi_read = false;    // CSが無い場合はリード系コマンドは使用できない
  exPanel.spi_cs = -1;         // CSなし
  exPanel.spi_dc = EXT_LCD_DC;//13;
  exPanel.gpio_rst = -1;       // RSTは後で手動で制御する
  exPanel.panel_height = 240;  // 240×240のパネルなので縦サイズを240に設定。
  exPanel.reverse_invert = true;

// ※ CS無しのST7789はSPIモード3に設定しておく必要があるが、
//    SPIモード0のパネルと同じバスに接続している場合、
//    パネルを交互に使用するとモード切替でクロックを誤認識してしまう。
//    どうにかしてCS無しのST7789をSPIモード0の設定で動かす必要がある。

  constexpr int PANEL2_RST = EXT_LCD_RST;  // 外部LCDのRSTピン番号
  pinMode(PANEL2_RST, OUTPUT);
  digitalWrite(PANEL2_RST, LOW);  // 外部LCDのRSTをLOWにする（リセット実行）
  pinMode(LGFX_Config::spi_sclk, OUTPUT);
  digitalWrite(LGFX_Config::spi_sclk, HIGH); // SCLKピンをHIGHにしておく
  digitalWrite(PANEL2_RST, HIGH); // 外部LCDのRST解除

//     SCLKを事前にHIGHにしてからinitすることで、lcd2.init 実行時にSCLK立下りが生じる。
//     RST解除直後のCS無しパネルは、これをSPIモード3の最初のSCLK立下りと判断してくれるため、
//     以後の通信はSPIモード0の設定で行っても動作するようになる。

// CSのない外部LCDを先に初期化する。
  exLcd.setPanel(&exPanel);
  exLcd.init();
  // SDを使う場合はlcd2のinit後にSD.beginを行う。
#ifdef ADJUST_MZ1D05_FIGURE
exLcd.setRotation(3);
#endif
#endif

  SD.begin(SDCARD_SS_PIN, SDCARD_SPI, 25000000);
  //panel.reverse_invert = digitalRead(panel.gpio_rst);  // M5StackのLCD判定 (TNかIPSか)

// M5Stack本体内蔵LCDは後で初期化する。
  lcd.setPanel(&panel);

#ifdef USE_240X240LCD
  lcd.initPanel(); // バスは初期化済みなのでパネルのみ初期化する。
#else
  lcd.init(); // バスは初期化済みなのでパネルのみ初期化する。
#endif
  lcd.fillScreen(TFT_BLACK);

  lcd.setTextSize(1);
  lcd.setTextColor(TFT_WHITE);
  lcd.setSwapBytes(true);
#ifdef USE_240X240LCD
  exLcd.setTextSize(1);
  exLcd.setTextColor(TFT_WHITE);
  exLcd.println("");//何か表示すると安定する（？）
#endif
}