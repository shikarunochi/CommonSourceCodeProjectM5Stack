/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.20-

	M5Stack version.
	modified by shikarunochi 2019.03.21 -

	[ M5Stack dependent ]
*/

#ifndef _M5STACK_OSD_H_
#define _M5STACK_OSD_H_


#include <windows.h>
#include "../vm/vm.h"
//#include "../emu.h"
#include "../common.h"
#include "../emu_config.h"
#if defined(_M5DUMMY)
	#include"M5Dummy.h"
#else
	#include<M5Stack.h>
#endif

#include <SD.h>
#ifndef SDCARD_SS_PIN
#define SDCARD_SS_PIN 4
#endif
#ifndef SDCARD_SPI
#define SDCARD_SPI SPI
#endif
#include <LovyanGFX.hpp>

// ボードの自動検出は使わずに手動設定を行う。
struct LGFX_Config
{
  static constexpr spi_host_device_t spi_host = VSPI_HOST;
  static constexpr int dma_channel = 1;
  static constexpr int spi_sclk = 18;
  static constexpr int spi_mosi = 23;
  static constexpr int spi_miso = 19;  // SDカード等と共通のSPIバスを使う場合はMISOも必ず設定すること。
};
// struct LGFX_EXT_Config
// {
//   static constexpr spi_host_device_t spi_host = HSPI_HOST;
//   static constexpr int dma_channel = 2;
//   static constexpr int spi_sclk = 12;
//   static constexpr int spi_mosi = 13;
//   static constexpr int spi_miso = -1;  // SDカード等と共通のSPIバスを使う場合はMISOも必ず設定すること。
// };
extern lgfx::LGFX_SPI<LGFX_Config> lcd;  // M5Stack本体内蔵LCD用
extern lgfx::LGFX_SPI<LGFX_Config> exLcd; // 外付けST7789用
//TODO:M5DUMMYの場合、ldcを外付け液晶として扱う。

#define EXT_LCD_RST 5
#define EXT_LCD_DC 2

typedef struct bitmap_s {
	// common
	inline bool initialized()
	{
		return (lpBmp != NULL);
	}
	inline scrntype_t* get_buffer(int y)
	{
		return lpBmp + width * y;
	}
	int width, height;
	scrntype_t* lpBmp;

} bitmap_t;

class OSD
{
private:
	int lock_count;

	void initialize_input();
	void release_input();
	void initialize_screen();
	void release_screen();
	
	void initialize_screen_buffer(bitmap_t *buffer, int width, int height, int mode);
	void rotate_screen_buffer(bitmap_t *source, bitmap_t *dest);
	void release_screen_buffer(bitmap_t *buffer);
	void initialize_sound(int rate, int samples);
	
	bitmap_t vm_screen_buffer;
	bitmap_t* draw_screen_buffer;

	void release_sound();
	uint8_t keycode_conv[256];
	uint8_t key_status[256];	// M5Stack key code mapping
	uint8_t key_dik[256];
	uint8_t key_dik_prev[256];
	bool key_shift_pressed, key_shift_released;
	bool key_caps_locked;
	bool lost_focus;

	int vm_screen_width, vm_screen_height;
	int vm_window_width, vm_window_height;
	int vm_window_width_aspect, vm_window_height_aspect;
	int draw_screen_width, draw_screen_height;
	int screenOffsetX, screenOffsetY;

	String screenMessage;
	String preScreenMessage;

	void checkKeyboard();
	int checkSerialKey();
	int checkI2cKeyboard();
	int keyPress(int keyCode);
	uint8_t pressedVMKey;
	uint8_t keyCheckFrameCount;
	bool btnBLongPress;

	int pressedKey;
	
	String selectFile();
	void sortList(String fileList[], int fileListCount);

	void systemMenu();

	uint32_t joy_status[4]; //joystick #1, #2 (b0 = up, b1 = down, b2 = left, b3 = right, b4- = buttons
	bool joy_to_key_status[256];
	
