/*
	NEC PC-6001 Emulator 'yaPC-6001'
	NEC PC-6001mkII Emulator 'yaPC-6201'
	NEC PC-6001mkIISR Emulator 'yaPC-6401'
	NEC PC-6601 Emulator 'yaPC-6601'
	NEC PC-6601SR Emulator 'yaPC-6801'

	Author : tanam
	Date   : 2013.07.15-

	M5Stack version.
	modified by shikarunochi 2019.03.24 - 

	[ pseudo sub cpu ]
*/

#include "psub.h"
#include "timer.h"
#include "../i8255.h"
#include "../../M5StackLGFX/osd.h"

#define STICK0_SPACE	0x80
#define STICK0_LEFT		0x20
#define STICK0_RIGHT	0x10
#define STICK0_DOWN		0x08
#define STICK0_UP		0x04
#define STICK0_STOP		0x02
#define STICK0_SHIFT	0x01

#define INTADDR_KEY1	0x02
#define INTADDR_TIMER	0x06
#define INTADDR_CMTREAD	0x08
#define INTADDR_CMTSTOP	0x0E
#define INTADDR_CMTERR	0x12
#define INTADDR_KEY2	0x14
#define INTADDR_STRIG	0x16
#define INTADDR_TVR		0x18	// TV RESERVE-DATA   Read Interrupt
#define INTADDR_DATE	0x1A	// DATE-DATA         Read Interrupt

#define CAS_NONE				0
#define CAS_SAVEBYTE			1
#define CAS_LOADING				2
#define CAS_LOADBYTE			3

#define EVENT_CASSETTE	0
#define EVENT_STRIG	1

/* normal (small alphabet) */
uint8_t Keys1[256][2] =
{
/* 0       1         2        3        4        5        6        7 */
/* 00 */
  /* */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /* BS      TAB                                ENTER                  */
  {0,0x7f},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},

/* 10 */
  /* SHIFT   CTRL      ALT     PAUSE    CAPS     KANA                  */
  {0,0x00},{0,0x00},{0,0x00},{1,0xfa},{1,0xfb},{0,0x00},{0,0x00},{0,0x00},
  /*                           ESC                                     */
  {0,0x00},{0,0x00},{0,0x00},{0,0x1b},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 20 */
  /* SPC    PAGE UP PAGE DOWN    END    HOME      LEFT    UP      RIGHT */
  {0,0x20},{0,0xfe},{0,0xfe},{0,0xfe},{0,0x0c},{0,0x1d},{0,0x1e},{0,0x1c},
  /* DOWN                              PRN SCR   INSERT   DELETE  HELP  */
  {0,0x1f},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x12},{0,0x08},{0,0x00},

/* 30 */
  /* 0        1         2        3        4       5          6       7  */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /* 8        9                                                         */
  {0,0x38},{0,0x39},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 40 */
  /*          A         B        C        D       E         F       G   */
  {0,0x00},{0,0x61},{0,0x62},{0,0x63},{0,0x64},{0,0x65},{0,0x66},{0,0x67},
  /*   H      I         J        K        L       M         N       O   */
  {0,0x68},{0,0x69},{0,0x6a},{0,0x6b},{0,0x6c},{0,0x6d},{0,0x6e},{0,0x6f},

/* 50 */
  /*  P       Q         R        S        T       U         V       W   */
  {0,0x70},{0,0x71},{0,0x72},{0,0x73},{0,0x74},{0,0x75},{0,0x76},{0,0x77},
  /*  X       Y         Z        */
  {0,0x78},{0,0x79},{0,0x7a},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 60   10key*/
  /*  0       1         2        3        4       5         6       7   */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*  8       9         *        +                -         .       /   */
  {0,0x38},{0,0x39},{0,0x2a},{0,0x2b},{0,0x00},{0,0x2d},{0,0x2e},{0,0x2f},

/* 70 */
  /* F1       F2       F3        F4      F5       F6       F7       F8  */
  {1,0xf0},{1,0xf1},{1,0xf2},{1,0xf3},{1,0xf4},{1,0x00},{0,0x00},{1,0xfd},
  /* F9       F10      F11       F12     F13      F14      F15      F16 */
  {0,0x03},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 80 */
  /* F17      F18      F19       F20     F21      F22      F23      F24 */
  {0,0x20},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 90 */
  /* NUM     SCROLL                                                     */
  {0,0x00},{1,0xfe},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* A0 */
  /*                       SCROLL LOCK                                  */
  {0,0x00},{0,0x00},{0,0x00},{1,0xfe},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                    *       +        ,         -        .        /  */
  {0,0x00},{0,0x00},{0,0x2a},{0,0x2b},{0,0x2c},{0,0x2d},{0,0x2e},{0,0x2f},

/* B0 */
  /*                                                                    */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                    :*       ;+       ,<       -        .>      /?  */
  {0,0x00},{0,0x00},{0,0x3A},{0,0x3b},{0,0x2c},{0,0x2d},{0,0x2e},{0,0x2f},

