/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.26-

	[ win32 input ]
*/

#include "osd.h"
#include "../fifo.h"

//M5Stack CardKB-> VM key 
static const uint8_t M5StackKeyMap[][2]={
//cardKBのキーコードからの変換
//M5Stackのキーが入力されたときに
//実際にはどのキーを押すか , SHIFT 必要か
{0,0},// 0	0x00	
{1,0},// 1	0x01	
{2,0},// 2	0x02	
{3,0},// 3	0x03	
{4,0},// 4	0x04	
{5,0},// 5	0x05	
{6,0},// 6	0x06	
{7,0},// 7	0x07	
//{0x2E,0},// 8	0x08 BS	
{8,0},// 8	0x08 BS	
{9,0},// 9	0x09	
{10,0},// 10	0x0a	
{11,0},// 11	0x0b	
{12,0},// 12	0x0c	
{13,0},// 13	0x0d	
{14,0},// 14	0x0e	
{15,0},// 15	0x0f	
{16,0},// 16	0x10	
{17,0},// 17	0x11	
{18,0},// 18	0x12	
{19,0},// 19	0x13	
{20,0},// 20	0x14	
{21,0},// 21	0x15	
{22,0},// 22	0x16	
{23,0},// 23	0x17	
{24,0},// 24	0x18	
{25,0},// 25	0x19	
{26,0},// 26	0x1a	
{27,0},// 27	0x1b ESC	
{28,0},// 28	0x1c	
{29,0},// 29	0x1d	
{30,0},// 30	0x1e	
{31,0},// 31	0x1f	
{' ',0},// 32	0x20	SPC（空白文字）
{'1',1},// 33	0x21	!
{'2',1},// 34	0x22	"
{'3',1},// 35	0x23	#
{'4',1},// 36	0x24	$
{'5',1},// 37	0x25	%
{'6',1},// 38	0x26	&
{'7',1},// 39	0x27	'
{'8',1},// 40	0x28	(
{'9',1},// 41	0x29	)
{0xBA,1},// 42	0x2a	*
{0xBB,1},// 43	0x2b	+
{0xBC,0},// 44	0x2c	,
{0xBD,0},// 45	0x2d	-
{0xBE,0},// 46	0x2e	.
{0xBF,0},// 47	0x2f	/
{'0',0},// 48	0x30	0
{'1',0},// 49	0x31	1
{'2',0},// 50	0x32	2
{'3',0},// 51	0x33	3
{'4',0},// 52	0x34	4
{'5',0},// 53	0x35	5
{'6',0},// 54	0x36	6
{'7',0},// 55	0x37	7
{'8',0},// 56	0x38	8
{'9',0},// 57	0x39	9
{0xBA,0},// 58	0x3a	:
{0xBB,0},// 59	0x3b	;
{0xBC,1},// 60	0x3c	<
{0xBD,1},// 61	0x3d	=
{0xBE,1},// 62	0x3e	>
{0xBF,1},// 63	0x3f	?
{0xC0,0},// 64	0x40	@
{'A',0},// 65	0x41	A
{'B',0},// 66	0x42	B
{'C',0},// 67	0x43	C
{'D',0},// 68	0x44	D
{'E',0},// 69	0x45	E
{'F',0},// 70	0x46	F
{'G',0},// 71	0x47	G
{'H',0},// 72	0x48	H
{'I',0},// 73	0x49	I
{'J',0},// 74	0x4a	J
{'K',0},// 75	0x4b	K
{'L',0},// 76	0x4c	L
{'M',0},// 77	0x4d	M
{'N',0},// 78	0x4e	N
{'O',0},// 79	0x4f	O
{'P',0},// 80	0x50	P
{'Q',0},// 81	0x51	Q
{'R',0},// 82	0x52	R
{'S',0},// 83	0x53	S
{'T',0},// 84	0x54	T
{'U',0},// 85	0x55	U
{'V',0},// 86	0x56	V
{'W',0},// 87	0x57	W
{'X',0},// 88	0x58	X
{'Y',0},// 89	0x59	Y
{'Z',0},// 90	0x5a	Z
{0xDB,0},// 91	0x5b	[
{'\\',0},// 92	0x5c	\.
{0xDD,0},// {0,0},// 93	0x5d	]
{0xDE,0},// 94	0x5e	^
{'\\',1},// 95	0x5f	_
{0xC0,1},// 96	0x60	`
{'A',1},// 97	0x61	a
{'B',1},// 98	0x62	b
{'C',1},// 99	0x63	c
{'D',1},// 100	0x64	d
{'E',1},// 101	0x65	e
{'F',1},// 102	0x66	f
{'G',1},// 103	0x67	g
{'H',1},// 104	0x68	h
{'I',1},// 105	0x69	i
{'J',1},// 106	0x6a	j
{'K',1},// 107	0x6b	k
{'L',1},// 108	0x6c	l
{'M',1},// 109	0x6d	m
{'N',1},// 110	0x6e	n
{'O',1},// 111	0x6f	o
{'P',1},// 112	0x70	p
{'Q',1},// 113	0x71	q
{'R',1},// 114	0x72	r
{'S',1},// 115	0x73	s
{'T',1},// 116	0x74	t
{'U',1},// 117	0x75	u
{'V',1},// 118	0x76	v
{'W',1},// 119	0x77	w
{'X',1},// 120	0x78	x
{'Y',1},// 121	0x79	y
{'Z',1},// 122	0x7a	z
{0xDB,1},// 123	0x7b	{
{'\\',1},// 124	0x7c	|
{0xDD,1},// 125	0x7d	}
{0xDE,1},// 126	0x7e	~
{VK_INSERT,0},// 127	0x7f SHIFT+BS
{0x00,0},//128 0x80 Fn+ESC
{VK_F1,0},//129 0x81 Fn+1
{VK_F2,0},//130 0x82 Fn+2
{VK_F3,0},//131 0x83 Fn+3
{VK_F4,0},//132 0x84 Fn+4
{VK_F5,0},//133 0x85 Fn+5
{VK_F6,0},//134 0x86 Fn+6
{VK_F7,0},//135 0x87 Fn+7
{VK_F8,0},//136 0x88 Fn+8
{VK_F9,0},//137 0x89 Fn+9
{VK_F10,0},//138 0x8A Fn+0
{VK_KANA,0},//139 0x8B Fn+BS
{0x00,0},//140 0x8C
{0x00,0},//141 0x8D
{0x00,0},//142 0x8E
{0x00,0},//143 0x8F
{0x00,0},//144 0x90
{0x00,0},//145 0x91
{0x00,0},//146 0x92
{0x00,0},//147 0x93
{0x00,0},//148 0x94
{0x00,0},//149 0x95
{0x00,0},//150 0x96
{0x00,0},//151 0x97
{0x00,0},//152 0x98
{VK_HOME,0},//153 0x99 Fn+UP
{0x00,0},//154 0x9A
{0x00,0},//155 0x9B
{0x00,0},//156 0x9C
{0x00,0},//157 0x9D
{0x00,0},//158 0x9E
{0x00,0},//159 0x9F
{0x00,0},//160 0xA0
{0x00,0},//161 0xA1
{0x00,0},//162 0xA2
{0x00,0},//163 0xA3
{VK_END,0},//164 0xA4 Fn+Down
{0x00,0},//165 0xA5
{0x00,0},//166 0xA6
{0x00,0},//167 0xA7
{0x00,0},//168 0xA8
{0x00,0},//169 0xA9
{0x00,0},//170 0xAA
{0x00,0},//171 0xAB
{0x00,0},//172 0xAC
{0x00,0},//173 0xAD
{0x00,0},//174 0xAE
{0x00,0},//175 0xAF
{0x00,0},//176 0xB0
{0x00,0},//177 0xB1
{0x00,0},//178 0xB2
{0x00,0},//179 0xB3
{VK_LEFT,0},//180 0xB4 LEFT
{VK_UP,0},//181 0xB5 UP
{VK_DOWN,0},//182 0xB6 DOWN
{VK_RIGHT,0},//183 0xB7 RIGHT
{0x00,0},//184 0xB8 
{0x00,0},//185 0xB9 
{0x00,0},//186 0xBA
{0x00,0},//187 0xBB
{0x00,0},//188 0xBC
{0x00,0},//189 0xBD
{0x00,0},//190 0xBE
{0x00,0},//191 0xBF
{0x00,0},//192 0xC0
{0x00,0},//193 0xC1
{0x00,0},//194 0xC2
{0x00,0},//195 0xC3
{0x00,0},//196 0xC4
{0x00,0},//197 0xC5
{0x00,0},//198 0xC6
{0x00,0},//199 0xC7
{0x00,0},//200 0xC8
{0x00,0},//201 0xC9
{0x00,0},//202 0xCA
{0x00,0},//203 0xCB
{0x00,0},//204 0xCC
{0x00,0},//205 0xCD
{0x00,0},//206 0xCE
{0x00,0},//207 0xCF
{0x00,0},//208 0xD0
{0x00,0},//209 0xD1
{0x00,0},//210 0xD2
{0x00,0},//211 0xD3
{0x00,0},//212 0xD4
{0x00,0},//213 0xD5
{0x00,0},//214 0xD6
{0x00,0},//215 0xD7
{0x00,0},//216 0xD8
{0x00,0},//217 0xD9
{0x00,0},//218 0xDA
{0x00,0},//219 0xDB
{0x00,0},//220 0xDC
{0x00,0},//221 0xDD
{0x00,0},//222 0xDE
{0x00,0},//223 0xDF
{0x00,0},//224 0xE0
{0x00,0},//225 0xE1
{0x00,0},//226 0xE2
{0x00,0},//227 0xE3
{0x00,0},//228 0xE4
{0x00,0},//229 0xE5
{0x00,0},//230 0xE6
{0x00,0},//231 0xE7
{0x00,0},//232 0xE8
{0x00,0},//233 0xE9
{0x00,0},//234 0xEA
{0x00,0},//235 0xEB
{0x00,0},//236 0xEC
{0x00,0},//237 0xED
{0x00,0},//238 0xEE
{0x00,0},//239 0xEF
{0x00,0},//240 0xF0
{0x00,0},//241 0xF1
{0x00,0},//242 0xF2
{0x00,0},//243 0xF3
{0x00,0},//244 0xF4
{0x00,0},//245 0xF5
{0x00,0},//246 0xF6
{0x00,0},//247 0xF7
{0x00,0},//248 0xF8
{0x00,0},//249 0xF9
{0x00,0},//250 0xFA
{0x00,0},//251 0xFB
{0x00,0},//252 0xFC
{0x00,0},//253 0xFD
{0x00,0},//254 0xFE
{0x00,0},//255 0xFF
};