	void checkJoyStick();
	uint8_t joyPadMode;
#define JOYPAD_NONE 0 
//NORMAL PAD
#define JOYPAD_MODE1 1
//ROTATE PAD
#define JOYPAD_MODE2 2

	uint8_t lcdMode;
#define INTERNAL_LCD 0
#define DOUBLE_LCD 1
#define EXTERNAL_LCD 2
#define EXTERNAL_LCD_SPEED 3

	void keyInputConfig();
	uint8_t M5StackKeyMap[255][2];

	uint8_t diskStatus[4];

	uint8_t drawCount;

	bool drawEnable;	
	bool needRedraw;

public:
	OSD()
	{
		lock_count = 0;
	}
	~OSD() {}
	
	// common
	VM_TEMPLATE* vm;
	
	void initialize(int rate, int samples);
	void release();
	void power_off();
	void suspend();
	void restore();
	void lock_vm();
	void unlock_vm();
	bool is_vm_locked()
	{
		return (lock_count != 0);
	}
	void force_unlock_vm();
	void sleep(uint32_t ms);
	
	// common debugger
#ifdef USE_DEBUGGER
	void start_waiting_in_debugger();
	void finish_waiting_in_debugger();
	void process_waiting_in_debugger();
#endif
	
	// common console
	void open_console(const _TCHAR* title);
	void close_console();
	unsigned int get_console_code_page();
	bool is_console_active();
	void set_console_text_attribute(unsigned short attr);
	void write_console(const _TCHAR* buffer, unsigned int length);
	int read_console_input(_TCHAR* buffer, unsigned int length);
	bool is_console_key_pressed(int vk);
	void close_debugger_console();
	
	// common input
	void update_input();
	void key_down(int code, bool extended, bool repeat);
	void key_up(int code, bool extended);
	void key_down_native(int code, bool repeat);
	void key_up_native(int code);
	void key_lost_focus()
	{
		lost_focus = true;
	}
#ifdef USE_MOUSE
	void enable_mouse();
	void disable_mouse();
	void toggle_mouse();
	bool is_mouse_enabled()
	{
		return mouse_enabled;
	}
#endif
	uint8_t* get_key_buffer()
	{
		return key_status;
	}
#ifdef USE_JOYSTICK
	uint32_t* get_joy_buffer()
	{
		return joy_status;
	}
#endif
#ifdef USE_MOUSE
	int32_t* get_mouse_buffer()
	{
		return mouse_status;
	}
#endif
#ifdef USE_AUTO_KEY
	bool now_auto_key;
#endif
	
	// common screen
	double get_window_mode_power(int mode);
	int get_window_mode_width(int mode);
	int get_window_mode_height(int mode);
	void set_host_window_size(int window_width, int window_height, bool window_mode);
	void set_vm_screen_size(int screen_width, int screen_height, int window_width, int window_height, int window_width_aspect, int window_height_aspect);
	void set_vm_screen_lines(int lines);
	int get_vm_window_width()
	{
		return vm_window_width;
	}
	int get_vm_window_height()
	{
		return vm_window_height;
	}
	int get_vm_window_width_aspect()
	{
		return vm_window_width_aspect;
	}
	int get_vm_window_height_aspect()
	{
		return vm_window_height_aspect;
	}
	scrntype_t* get_vm_screen_buffer(int y);
	int draw_screen();
	
	int draw_screen_exec();
	void draw_screen_enable();
	void draw_screen_disable();
	void dmaDraw(bitmap_t *draw_screen_buffer);

	// common sound[Dummy]
	void update_sound(int* extra_frames){};
	void mute_sound(){};
	void stop_sound(){};
	void start_record_sound(){};
	void stop_record_sound(){};
	void restart_record_sound(){};