/* C0 */
  /*  @                                                                 */
  {0,0x40},{1,0x00},{1,0x00},{0,0x00},{0,0x00},{0,0x00},{1,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* D0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                            [{        \|        ]}      ^           */
  {0,0x00},{0,0x00},{0,0x00},{0,0x5b},{0,0x5c},{0,0x5d},{0,0x5e},{0,0x00},

/* E0 */
  /*                     _                                              */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{1,0xfe},{1,0xfb},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
/* F0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0xba},{0,0xbb},
  {0,0xbc},{0,0xbd},{0,0xbe},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
};

/* normal (small alphabet) + shift */
uint8_t Keys2[256][2] =
{
/* 00 */
  {0,0x35},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /* BS      TAB                                ENTER                  */
  {0,0x7f},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},

/* 10 */
  /* SHIFT   CTRL      ALT     PAUSE    CAPS     KANA                  */
  {0,0x00},{0,0x00},{0,0x00},{1,0xfa},{1,0xfb},{0,0x00},{0,0x00},{0,0x00},
  /*                           ESC                                     */
  {0,0x00},{0,0x00},{0,0x00},{0,0x1b},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 20 */
  /* SPC    PAGE UP PAGE DOWN    END    HOME      LEFT    UP      RIGHT */
  {0,0x20},{1,0xfc},{1,0xfc},{0,0xfc},{0,0x0b},{0,0x1d},{0,0x1e},{0,0x1c},
  /* DOWN                              PRN SCR   INSERT   DELETE  HELP  */
  {0,0x1f},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x12},{0,0x08},{0,0x00},

/* 30 */
  /* 0        1         2        3        4       5          6       7  */
  {0,0x00},{0,0x21},{0,0x22},{0,0x23},{0,0x24},{0,0x25},{0,0x26},{0,0x27},
  /* 8        9                                                         */
  {0,0x28},{0,0x29},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 40 */
  /*          A         B        C        D       E         F       G   */
  {0,0x40},{0,0x41},{0,0x42},{0,0x43},{0,0x44},{0,0x45},{0,0x46},{0,0x47},
  /*   H      I         J        K        L       M         N       O   */
  {0,0x48},{0,0x49},{0,0x4a},{0,0x4b},{0,0x4c},{0,0x4d},{0,0x4e},{0,0x4f},

/* 50 */
  /*  P       Q         R        S        T       U         V       W   */
  {0,0x50},{0,0x51},{0,0x52},{0,0x53},{0,0x54},{0,0x55},{0,0x56},{0,0x57},
  /*  X       Y         Z        */
  {0,0x58},{0,0x59},{0,0x5a},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 60  10 key */
  /*  0       1         2        3        4       5         6       7   */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*  8       9         *        +                -         .       /   */
  {0,0x38},{0,0x39},{0,0x2a},{0,0x2b},{0,0x00},{0,0x2d},{0,0x2e},{0,0x2f},

/* 70 */
  /* F1       F2       F3        F4      F5       F6       F7       F8  */
  {1,0xf5},{1,0xf6},{1,0xf7},{1,0xf8},{1,0xf9},{1,0x00},{0,0x00},{1,0xfd},
  {0,0x03},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 80 */
  /* F17      F18      F19       F20     F21      F22      F23      F24 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},

/* 90 */
  /* NUM     SCROLL                                                     */
  {0,0x00},{1,0xfe},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* A0 */
  /*                       SCROLL LOCK                                  */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x2a},{0,0x2b},{0,0x2c},{0,0x2d},{0,0x2e},{0,0x2f},

/* B0 */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*                    :*       ;+       ,<       -        .>      /?  */
  {0,0x38},{0,0x39},{0,0x2A},{0,0x2b},{0,0x3c},{0,0x3d},{0,0x3e},{0,0x3f},

/* C0 */
  {0,0x00},{1,0x00},{1,0x00},{0,0x00},{0,0x00},{0,0x00},{1,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* D0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                            [{        \|        ]}      ^           */
  {0,0x00},{0,0x00},{0,0x00},{0,0x7B},{0,0x7C},{0,0x7D},{0,0x00},{0,0x00},

/* E0 */
  /*                     _                                              */
  {0,0x00},{0,0x00},{0,0x5f},{0,0x00},{1,0xfe},{1,0xfb},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
/* F0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0xba},{0,0xbb},
  {0,0xbc},{0,0xbd},{0,0xbe},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
};

/* hiragana */
uint8_t Keys3[256][2] =
{
/* 00 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /* BS      TAB                                ENTER                  */
  {0,0x7f},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},

/* 10 */
  /* SHIFT   CTRL      ALT     PAUSE    CAPS     KANA                  */
  {0,0x00},{0,0x00},{0,0x00},{1,0xfa},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                           ESC                                     */
  {0,0x00},{0,0x00},{0,0x00},{0,0x1b},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 20 */
  /* SPC    PAGE UP PAGE DOWN    END    HOME      LEFT    UP      RIGHT */
  {0,0x20},{0,0xfe},{0,0xfe},{0,0x00},{0,0x0c},{0,0x1d},{0,0x1e},{0,0x1c},
  /* DOWN                              PRN SCR   INSERT   DELETE  HELP  */
  {0,0x1f},{0,0x00},{0,0x00},{0,0x00},{0,0xe8},{0,0x12},{0,0x08},{0,0xf2},

/* 30 */
  /* 0        1         2        3        4       5          6       7  */
  {0,0xfc},{0,0xe7},{0,0xec},{0,0x91},{0,0x93},{0,0x94},{0,0x95},{0,0xf4},
  /* 8        9                                                         */
  {0,0xf5},{0,0xf6},{0,0x99},{0,0xfa},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 40 */
  /*          A         B        C        D       E         F       G   */
  {0,0x00},{0,0xe1},{0,0x9a},{0,0x9f},{0,0x9c},{0,0x92},{0,0xea},{0,0x97},
  /*   H      I         J        K        L       M         N       O   */
  {0,0x98},{0,0xe6},{0,0xef},{0,0xe9},{0,0xf8},{0,0xf3},{0,0xf0},{0,0xf7},

/* 50 */
  /*  P       Q         R        S        T       U         V       W   */
  {0,0x9e},{0,0xe0},{0,0x9d},{0,0xe4},{0,0x96},{0,0xe5},{0,0xeb},{0,0xe3},
  /*  X       Y         Z        */
  {0,0x9b},{0,0xfd},{0,0xe2},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 60  10 key */
  /*  0       1         2        3        4       5         6       7   */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*  8       9         *        +                -         .       /   */
  {0,0x38},{0,0x39},{0,0x2a},{0,0x2b},{0,0x00},{0,0x2d},{0,0x2e},{0,0x2f},

/* 70 */
  /* F1       F2       F3        F4      F5       F6       F7       F8  */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x03},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 80 */
  {0,0x20},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},