#define get_joy_range(min_value, max_value, lo_value, hi_value) \
{ \
	uint64_t center = ((uint64_t)min_value + (uint64_t)max_value) / 2; \
	lo_value = (DWORD)((center + (uint64_t)min_value) / 2); \
	hi_value = (DWORD)((center + (uint64_t)max_value) / 2); \
}

void OSD::initialize_input()
{
	// initialize status
	memset(key_status, 0, sizeof(key_status));
	memset(joy_status, 0, sizeof(joy_status));
	memset(joy_to_key_status, 0, sizeof(joy_to_key_status));
	pressedVMKey = 0;
    keyCheckFrameCount = 0;

    pressKey = 0;
    pressedKey = 0;
}

void OSD::release_input()
{
}

void OSD::update_input()
{
	//M5Stackからのキー入力によって状態を変更する
	checkKeyboard();
	M5.update();
    if (M5.BtnB.wasReleased())
    {
		lock_vm();
        String file = selectFile();
        if(file.length() > 0){
            String fileName  = ("/" + String(CONFIG_NAME) + "ROM/" + file);
			const char *cFileName = fileName.c_str();
        	M5.Lcd.fillScreen(TFT_BLACK);
        	M5.Lcd.setCursor(0, 0);
			vm->open_cart(0, cFileName);
			//vm->reset();
        	M5.Lcd.println(file);
			delay(2000);
		}
		M5.Lcd.fillScreen(TFT_BLACK);
		unlock_vm();
    }
    if(pressedKey != 0){
        key_status[pressedKey] = 0x00;
        pressedKey = 0;
    }
}