	void capture_screen();
	bool start_record_video(int fps);
	void stop_record_video();
	void restart_record_video();
	void add_extra_frames(int extra_frames);
#ifdef USE_SCREEN_FILTER
	bool screen_skip_line;
#endif
	bool openFile(String file);

	void set_screen_message(String message);

	void set_disk_status(int drvNo, int status);

	String getFilePathSuffix();

	void keyboard(const uint8_t* d, int len);
	void gui_hid(const uint8_t* hid, int len);  // Parse HID event

//シリアル入力からの入力を内部的に処理するための値。
#define M5KEY_UP 0xB5
#define M5KEY_DOWN 0xB6
#define M5KEY_RIGHT 0xB7
#define M5KEY_LEFT 0xB4
#define M5KEY_HOME 0x99
#define M5KEY_END 0xA4
#define M5KEY_INSERT 0x7F
#define M5KEY_BACKSPACE 0x08
/*
 * Virtual Keys, Standard Set
 */
#define VK_LBUTTON        0x01
#define VK_RBUTTON        0x02
#define VK_CANCEL         0x03
#define VK_MBUTTON        0x04    /* NOT contiguous with L & RBUTTON */

/*
 * 0x07 : reserved
 */


#define VK_BACK           0x08
#define VK_TAB            0x09

/*
 * 0x0A - 0x0B : reserved
 */

#define VK_CLEAR          0x0C
#define VK_RETURN         0x0D

/*
 * 0x0E - 0x0F : unassigned
 */

#define VK_SHIFT          0x10
#define VK_CONTROL        0x11
#define VK_MENU           0x12
#define VK_PAUSE          0x13
#define VK_CAPITAL        0x14

#define VK_KANA           0x15
#define VK_HANGEUL        0x15  /* old name - should be here for compatibility */
#define VK_HANGUL         0x15

/*
 * 0x16 : unassigned
 */

#define VK_JUNJA          0x17
#define VK_FINAL          0x18
#define VK_HANJA          0x19
#define VK_KANJI          0x19

/*
 * 0x1A : unassigned
 */

#define VK_ESCAPE         0x1B

#define VK_CONVERT        0x1C
#define VK_NONCONVERT     0x1D
#define VK_ACCEPT         0x1E
#define VK_MODECHANGE     0x1F

#define VK_SPACE          0x20
#define VK_PRIOR          0x21
#define VK_NEXT           0x22
#define VK_END            0x23
#define VK_HOME           0x24
#define VK_LEFT           0x25
#define VK_UP             0x26
#define VK_RIGHT          0x27
#define VK_DOWN           0x28
#define VK_SELECT         0x29
#define VK_PRINT          0x2A
#define VK_EXECUTE        0x2B
#define VK_SNAPSHOT       0x2C
#define VK_INSERT         0x2D
#define VK_DELETE         0x2E
#define VK_HELP           0x2F

/*
 * VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
 * 0x3A - 0x40 : unassigned
 * VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
 */
#define VK_0           0x30
#define VK_1           0x31
#define VK_2           0x32
#define VK_3           0x33
#define VK_4           0x34
#define VK_5           0x35
#define VK_6           0x36
#define VK_7           0x37
#define VK_8           0x38
#define VK_9           0x39

#define VK_A           0x41
#define VK_B           0x42
#define VK_C           0x43
#define VK_D           0x44
#define VK_E           0x45
#define VK_F           0x46
#define VK_G           0x47
#define VK_H           0x48
#define VK_I           0x49
#define VK_J           0x4A
#define VK_K           0x4B
#define VK_L           0x4C
#define VK_M           0x4D
#define VK_N           0x4E
#define VK_O           0x4F
#define VK_P           0x50
#define VK_Q           0x51
#define VK_R           0x52
#define VK_S           0x53
#define VK_T           0x54
#define VK_U           0x55
#define VK_V           0x56
#define VK_W           0x57
#define VK_X           0x58
#define VK_Y           0x59
#define VK_Z           0x5A

#define VK_LWIN           0x5B
#define VK_RWIN           0x5C
#define VK_APPS           0x5D

/*
 * 0x5E : reserved
 */

#define VK_SLEEP          0x5F

#define VK_NUMPAD0        0x60
#define VK_NUMPAD1        0x61
#define VK_NUMPAD2        0x62
#define VK_NUMPAD3        0x63
#define VK_NUMPAD4        0x64
#define VK_NUMPAD5        0x65
#define VK_NUMPAD6        0x66
#define VK_NUMPAD7        0x67
#define VK_NUMPAD8        0x68
#define VK_NUMPAD9        0x69
#define VK_MULTIPLY       0x6A
#define VK_ADD            0x6B
#define VK_SEPARATOR      0x6C
#define VK_SUBTRACT       0x6D
#define VK_DECIMAL        0x6E
#define VK_DIVIDE         0x6F
#define VK_F1             0x70
#define VK_F2             0x71
#define VK_F3             0x72
#define VK_F4             0x73
#define VK_F5             0x74
#define VK_F6             0x75
#define VK_F7             0x76
#define VK_F8             0x77
#define VK_F9             0x78
#define VK_F10            0x79
#define VK_F11            0x7A
#define VK_F12            0x7B
#define VK_F13            0x7C
#define VK_F14            0x7D
#define VK_F15            0x7E
#define VK_F16            0x7F
#define VK_F17            0x80
#define VK_F18            0x81
#define VK_F19            0x82
#define VK_F20            0x83
#define VK_F21            0x84
#define VK_F22            0x85
#define VK_F23            0x86
#define VK_F24            0x87

#define VK_NUMLOCK        0x90
#define VK_SCROLL         0x91

/*
 * NEC PC-9800 kbd definitions
 */
#define VK_OEM_NEC_EQUAL  0x92   // '=' key on numpad

/*
 * Fujitsu/OASYS kbd definitions
 */
#define VK_OEM_FJ_JISHO   0x92   // 'Dictionary' key
#define VK_OEM_FJ_MASSHOU 0x93   // 'Unregister word' key
#define VK_OEM_FJ_TOUROKU 0x94   // 'Register word' key
#define VK_OEM_FJ_LOYA    0x95   // 'Left OYAYUBI' key
#define VK_OEM_FJ_ROYA    0x96   // 'Right OYAYUBI' key

/*
 * 0x97 - 0x9F : unassigned
 */

/*
 * VK_L* & VK_R* - left and right Alt, Ctrl and Shift virtual keys.
 * Used only as parameters to GetAsyncKeyState() and GetKeyState().
 * No other API or message will distinguish left and right keys in this way.
 */
#define VK_LSHIFT         0xA0
#define VK_RSHIFT         0xA1
#define VK_LCONTROL       0xA2
#define VK_RCONTROL       0xA3
#define VK_LMENU          0xA4
#define VK_RMENU          0xA5

/*
 * 0xB8 - 0xB9 : reserved
 */

#define VK_OEM_1          0xBA   // ';:' for US     :
#define VK_OEM_PLUS       0xBB   // '+' any country ;
#define VK_OEM_COMMA      0xBC   // ',' any country ,
#define VK_OEM_MINUS      0xBD   // '-' any country -
#define VK_OEM_PERIOD     0xBE   // '.' any country .
#define VK_OEM_2          0xBF   // '/?' for US    /
#define VK_OEM_3          0xC0   // '`~' for US    @

/*
 * 0xC1 - 0xC2 : reserved
 */

#define VK_OEM_4          0xDB  //  '[{' for US  [
#define VK_OEM_5          0xDC  //  '\|' for US  \""
#define VK_OEM_6          0xDD  //  ']}' for US  ]
#define VK_OEM_7          0xDE  //  ''"' for US  ^
#define VK_OEM_8          0xDF

/*
 * 0xE0 : reserved
 */

/*
 * Various extended or enhanced keyboards
 */
#define VK_OEM_AX         0xE1  //  'AX' key on Japanese AX kbd
#define VK_OEM_102        0xE2  //  "<>" or "\|" on RT 102-key kbd. _
#define VK_ICO_HELP       0xE3  //  Help key on ICO
#define VK_ICO_00         0xE4  //  00 key on ICO


#define VK_ICO_CLEAR      0xE6

/*
 * 0xE8 : unassigned
 */

/*
 * Nokia/Ericsson definitions
 */
#define VK_OEM_RESET      0xE9
#define VK_OEM_JUMP       0xEA
#define VK_OEM_PA1        0xEB
#define VK_OEM_PA2        0xEC
#define VK_OEM_PA3        0xED
#define VK_OEM_WSCTRL     0xEE
#define VK_OEM_CUSEL      0xEF
#define VK_OEM_ATTN       0xF0
#define VK_OEM_FINISH     0xF1
#define VK_OEM_COPY       0xF2
#define VK_OEM_AUTO       0xF3
#define VK_OEM_ENLW       0xF4
#define VK_OEM_BACKTAB    0xF5

#define VK_ATTN           0xF6
#define VK_CRSEL          0xF7
#define VK_EXSEL          0xF8
#define VK_EREOF          0xF9
#define VK_PLAY           0xFA
#define VK_ZOOM           0xFB
#define VK_NONAME         0xFC
#define VK_PA1            0xFD
#define VK_OEM_CLEAR      0xFE

/*
 * 0xFF : reserved
 */

};