/* 90 */
  /* NUM     SCROLL                                                     */
  {0,0x00},{1,0xfe},{0,0x00},{0,0x00},{0,0x00},{0,0x37},{0,0x34},{0,0x38},
  {0,0x36},{0,0x32},{0,0x39},{0,0x33},{0,0x31},{0,0x35},{0,0x30},{0,0x2e},

/* A0 */
  /*                       SCROLL LOCK                                  */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x99},{0,0xfa},{0,0xe8},{0,0xee},{0,0xf9},{0,0xf2},

/* B0 */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*                    :*       ;+       ,<       -        .>      /?  */
  {0,0x38},{0,0x39},{0,0x99},{0,0xfa},{0,0xe8},{0,0xee},{0,0xf9},{0,0xf2},

/* C0 */
  {0,0xde},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{1,0xfd},{0,0x00},
  {0,0x00},{0,0x00},{0,0x12},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* D0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                            [{        \|        ]}      ^           */
  {0,0x00},{0,0x00},{0,0x00},{0,0xdf},{0,0xb0},{0,0xf1},{0,0xed},{0,0x00},

/* E0 */
  /*                    _              scroll?    caps? */
  {0,0x00},{0,0x00},{0,0xfb},{0,0x00},{1,0xfe},{1,0xfb},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
/* F0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x7f},
};

/* hiragana + shift */
uint8_t Keys4[256][2] =
{
/* 00 */
  {0,0x35},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /* BS      TAB                                ENTER                  */
  {0,0x7f},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},

/* 10 */
  /* SHIFT   CTRL      ALT     PAUSE    CAPS     KANA                  */
  {0,0x00},{0,0x00},{0,0x00},{1,0xfa},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                           ESC                                     */
  {0,0x00},{0,0x00},{0,0x00},{0,0x1b},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 20 */
  /* SPC    PAGE UP PAGE DOWN    END    HOME      LEFT    UP      RIGHT */
  {0,0x20},{1,0xfc},{1,0xfc},{0,0x00},{0,0x0b},{0,0x1d},{0,0x1e},{0,0x1c},
  /* DOWN                              PRN SCR   INSERT   DELETE  HELP  */
  {0,0x1f},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0xa1},{0,0xa5},

/* 30 */
  /* 0        1         2        3        4       5          6       7  */
  {0,0x86},{0,0x00},{0,0x00},{0,0x87},{0,0x89},{0,0x8a},{0,0x8b},{0,0x8c},
  /* 8        9                                                         */
  {0,0x8d},{0,0x8e},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 40 */
  /*          A         B        C        D       E         F       G   */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x88},{0,0x00},{0,0x00},
  /*   H      I         J        K        L       M         N       O   */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 50 */
  /*  P       Q         R        S        T       U         V       W   */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*  X       Y         Z        */
  {0,0x00},{0,0x00},{0,0x8f},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 60  10 key*/
  /*  0       1         2        3        4       5         6       7   */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*  8       9         *        +                -         .       /   */
  {0,0x38},{0,0x39},{0,0x2a},{0,0x2b},{0,0x00},{0,0x2d},{0,0x2e},{0,0x2f},

/* 70 */
  /* F1       F2       F3        F4      F5       F6       F7       F8  */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x03},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 80 */
  {0,0x20},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},

/* 90 */
  /* NUM     SCROLL                                                     */
  {0,0x00},{1,0xfe},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* A0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x2a},{0,0x2b},{0,0x2c},{0,0x2d},{0,0x2e},{0,0x2f},

/* B0 */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*                    :*       ;+       ,<       -        .>      /?  */
  {0,0x38},{0,0x39},{0,0x00},{0,0x00},{0,0xa4},{0,0x00},{0,0xa1},{0,0x00},

/* C0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{1,0xfd},{0,0x00},
  {0,0x00},{0,0x00},{0,0x12},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
/* D0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                            [{        \|        ]}      ^           */
  {0,0x00},{0,0x00},{0,0x00},{0,0xa2},{0,0x00},{0,0xa3},{0,0x00},{0,0x00},
/* E0 */
  /*                     _                                              */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{1,0xfe},{1,0xfb},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
/* F0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x7f},
};

/* katakana */
uint8_t Keys5[256][2] =
{
/* 00 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x7f},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},

/* 10 */
  /* SHIFT   CTRL      ALT     PAUSE    CAPS     KANA                  */
  {0,0x00},{0,0x00},{0,0x00},{1,0xfa},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                           ESC                                     */
  {0,0x00},{0,0x00},{0,0x00},{0,0x1b},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 20 */
  /* SPC    PAGE UP PAGE DOWN    END    HOME      LEFT    UP      RIGHT */
  {0,0x20},{0,0xfe},{0,0xfe},{0,0x00},{0,0x0c},{0,0x1d},{0,0x1e},{0,0x1c},
  /* DOWN                              PRN SCR   INSERT   DELETE  HELP  */
  {0,0x1f},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x12},{0,0x08},{0,0x00},