void OSD::key_down(int code, bool extended, bool repeat)
{
}

void OSD::key_up(int code, bool extended)
{
}


#ifndef VK_LSHIFT
#define VK_LSHIFT 0xA0
#endif
#ifndef VK_SHIFT
#define VK_SHIFT 0x10
#endif
void OSD::checkKeyboard()
{
    if (pressedVMKey != 0)
    {
        keyCheckFrameCount++;
        if (keyCheckFrameCount < 3)
        {
            return;
        }
        keyCheckFrameCount = 0;
        //vm->key_up(VK_SHIFT);
		//vm->key_up(pressedVMKey);
		key_status[VK_SHIFT] &= 0x7f;
		key_status[pressedVMKey] &= 0x7f;
        pressedVMKey = 0;
    }

    int inKeyCode;
    inKeyCode = checkSerialKey();

    if(inKeyCode == 0){
      inKeyCode = checkI2cKeyboard();
    }
    if(inKeyCode == 0){
      return;
    }
    pressedVMKey = keyPress(inKeyCode);
}

//--------------------------------------------------------------
// シリアル入力
//--------------------------------------------------------------
int OSD::checkSerialKey()
{
  int serialKeyCode = 0;
  if (Serial.available() > 0) {
    serialKeyCode = Serial.read();
    if( serialKeyCode == 27 ){ //ESC
      serialKeyCode = Serial.read();
      if(serialKeyCode == 91){
        serialKeyCode = Serial.read();
        switch(serialKeyCode){
          case 65:
            serialKeyCode = M5KEY_UP;  //UP
            break;
          case 66:
            serialKeyCode = M5KEY_DOWN;  //DOWN
            break;
          case 67:
            serialKeyCode = M5KEY_RIGHT;  //RIGHT
            break;
          case 68:
            serialKeyCode = M5KEY_LEFT;  //LEFT
            break;
          case 49:
            serialKeyCode = M5KEY_HOME;  //HOME
            break;
          case 52:
            serialKeyCode = M5KEY_END;  //END
            break;
          case 50:
            serialKeyCode = M5KEY_INSERT;  //INST
            break;
          default:
            serialKeyCode = 0;
        }
      }else if(serialKeyCode == 255)
      {
        serialKeyCode = 0x03;  //ESC 
      }
    }
    if(serialKeyCode == 127){ //BackSpace
      serialKeyCode = 0x08;
    }
    while(Serial.available() > 0 && Serial.read() != -1);
  }
	return serialKeyCode;
}

//--------------------------------------------------------------
// I2C Keyboard Logic
//--------------------------------------------------------------
#define CARDKB_ADDR 0x5F
int OSD::checkI2cKeyboard()
{
    int i2cKeyCode = 0;
    if (Wire.requestFrom(CARDKB_ADDR, 1))
    { // request 1 byte from keyboard
        while (Wire.available())
        {
            i2cKeyCode = Wire.read(); // receive a byte as
            break;
        }
    }
    return i2cKeyCode;
}

//--------------------------------------------------------------
// keyPress
//--------------------------------------------------------------
int OSD::keyPress(int m5StackKeyCode)
{
	//M5Stack側のキーコードを変換してVMに渡す
    pressedVMKey = 0;
    if (m5StackKeyCode != 0)
    {
		pressedVMKey = M5StackKeyMap[m5StackKeyCode][0];
		//shiftFlag
		if(M5StackKeyMap[m5StackKeyCode][1] == 1){
			//vm->key_down(VK_SHIFT, true); 
			key_status[VK_SHIFT] = 0x80;
		}
		//vm->key_down(pressedVMKey, false);
        //Serial.println(pressedVMKey,HEX);
		key_status[pressedVMKey] = 0x80;
	}
    return pressedVMKey;
}