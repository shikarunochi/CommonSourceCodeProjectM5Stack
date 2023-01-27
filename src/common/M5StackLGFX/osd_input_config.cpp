/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.20-

	M5Stack version.
	modified by shikarunochi 2019.03.21 -

	[ M5Stack keyConfig ]
*/
#pragma GCC optimize("O3")
#include "osd.h"

static uint8_t M5StackKeyMapDefault[][2]={
//cardKBのキーコードからの変換
//M5Stackのキーが入力されたときに
//実際にはどのキーを押すか , SHIFT(1) 必要か、CTRL(2)必要か
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
{'A',1},// 65	0x41	A
{'B',1},// 66	0x42	B
{'C',1},// 67	0x43	C
{'D',1},// 68	0x44	D
{'E',1},// 69	0x45	E
{'F',1},// 70	0x46	F
{'G',1},// 71	0x47	G
{'H',1},// 72	0x48	H
{'I',1},// 73	0x49	I
{'J',1},// 74	0x4a	J
{'K',1},// 75	0x4b	K
{'L',1},// 76	0x4c	L
{'M',1},// 77	0x4d	M
{'N',1},// 78	0x4e	N
{'O',1},// 79	0x4f	O
{'P',1},// 80	0x50	P
{'Q',1},// 81	0x51	Q
{'R',1},// 82	0x52	R
{'S',1},// 83	0x53	S
{'T',1},// 84	0x54	T
{'U',1},// 85	0x55	U
{'V',1},// 86	0x56	V
{'W',1},// 87	0x57	W
{'X',1},// 88	0x58	X
{'Y',1},// 89	0x59	Y
{'Z',1},// 90	0x5a	Z
{0xDB,0},// 91	0x5b	[
{'\\',0},// 92	0x5c	\.
{0xDD,0},// {0,0},// 93	0x5d	]
{0xDE,0},// 94	0x5e	^
{'\\',1},// 95	0x5f	_
{0xC0,1},// 96	0x60	`
{'A',0},// 97	0x61	a
{'B',0},// 98	0x62	b
{'C',0},// 99	0x63	c
{'D',0},// 100	0x64	d
{'E',0},// 101	0x65	e
{'F',0},// 102	0x66	f
{'G',0},// 103	0x67	g
{'H',0},// 104	0x68	h
{'I',0},// 105	0x69	i
{'J',0},// 106	0x6a	j
{'K',0},// 107	0x6b	k
{'L',0},// 108	0x6c	l
{'M',0},// 109	0x6d	m
{'N',0},// 110	0x6e	n
{'O',0},// 111	0x6f	o
{'P',0},// 112	0x70	p
{'Q',0},// 113	0x71	q
{'R',0},// 114	0x72	r
{'S',0},// 115	0x73	s
{'T',0},// 116	0x74	t
{'U',0},// 117	0x75	u
{'V',0},// 118	0x76	v
{'W',0},// 119	0x77	w
{'X',0},// 120	0x78	x
{'Y',0},// 121	0x79	y
{'Z',0},// 122	0x7a	z
{0xDB,1},// 123	0x7b	{
{'\\',1},// 124	0x7c	|
{0xDD,1},// 125	0x7d	}
{0xDE,1},// 126	0x7e	~
{VK_INSERT,0},// 127	0x7f SHIFT+BS
{VK_PAUSE,0},//128 0x80 Fn+ESC
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
{0,0},//139 0x8B Fn+BS
{0x00,0},//140 0x8C Fn+TAB
{0x11,0},//141 0x8D Fn+Q
{0x17,0},//142 0x8E Fn+W
{0x05,0},//143 0x8F Fn+E
{0x12,0},//144 0x90 Fn+R
{0x14,0},//145 0x91 Fn+T
{0x19,0},//146 0x92 Fn+Y
{0x15,0},//147 0x93 Fn+U
{0x09,0},//148 0x94 Fn+I
{0x0F,0},//149 0x95 Fn+O
{0x10,0},//150 0x96 Fn+P
{0x00,0},//151 0x97 
{VK_HOME,0},//152 0x98 Fn+Left
{VK_PRIOR,0},//153 0x99  Fn+Up
{0x01,0},//154 0x9A Fn+A
{0x13,0},//155 0x9B Fn+S
{0x04,0},//156 0x9C Fn+D
{0x06,0},//157 0x9D Fn+F
{0x07,0},//158 0x9E Fn+G
{0x08,0},//159 0x9F Fn+H
{0x0A,0},//160 0xA0 Fn+J
{0x0B,0},//161 0xA1 Fn+K
{0x0C,0},//162 0xA2 Fn+L
{0x00,0},//163 0xA3 Fn+Enter
{VK_NEXT,0},//164 0xA4 F Fn+Down
{VK_END,0},//165 0xA5 Fn+Right
{0x1A,0},//166 0xA6 Fn+Z
{0x18,0},//167 0xA7 Fn+X
{0x03,0},//168 0xA8 Fn+C
{0x16,0},//169 0xA9 Fn+V
{0x02,0},//170 0xAA Fn+B
{0x0E,0},//171 0xAB Fn+N
{0x0D,0},//172 0xAC Fn+M
{0x00,0},//173 0xAD Fn+,
{0x00,0},//174 0xAE Fn+.
{VK_KANA,0},//175 0xAF Fn+SPACE
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


void OSD::keyInputConfig(){
//M5Stack CardKB-> VM key 
//initialize
for(int i = 0;i < 256;i++){
    M5StackKeyMap[i][0] = M5StackKeyMapDefault[i][0];
    M5StackKeyMap[i][1] = M5StackKeyMapDefault[i][1];
}

#if defined(_JR100)
M5StackKeyMap[M5KEY_UP][0]='8';
M5StackKeyMap[M5KEY_UP][1]=2;//+Ctlr
M5StackKeyMap[M5KEY_DOWN][0]='7';
M5StackKeyMap[M5KEY_DOWN][1]=2;//+Ctlr
M5StackKeyMap[M5KEY_LEFT][0]='6';
M5StackKeyMap[M5KEY_LEFT][1]=2;//+Ctlr
M5StackKeyMap[M5KEY_RIGHT][0]='9';
M5StackKeyMap[M5KEY_RIGHT][1]=2;//+Ctlr
M5StackKeyMap[M5KEY_BACKSPACE][0]='5';
M5StackKeyMap[M5KEY_BACKSPACE][1]=2;//+Ctlr
M5StackKeyMap[M5KEY_INSERT][0]='0';
M5StackKeyMap[M5KEY_INSERT][1]=2;//+Ctlr
M5StackKeyMap[M5KEY_HOME][0]='1';
M5StackKeyMap[M5KEY_HOME][1]=2;//+Ctlr
#endif

#if defined(_M5)
M5StackKeyMap[M5KEY_UP][0]='@';
M5StackKeyMap[M5KEY_UP][1]=2;//+Ctlr
M5StackKeyMap[M5KEY_DOWN][0]='/';
M5StackKeyMap[M5KEY_DOWN][1]=2;//+Ctlr
M5StackKeyMap[M5KEY_LEFT][0]=';';
M5StackKeyMap[M5KEY_LEFT][1]=2;//+Ctlr
M5StackKeyMap[M5KEY_RIGHT][0]=':';
M5StackKeyMap[M5KEY_RIGHT][1]=2;//+Ctlr
M5StackKeyMap[M5KEY_BACKSPACE][0]='P';
M5StackKeyMap[M5KEY_BACKSPACE][1]=2;//+Ctlr
M5StackKeyMap[M5KEY_INSERT][0]='O';
M5StackKeyMap[M5KEY_INSERT][1]=2;//+Ctlr
#endif

#if defined(_BMJR)
M5StackKeyMap[M5KEY_UP][0]=0xc0; //@
M5StackKeyMap[M5KEY_UP][1]=1;//+Shift
M5StackKeyMap[M5KEY_DOWN][0]=0xdb; // [
M5StackKeyMap[M5KEY_DOWN][1]=1;//+Shift
M5StackKeyMap[M5KEY_LEFT][0]=0xde;// ^
M5StackKeyMap[M5KEY_LEFT][1]=1;//+Shift
M5StackKeyMap[M5KEY_RIGHT][0]=0xdc;// 
M5StackKeyMap[M5KEY_RIGHT][1]=1;//+Shift
M5StackKeyMap[M5KEY_BACKSPACE][0]=0x2E;
#endif

#if defined(_MZ1500)||defined(_MZ700)
M5StackKeyMap[M5KEY_BACKSPACE][0]=0x2E;
#endif

}