/* 30 */
  /* 0        1         2        3        4       5          6       7  */
  {0,0xdc},{0,0xc7},{0,0xcc},{0,0xb1},{0,0xb3},{0,0xb4},{0,0xb5},{0,0xd4},
  /* 8        9                                                         */
  {0,0xd5},{0,0xd6},{0,0xb9},{0,0xda},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 40 */
  /*          A         B        C        D       E         F       G   */
  {0,0x00},{0,0xc1},{0,0xba},{0,0xbf},{0,0xbc},{0,0xb2},{0,0xca},{0,0xb7},
  /*   H      I         J        K        L       M         N       O   */
  {0,0xb8},{0,0xc6},{0,0xcf},{0,0xc9},{0,0xd8},{0,0xd3},{0,0xd0},{0,0xd7},

/* 50 */
  /*  P       Q         R        S        T       U         V       W   */
  {0,0xbe},{0,0xc0},{0,0xbd},{0,0xc4},{0,0xb6},{0,0xc5},{0,0xcb},{0,0xc3},
  /*  X       Y         Z        */
  {0,0xbb},{0,0xdd},{0,0xc2},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 60  10 key*/
  /*  0       1         2        3        4       5         6       7   */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*  8       9         *        +                -         .       /   */
  {0,0x38},{0,0x39},{0,0x2a},{0,0x2b},{0,0x00},{0,0x2d},{0,0x2e},{0,0x2f},

/* 70 */
  /* F1       F2       F3        F4      F5       F6       F7       F8  */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x03},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 80 */
  {0,0x20},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},

/* 90 */
  /* NUM     SCROLL                                                     */
  {0,0x00},{1,0xfe},{0,0x00},{0,0x00},{0,0x00},{0,0x37},{0,0x34},{0,0x38},
  {0,0x36},{0,0x32},{0,0x39},{0,0x33},{0,0x31},{0,0x35},{0,0x30},{0,0x2e},

/* A0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0xb9},{0,0xda},{0,0xc8},{0,0xce},{0,0xd9},{0,0xd2},

/* B0 */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*                    :*       ;+       ,<       -        .>      /?  */
  {0,0x38},{0,0x39},{0,0xb9},{0,0xda},{0,0xc8},{0,0xce},{0,0xd9},{0,0xd2},

/* C0 */
  {0,0xde},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x12},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* D0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                            [{        \|        ]}      ^           */
  {0,0x00},{0,0x00},{0,0x00},{0,0xdf},{0,0xb0},{0,0xd1},{0,0xcd},{0,0x00},

/* E0 */
  /*                     _                                              */
  {0,0x00},{0,0x00},{0,0xdb},{0,0x00},{1,0xfe},{1,0xfb},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
/* F0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x7f},
};

/* katakana + shift */
uint8_t Keys6[256][2] =
{
/* 00 */
  {0,0x35},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x7f},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},
/* 10 */
  {0,0x00},{0,0x00},{0,0x00},{1,0xfa},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x1b},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
/* 20 */
  /* SPC    PAGE UP PAGE DOWN    END    HOME      LEFT    UP      RIGHT */
  {0,0x20},{1,0xfc},{1,0xfc},{0,0x00},{0,0x0b},{0,0x1d},{0,0x1e},{0,0x1c},
  /* DOWN                              PRN SCR   INSERT   DELETE  HELP  */
  {0,0x1f},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0xa1},{0,0xa5},

/* 30 */
  /* 0        1         2        3        4       5          6       7  */
  {0,0xa6},{0,0x00},{0,0x00},{0,0xa7},{0,0xa9},{0,0xaa},{0,0xab},{0,0xac},
  /* 8        9                                                         */
  {0,0xad},{0,0xae},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 40 */
  /*          A         B        C        D       E         F       G   */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0xa8},{0,0x00},{0,0x00},
  /*   H      I         J        K        L       M         N       O   */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 50 */
  /*  P       Q         R        S        T       U         V       W   */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*  X       Y         Z        */
  {0,0x00},{0,0x00},{0,0xaf},{0,0xa2},{0,0xb0},{0,0xa3},{0,0x00},{0,0x00},

/* 60 */
  /*  0       1         2        3        4       5         6       7   */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*  8       9         *        +                -         .       /   */
  {0,0x38},{0,0x39},{0,0x2a},{0,0x2b},{0,0x00},{0,0x2d},{0,0x2e},{0,0x2f},

/* 70 */
  /* F1       F2       F3        F4      F5       F6       F7       F8  */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x03},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 80 */
  {0,0x20},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},

/* 90 */
  {0,0x00},{1,0xfe},{0,0x00},{0,0x00},{0,0x00},{0,0x37},{0,0x34},{0,0x38},
  {0,0x36},{0,0x32},{0,0x39},{0,0x33},{0,0x31},{0,0x35},{0,0x30},{0,0x2e},

/* A0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x2a},{0,0x2b},{0,0x2c},{0,0x2d},{0,0x2e},{0,0x2f},

/* B0 */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*                    :*       ;+       ,<       -        .>      /?  */
  {0,0x38},{0,0x39},{0,0x00},{0,0x00},{0,0xa4},{0,0x00},{0,0xa1},{1,0x00},

/* C0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{1,0xfd},{0,0x00},
  {0,0x00},{0,0x00},{0,0x12},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* D0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                            [{        \|        ]}      ^           */
  {0,0x00},{0,0x00},{0,0x00},{0,0xa2},{0,0x00},{0,0xa3},{0,0x00},{0,0x00},

/* E0 */
  /*                     _                                              */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{1,0xfe},{1,0xfb},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* F0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x7f},
};