String getEmulatorVersion();

void initLGFXLcd();



//HIDキーコードからエミュレータコードへの変換
//https://wiki.onakasuita.org/pukiwiki/?HID%2F%E3%82%AD%E3%83%BC%E3%82%B3%E3%83%BC%E3%83%89
static uint8_t HidKeyMap[]={
0,// 0	0x00	Reserved (no event indicated)
0,// 1	0x01	Keyboard ErrorRollOver?
0,// 2	0x02	Keyboard POSTFail
0,// 3	0x03	Keyboard ErrorUndefined?
VK_A,// 4	0x04	Keyboard a and A
VK_B,// 5	0x05	Keyboard b and B
VK_C,// 6	0x06	Keyboard c and C
VK_D,// 7	0x07	Keyboard d and D
VK_E,// 8	0x08	Keyboard e and E
VK_F,// 9	0x09	Keyboard f and F
VK_G,// 10	0x0A	Keyboard g and G
VK_H,// 11	0x0B	Keyboard h and H
VK_I,// 12	0x0C	Keyboard i and I
VK_J,// 13	0x0D	Keyboard j and J
VK_K,// 14	0x0E	Keyboard k and K
VK_L,// 15	0x0F	Keyboard l and L
VK_M,// 16	0x10	Keyboard m and M
VK_N,// 17	0x11	Keyboard n and N
VK_O,// 18	0x12	Keyboard o and O
VK_P,// 19	0x13	Keyboard p and P
VK_Q,// 20	0x14	Keyboard q and Q
VK_R,// 21	0x15	Keyboard r and R
VK_S,// 22	0x16	Keyboard s and S
VK_T,// 23	0x17	Keyboard t and T
VK_U,// 24	0x18	Keyboard u and U
VK_V,// 25	0x19	Keyboard v and V
VK_W,// 26	0x1A	Keyboard w and W
VK_X,// 27	0x1B	Keyboard x and X
VK_Y,// 28	0x1C	Keyboard y and Y
VK_Z,// 29	0x1D	Keyboard z and Z
VK_1,// 30	0x1E	Keyboard 1 and !
VK_2,// 31	0x1F	Keyboard 2 and @
VK_3,// 32	0x20	Keyboard 3 and #
VK_4,// 33	0x21	Keyboard 4 and $
VK_5,// 34	0x22	Keyboard 5 and %
VK_6,// 35	0x23	Keyboard 6 and ^
VK_7,// 36	0x24	Keyboard 7 and &
VK_8,// 37	0x25	Keyboard 8 and *
VK_9,// 38	0x26	Keyboard 9 and (
VK_0,// 39	0x27	Keyboard 0 and )
VK_RETURN,// 40	0x28	Keyboard Return (ENTER)
VK_ESCAPE,// 41	0x29	Keyboard ESCAPE
VK_BACK,// 42	0x2A	Keyboard DELETE (Backspace)
VK_TAB,// 43	0x2B	Keyboard Tab
VK_SPACE,// 44	0x2C	Keyboard Spacebar
VK_OEM_MINUS,// 45	0x2D	Keyboard - and (underscore)
VK_OEM_7,// 46	0x2E	Keyboard = and +
VK_OEM_3,// 47	0x2F	Keyboard [ and {
VK_OEM_4,// 48	0x30	Keyboard ] and }
VK_OEM_5,// 49	0x31	Keyboard \ and ｜
VK_OEM_6,// 50	0x32	Keyboard Non-US # and ~
VK_OEM_PLUS,// 51	0x33	Keyboard ; and :
VK_OEM_1,// 52	0x34	Keyboard ' and "
0,// 53	0x35	Keyboard Grave Accent and Tilde
VK_OEM_COMMA,// 54	0x36	Keyboard, and <
VK_OEM_PERIOD,// 55	0x37	Keyboard . and >
VK_OEM_2,// 56	0x38	Keyboard / and ?
VK_CAPITAL,// 57	0x39	Keyboard Caps Lock
VK_F1,// 58	0x3A	Keyboard F1
VK_F2,// 59	0x3B	Keyboard F2
VK_F3,// 60	0x3C	Keyboard F3
VK_F4,// 61	0x3D	Keyboard F4
VK_F5,// 62	0x3E	Keyboard F5
VK_F6,// 63	0x3F	Keyboard F6
VK_F7,// 64	0x40	Keyboard F7
VK_F8,// 65	0x41	Keyboard F8
VK_F9,// 66	0x42	Keyboard F9
VK_F10,// 67	0x43	Keyboard F10
VK_F11,// 68	0x44	Keyboard F11
VK_F12,// 69	0x45	Keyboard F12
VK_SNAPSHOT,// 70	0x46	Keyboard PrintScreen?
0,// 71	0x47	Keyboard Scroll Lock
VK_PAUSE,// 72	0x48	Keyboard Pause
VK_INSERT,// 73	0x49	Keyboard Insert
VK_HOME,// 74	0x4A	Keyboard Home
VK_PRIOR,// 75	0x4B	Keyboard PageUp?
VK_DELETE,// 76	0x4C	Keyboard Delete Forward
VK_END,// 77	0x4D	Keyboard End
VK_NEXT,// 78	0x4E	Keyboard PageDown?
VK_RIGHT,// 79	0x4F	Keyboard RightArrow?
VK_LEFT,// 80	0x50	Keyboard LeftArrow?
VK_DOWN,// 81	0x51	Keyboard DownArrow?
VK_UP,// 82	0x52	Keyboard UpArrow?
0,// 83	0x53	Keypad Num Lock and Clear
0,// 84	0x54	Keypad /
0,// 85	0x55	Keypad *
0,// 86	0x56	Keypad -
0,// 87	0x57	Keypad +
0,// 88	0x58	Keypad ENTER
0,// 89	0x59	Keypad 1 and End
0,// 90	0x5A	Keypad 2 and Down Arrow
0,// 91	0x5B	Keypad 3 and PageDn?
0,// 92	0x5C	Keypad 4 and Left Arrow
0,// 93	0x5D	Keypad 5
0,// 94	0x5E	Keypad 6 and Right Arrow
0,// 95	0x5F	Keypad 7 and Home
0,// 96	0x60	Keypad 8 and Up Arrow
0,// 97	0x61	Keypad 9 and PageUp?
0,// 98	0x62	Keypad 0 and Insert
0,// 99	0x63	Keypad . and Delete
0,// 100	0x64	Keyboard Non-US \ and ｜
0,// 101	0x65	Keyboard Application
0,// 102	0x66	Keyboard Power
0,// 103	0x67	Keypad =
0,// 104	0x68	Keyboard F13
0,// 105	0x69	Keyboard F14
0,// 106	0x6A	Keyboard F15
0,// 107	0x6B	Keyboard F16
0,// 108	0x6C	Keyboard F17
0,// 109	0x6D	Keyboard F18
0,// 110	0x6E	Keyboard F19
0,// 111	0x6F	Keyboard F20
0,// 112	0x70	Keyboard F21
0,// 113	0x71	Keyboard F22
0,// 114	0x72	Keyboard F23
0,// 115	0x73	Keyboard F24
0,// 116	0x74	Keyboard Execute
0,// 117	0x75	Keyboard Help
0,// 118	0x76	Keyboard Menu
0,// 119	0x77	Keyboard Select
0,// 120	0x78	Keyboard Stop
0,// 121	0x79	Keyboard Again
0,// 122	0x7A	Keyboard Undo
0,// 123	0x7B	Keyboard Cut
0,// 124	0x7C	Keyboard Copy
0,// 125	0x7D	Keyboard Paste
0,// 126	0x7E	Keyboard Find
0,// 127	0x7F	Keyboard Mute
0,// 128	0x80	Keyboard Volume Up
0,// 129	0x81	Keyboard Volume Down
0,// 130	0x82	Keyboard Locking Caps Lock
0,// 131	0x83	Keyboard Locking Num Lock
0,// 132	0x84	Keyboard Locking Scroll Lock
0,// 133	0x85	Keypad Comma
0,// 134	0x86	Keypad Equal Sign
0,// 135	0x87	Keyboard International1
0,// 136	0x88	Keyboard International2
0,// 137	0x89	Keyboard International3
0,// 138	0x8A	Keyboard International4
0,// 139	0x8B	Keyboard International5
0,// 140	0x8C	Keyboard International6
0,// 141	0x8D	Keyboard International7
0,// 142	0x8E	Keyboard International8
0,// 143	0x8F	Keyboard International9
0,// 144	0x90	Keyboard LANG1
0,// 145	0x91	Keyboard LANG2
0,// 146	0x92	Keyboard LANG3
0,// 147	0x93	Keyboard LANG4
0,// 148	0x94	Keyboard LANG5
0,// 149	0x95	Keyboard LANG6
0,// 150	0x96	Keyboard LANG7
0,// 151	0x97	Keyboard LANG8
0,// 152	0x98	Keyboard LANG9
0,// 153	0x99	Keyboard Alternate Erase
0,// 154	0x9A	Keyboard SysReq?/Attention
0,// 155	0x9B	Keyboard Cancel
0,// 156	0x9C	Keyboard Clear
0,// 157	0x9D	Keyboard Prior
0,// 158	0x9E	Keyboard Return
0,// 159	0x9F	Keyboard Separator
0,// 160	0xA0	Keyboard Out
0,// 161	0xA1	Keyboard Oper
0,// 162	0xA2	Keyboard Clear/Again
0,// 163	0xA3	Keyboard CrSel?/Props
0// 164	0xA4	Keyboard ExSel?
//
// 224	0xE0	Keyboard LeftControl?
// 225	0xE1	Keyboard LeftShift?
// 226	0xE2	Keyboard LeftAlt?
// 227	0xE3	Keyboard Left GUI
// 228	0xE4	Keyboard RightControl?
// 229	0xE5	Keyboard RightShift?
// 230	0xE6	Keyboard RightAlt?
// 231	0xE7	Keyboard Right GUI
// 232-65535	0xE8-0xFFFF	Reserved
};

#endif