/* with graph key */
uint8_t Keys7[256][2] =
{
/* 00 */
  {0,0x35},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x7f},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 10 */
  {0,0x00},{0,0x00},{0,0x00},{1,0xfa},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 20 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{1,0x1f},{1,0x17},{1,0x1d},{1,0x80},

/* 30 */
  {1,0x0f},{1,0x07},{1,0x01},{1,0x02},{1,0x03},{1,0x04},{1,0x05},{1,0x06},
  {1,0x0d},{1,0x0e},{1,0x81},{1,0x82},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 40 */
  /*          A         B        C        D       E         F       G   */
  {1,0x83},{0,0x00},{1,0x1b},{1,0x1a},{1,0x14},{1,0x18},{1,0x15},{1,0x13},
  /*   H      I         J        K        L       M         N       O   */
  {1,0x0a},{1,0x16},{0,0x00},{0,0x00},{1,0x1e},{1,0x0b},{0,0x00},{0,0x00},

/* 50 */
  {1,0x10},{0,0x00},{1,0x12},{1,0x0c},{1,0x19},{0,0x00},{1,0x11},{0,0x00},
  {1,0x1c},{1,0x08},{0,0x00},{1,0x84},{1,0x09},{1,0x85},{0,0x00},{0,0x00},

/* 60 */
  {0,0x00},{0,0x00},{1,0x1b},{1,0x1a},{1,0x14},{1,0x18},{1,0x15},{1,0x13},
  {1,0x0a},{1,0x16},{0,0x00},{0,0x00},{1,0x1e},{1,0x0b},{0,0x00},{0,0x00},

/* 70 */
  /* F1       F2       F3        F4      F5       F6       F7       F8  */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x03},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 80 */
  {0,0x20},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},

/* 90 */
  /* NUM     SCROLL                                                     */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x37},{0,0x34},{0,0x38},
  {0,0x36},{0,0x32},{0,0x39},{0,0x33},{0,0x31},{0,0x35},{0,0x30},{0,0x2e},

/* A0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x2a},{0,0x2b},{0,0x2c},{1,0x17},{0,0x2e},{0,0x2f},

/* B0 */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*                    :*       ;+       ,<       -        .>      /?  */
  {0,0x38},{0,0x39},{0,0x81},{0,0x82},{1,0x1f},{1,0x17},{1,0x1d},{0,0x80},

/* C0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x12},{0,0x00},{0,0x00},{1,0x17},{0,0x00},{0,0x00},

/* D0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                            [{        \|        ]}      ^           */
  {0,0x00},{0,0x00},{0,0x00},{0,0x84},{1,0x09},{0,0x85},{0,0x00},{0,0x00},

/* E0 */
  /*                     _                                              */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{1,0xfe},{1,0xfb},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* F0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x7f},
};

void PSUB::update_keyboard(int code)
{
//	for (int code=0; code < 256; code++) {
//		if (key_stat[code] & 0x80) {
			if (code == VK_LSHIFT || code == VK_LCONTROL || code == VK_LMENU ||
			    code == VK_RSHIFT || code == VK_RCONTROL || code == VK_RMENU) return;
			if (code == VK_SHIFT || code == VK_CONTROL) return;
//			key_stat[code]=0;
			if (code == 0x75) {kanaMode = -1 * (kanaMode-1);return;} // VK_F6
			if (code == 0x76) {katakana = -1 * (katakana-1);return;} // VK_F7
			if (code == 0x77) {kbFlagGraph = -1 * (kbFlagGraph-1);return;} // VK_F8
//			p6key=code;
			uint8_t *Keys;
			uint8_t ascii=0;
			if (kbFlagGraph) {
				Keys = Keys7[code];
			} else if (kanaMode) {
				if (katakana) {
					if (stick0 & STICK0_SHIFT) Keys = Keys6[code];
					else Keys = Keys5[code];
				} else if (stick0 & STICK0_SHIFT) { 
					Keys = Keys4[code];
				} else {
					Keys = Keys3[code];
				}
			} else if (stick0 & STICK0_SHIFT) { 
				Keys = Keys2[code];
			} else { 
				Keys = Keys1[code];
			}
			ascii = Keys[1];
			/* control key + alphabet key */
			if ((kbFlagCtrl == 1) && (code >= 0x41) && (code <= 0x5a)) ascii = code - 0x41 + 1;
			/* function key */
			if (!kanaMode && (ascii>0xef && ascii<0xfa)) kbFlagFunc=1;
			/* send key code and raise irq */
			if (!ascii) return;
			p6key = ascii;
			d_pio->write_signal(SIG_I8255_PORT_A, ascii, 0xff);
			d_timer->write_signal(SIG_TIMER_IRQ_SUB_CPU, 1, 1);
//		}
//	}
}

bool PSUB::play_tape(const _TCHAR* file_path)
{
	close_tape();
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		int data, remain = 0x10000;
		FileBaud = 1200;
		if(check_file_extension(file_path, _T(".p6t"))) {
			// get info block offset
			fio->Fseek(-4, FILEIO_SEEK_END);
			int length = fio->FgetInt32();
			// check info block
			fio->Fseek(length, FILEIO_SEEK_SET);
			char id_p = fio->Fgetc();
			char id_6 = fio->Fgetc();
			uint8_t ver = fio->FgetUint8();
			if(id_p == 'P' && id_6 == '6' && ver == 2) {
				uint8_t blocks = fio->FgetUint8();
				if(blocks >= 1) {
					fio->FgetUint8();
					fio->FgetUint8();
					fio->FgetUint8();
					uint16_t cmd = fio->FgetUint16();
					fio->Fseek(cmd, FILEIO_SEEK_CUR);
					uint16_t exp = fio->FgetUint16();
					fio->Fseek(exp, FILEIO_SEEK_CUR);
					// check 1st data block
					char id_t = fio->Fgetc();
					char id_i = fio->Fgetc();
					if(id_t == 'T' && id_i == 'I') {
						fio->FgetUint8();
						fio->Fseek(16, FILEIO_SEEK_CUR);
						FileBaud = (int)fio->FgetUint16();
					}
				}
				remain = min(length, 0x10000);
			}
		}
		fio->Fseek(0, FILEIO_SEEK_SET);
		CasLength = 0;
		while((data = fio->Fgetc()) != EOF && remain > 0) {
			CasData[CasLength++] = data;
			remain--;
		}
		Serial.printf("CasLength=%d,remain=%d", CasLength,remain);Serial.println();
		fio->Fclose();
		if(CasMode == CAS_LOADING /*&& CasBaud == FileBaud*/) {
			register_event(this, EVENT_CASSETTE, 1000000.0 / (CasBaud / 12), false, NULL);
		}
		CasIndex=0;
		play = true;
	}
	return play;
}

bool PSUB::rec_tape(const _TCHAR* file_path)
{
	close_tape();
	
	if(fio->Fopen(file_path, FILEIO_READ_WRITE_NEW_BINARY)) {
		my_tcscpy_s(rec_file_path, _MAX_PATH, file_path);
		CasIndex=0;
		rec = true;
		is_wav = check_file_extension(file_path, _T(".wav"));
		is_p6t = check_file_extension(file_path, _T(".p6t"));
	}
	return rec;
}

static const uint8_t pulse_1200hz[40] = {
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static const uint8_t pulse_2400hz[20] = {
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static const uint8_t pulse_2400hz_x2[40] = {
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

void PSUB::close_tape()
{
	if(fio->IsOpened()) {
		if(rec) {
			if(is_wav) {
				wav_header_t wav_header;
				wav_chunk_t wav_chunk;
				int sample_rate = (CasBaud == 600) ? 24000 : 48000;
				
				fio->Fwrite(&wav_header, sizeof(wav_header), 1);
				fio->Fwrite(&wav_chunk, sizeof(wav_chunk), 1);
				
				for(int i = 0; i < 9600; i++) {
					fio->Fwrite((void *)pulse_2400hz, sizeof(pulse_2400hz), 1);
				}
				for(int i = 0; i < 16; i++) {
					fio->Fwrite((void *)pulse_1200hz, sizeof(pulse_1200hz), 1);
					for(int j = 0; j < 8; j++) {
						if(CasData[i] & (1 << j)) {
							fio->Fwrite((void *)pulse_2400hz_x2, sizeof(pulse_2400hz_x2), 1);
						} else {
							fio->Fwrite((void *)pulse_1200hz, sizeof(pulse_1200hz), 1);
						}
					}
					fio->Fwrite((void *)pulse_2400hz_x2, sizeof(pulse_2400hz_x2), 1);
					fio->Fwrite((void *)pulse_2400hz_x2, sizeof(pulse_2400hz_x2), 1);
					fio->Fwrite((void *)pulse_2400hz_x2, sizeof(pulse_2400hz_x2), 1);
				}
	//			for(int i = 0; i < 1280; i++) {
				for(int i = 0; i < 2400; i++) {
					fio->Fwrite((void *)pulse_2400hz, sizeof(pulse_2400hz), 1);
				}
				for(int i = 16; i < CasIndex; i++) {
					fio->Fwrite((void *)pulse_1200hz, sizeof(pulse_1200hz), 1);
					for(int j = 0; j < 8; j++) {
						if(CasData[i] & (1 << j)) {
							fio->Fwrite((void *)pulse_2400hz_x2, sizeof(pulse_2400hz_x2), 1);
						} else {
							fio->Fwrite((void *)pulse_1200hz, sizeof(pulse_1200hz), 1);
						}
					}
					fio->Fwrite((void *)pulse_2400hz_x2, sizeof(pulse_2400hz_x2), 1);
					fio->Fwrite((void *)pulse_2400hz_x2, sizeof(pulse_2400hz_x2), 1);
					fio->Fwrite((void *)pulse_2400hz_x2, sizeof(pulse_2400hz_x2), 1);
				}
#if 1
				for(int i = 0; i < 16; i++) {
					fio->Fwrite((void *)pulse_1200hz, sizeof(pulse_1200hz), 1);
					for(int j = 0; j < 8; j++) {
						fio->Fwrite((void *)pulse_1200hz, sizeof(pulse_1200hz), 1);
					}
					fio->Fwrite((void *)pulse_2400hz_x2, sizeof(pulse_2400hz_x2), 1);
					fio->Fwrite((void *)pulse_2400hz_x2, sizeof(pulse_2400hz_x2), 1);
					fio->Fwrite((void *)pulse_2400hz_x2, sizeof(pulse_2400hz_x2), 1);
				}
#endif
				uint32_t length = fio->Ftell();
				
				memcpy(wav_header.riff_chunk.id, "RIFF", 4);
				wav_header.riff_chunk.size = length - 8;
				memcpy(wav_header.wave, "WAVE", 4);
				memcpy(wav_header.fmt_chunk.id, "fmt ", 4);
				wav_header.fmt_chunk.size = 16;
				wav_header.format_id = 1;
				wav_header.channels = 1;
				wav_header.sample_rate = sample_rate;
				wav_header.data_speed = sample_rate;
				wav_header.block_size = 1;
				wav_header.sample_bits = 8;
				
				memcpy(wav_chunk.id, "data", 4);
				wav_chunk.size = length - sizeof(wav_header) - sizeof(wav_chunk);
				
				fio->Fseek(0, FILEIO_SEEK_SET);
				fio->Fwrite(&wav_header, sizeof(wav_header), 1);
				fio->Fwrite(&wav_chunk, sizeof(wav_chunk), 1);
			} else {
				fio->Fwrite(CasData, CasIndex, 1);
				if(is_p6t) {
					fio->Fputc('P');
					fio->Fputc('6');
					fio->FputUint8(2);
					fio->FputUint8(2);
					fio->FputUint8(0);
					fio->FputUint8(0);
					fio->FputUint8(0);
					fio->FputUint16(0);
					fio->FputUint16(0);
					
					fio->Fputc('T');
					fio->Fputc('I');
					fio->FputUint8(0);
					for(int i = 0; i < 6; i++) {
						fio->FputUint8(CasData[10 + i]);
					}
					for(int i = 6; i < 16; i++) {
						fio->FputUint8(0);
					}
					fio->FputUint16(CasBaud);
					fio->FputUint16(3000);
					fio->FputUint16(4000);
					fio->FputUint32(0);
					fio->FputUint32(16);
					
					fio->Fputc('T');
					fio->Fputc('I');
					fio->FputUint8(0);
					for(int i = 0; i < 16; i++) {
						fio->FputUint8(0);
					}
					fio->FputUint16(CasBaud);
					fio->FputUint16(0);
					fio->FputUint16(1000);
					fio->FputUint32(16);
					fio->FputUint32(CasIndex - 16);
					
					fio->FputUint32(CasIndex);
				}
			}
		}
		fio->Fclose();
	}
	play = rec = false;
}

void PSUB::initialize()
{
	CasData = (uint8_t*)ps_malloc(0x10000);

	fio = new FILEIO();
	play = rec = false;
	
	key_stat = emu->get_key_buffer();
//	memset(key_stat, 0, sizeof(key_stat));
	
	kbFlagCtrl=0;
	kbFlagGraph=0;
	kbFlagFunc=0;
	kanaMode=0;
	katakana=0;
	
	// register event to update the key status
	register_frame_event(this);
}

void PSUB::release()
{
	close_tape();
	delete fio;
}

void PSUB::reset()
{
	close_tape();
	CasIntFlag=0;
	CasIndex=0;
	CasRecv=0xff;
	CasMode=CAS_NONE;
	CasBaud=FileBaud=1200;
	memset(CasData, 0, 0x10000);
	CasLength=0;
	CasSkipFlag=0;
	
	p6key=0;
	stick0=0;
	StrigIntFlag=0;
	StrigEventID=-1;
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
//	DateIntFlag=0;
#endif
}

void PSUB::event_frame()
{
	if (key_stat[VK_CONTROL] & 0x80) kbFlagCtrl=1;
	else kbFlagCtrl=0;
	stick0 = 0;
	if (key_stat[VK_SPACE]) stick0 |= STICK0_SPACE;
	if (key_stat[VK_LEFT]) stick0 |= STICK0_LEFT;
	if (key_stat[VK_RIGHT]) stick0 |= STICK0_RIGHT;
	if (key_stat[VK_DOWN]) stick0 |= STICK0_DOWN;
	if (key_stat[VK_UP]) stick0 |= STICK0_UP;
	if (key_stat[VK_F9]) stick0 |= STICK0_STOP;
	if (key_stat[VK_SHIFT]) stick0 |= STICK0_SHIFT;
//	update_keyboard();
//	if (p6key) d_timer->write_signal(SIG_TIMER_IRQ_SUB_CPU, 1, 1);
	
	if(CasSkipFlag != 0) {
		request_skip_frames();
		CasSkipFlag = 0;
	}
}

void PSUB::event_callback(int event_id, int err)
{
	if(event_id == EVENT_CASSETTE) {
		if(CasMode == CAS_LOADING) {
			if(play && CasIndex < CasLength) {
				CasIntFlag = 1;
				CasRecv = CasData[CasIndex++];
				if(CasIndex < CasLength) {
					if(CasIndex == 16) {
						register_event(this, EVENT_CASSETTE, 1000000.0, false, NULL);
					} else {
						register_event(this, EVENT_CASSETTE, 1000000.0 / (CasBaud / 12), false, NULL);
					}
					String screenMessage = "TAPE READ " + String(100 * CasIndex / CasLength) + " %";
					emu->set_screen_message(screenMessage);
					emu->out_message(_T("CMT: Play (%d %%)"), 100 * CasIndex / CasLength);
				} else {
					emu->out_message(_T("CMT: Stop (End-of-Tape)"));
					emu->set_screen_message("");
				}
				d_timer->write_signal(SIG_TIMER_IRQ_SUB_CPU, 1, 1);
			}
		}
	} else if(event_id == EVENT_STRIG) {
		StrigIntFlag = 1;
		StrigEventID = -1;
		d_timer->write_signal(SIG_TIMER_IRQ_SUB_CPU, 1, 1);
	}
}

uint32_t PSUB::get_intr_ack()
{
	if (CasMode != CAS_NONE && p6key == 0xFA && kbFlagGraph) {
		return(INTADDR_CMTSTOP); /* Press STOP while CMT Load or Save */
	} else if (StrigIntFlag) { /* if command 6 */
		StrigIntFlag=-1;
		return(INTADDR_STRIG);
	} else if (p6key) { /* if any key pressed */
		p6key = 0;
		if (kbFlagGraph || kbFlagFunc) {
			kbFlagFunc=0;
			return(INTADDR_KEY2); /* special key (graphic key, etc.) */
		} else {
			return(INTADDR_KEY1); /* normal key */
		}
	} else if (CasIntFlag) {
		CasIntFlag = 0;
		return(INTADDR_CMTREAD);
	}
	return(INTADDR_TIMER);
}

void PSUB::write_io8(uint32_t addr, uint32_t data)
{
	uint16_t port=(addr & 0x00ff);
	if (port == 0x90) {
		if (CasMode == CAS_SAVEBYTE) {  /* CMT SAVE */
			if (CasIndex<0x10000) {
				CasData[CasIndex++]=data;
				CasSkipFlag=1;
			}
			CasMode=CAS_NONE;
			emu->set_screen_message("");
		}
		else if(data==0x06) {
			if(StrigEventID != -1) {
				cancel_event(this, StrigEventID);
				StrigEventID = -1;
			}
//			register_event(this, EVENT_STRIG, 3000, false, &StrigEventID); // 3msec
			register_event(this, EVENT_STRIG, 100, false, &StrigEventID); // 0.1msec
		}
		else if (data==0x3e || data==0x3d) { //	１－１）0x3E 受信(1200baud）　または　0x3D 受信(600baud）
			CasBaud=(data==0x3e)?1200:600;
		}
		else if (data==0x39) { ///
			CasMode=CAS_NONE;
			emu->set_screen_message("");
		}
		else if (data==0x38) { /* CMT SAVE DATA */
			CasMode=CAS_SAVEBYTE;
		}
		else if (data==0x1e || data==0x1d) { //	１－１）0x1E 受信(1200baud）　または　0x1D 受信(600baud）
			CasBaud=(data==0x1e)?1200:600;
		}
		else if (data==0x1a && CasMode!=CAS_NONE) { /* CMT LOAD STOP */
			CasMode=CAS_NONE;
			emu->set_screen_message("");
		}
		/* CMT LOAD OPEN(0x1E,0x19(1200baud)/0x1D,0x19(600baud)) */
		else if (data==0x19) {
			if (play && CasMode != CAS_LOADING /*&& CasBaud == FileBaud*/) {
				register_event(this, EVENT_CASSETTE, 1000000.0 / (CasBaud / 12), false, NULL);
			}
			CasRecv=0xff;
			CasMode=CAS_LOADING;
		}
	}
	d_pio->write_io8(addr, data);
}

uint32_t PSUB::read_io8(uint32_t addr)
{
	uint16_t port=(addr & 0x00ff);
	uint8_t Value=0xff;
	if (port == 0x90) {
		if (CasMode == CAS_LOADING) {
			Value=CasRecv;
			CasRecv=0xff;
			CasSkipFlag=1;
		} else if (StrigIntFlag==-1) {
			Value=stick0;
			StrigIntFlag=0;
		} else {
			Value = (d_pio->read_io8(addr));
		}
	}
	return Value;
}

void PSUB::key_down(int code)
{
//	key_stat[code] = 0x80;
	update_keyboard(code);
}

void PSUB::key_up(int code)
{
//	key_stat[code] = 0x00;
}

#define STATE_VERSION	1

bool PSUB::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	// pre process
	if(loading) {
		close_tape();
	}
	state_fio->StateValue(play);
	state_fio->StateValue(rec);
	state_fio->StateValue(is_wav);
	state_fio->StateValue(is_p6t);
	state_fio->StateArray(rec_file_path, sizeof(rec_file_path), 1);
	if(loading) {
		int length_tmp = state_fio->FgetInt32_LE();
		if(rec) {
			fio->Fopen(rec_file_path, FILEIO_READ_WRITE_NEW_BINARY);
			while(length_tmp != 0) {
				uint8_t buffer_tmp[1024];
				int length_rw = min(length_tmp, (int)sizeof(buffer_tmp));
				state_fio->Fread(buffer_tmp, length_rw, 1);
				if(fio->IsOpened()) {
					fio->Fwrite(buffer_tmp, length_rw, 1);
				}
				length_tmp -= length_rw;
			}
		}
	} else {
		if(rec && fio->IsOpened()) {
			int length_tmp = (int)fio->Ftell();
			fio->Fseek(0, FILEIO_SEEK_SET);
			state_fio->FputInt32_LE(length_tmp);
			while(length_tmp != 0) {
				uint8_t buffer_tmp[1024];
				int length_rw = min(length_tmp, (int)sizeof(buffer_tmp));
				fio->Fread(buffer_tmp, length_rw, 1);
				state_fio->Fwrite(buffer_tmp, length_rw, 1);
				length_tmp -= length_rw;
			}
		} else {
			state_fio->FputInt32_LE(0);
		}
	}
	state_fio->StateValue(CasIntFlag);
	state_fio->StateValue(CasIndex);
	state_fio->StateValue(CasRecv);
	state_fio->StateValue(CasMode);
	state_fio->StateValue(CasBaud);
	state_fio->StateValue(FileBaud);
	state_fio->StateArray(CasData, 0x10000, 1);
	state_fio->StateValue(CasLength);
	state_fio->StateValue(CasSkipFlag);
	state_fio->StateValue(kbFlagFunc);
	state_fio->StateValue(kbFlagGraph);
	state_fio->StateValue(kbFlagCtrl);
	state_fio->StateValue(kanaMode);
	state_fio->StateValue(katakana);
	state_fio->StateValue(p6key);
	state_fio->StateValue(stick0);
	state_fio->StateValue(StrigIntFlag);
	state_fio->StateValue(StrigEventID);
	return true;
}

