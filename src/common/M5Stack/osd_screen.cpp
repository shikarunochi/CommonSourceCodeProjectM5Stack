/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.20-

	M5Stack version.
	modified by shikarunochi 2019.03.21 -

	[ M5Stack screen ]
*/

#include "osd.h"

#define REC_VIDEO_SUCCESS	1
#define REC_VIDEO_FULL		2
#define REC_VIDEO_ERROR		3

#define USE_DRAWBITMAP

#ifndef USE_DRAWBITMAP
static TFT_eSprite fb = TFT_eSprite(&M5.Lcd);
#endif

void OSD::initialize_screen()
{
	Serial.println("Initializing Graphic System ... ");
#ifndef USE_DRAWBITMAP
    fb.setColorDepth(8);
    fb.createSprite(320, 240);
    fb.fillSprite(TFT_BLACK);
    fb.pushSprite(0, 0);
#endif
	//host_window_width = WINDOW_WIDTH;
	//host_window_height = WINDOW_HEIGHT;
	//host_window_mode = true;
	
	vm_screen_width = SCREEN_WIDTH;
	vm_screen_height = SCREEN_HEIGHT;
	vm_window_width = WINDOW_WIDTH;
	vm_window_height = WINDOW_HEIGHT;
	vm_window_width_aspect = WINDOW_WIDTH_ASPECT;
	vm_window_height_aspect = WINDOW_HEIGHT_ASPECT;
	
	memset(&vm_screen_buffer, 0, sizeof(bitmap_t));
	initialize_screen_buffer(&vm_screen_buffer, vm_screen_width , vm_screen_height , 0);
	draw_screen_buffer = &vm_screen_buffer;

	screenOffsetX = 0;
	screenOffsetY = 0;
	if(SCREEN_WIDTH < 320){
		screenOffsetX = (320 - SCREEN_WIDTH) / 2;
	}
	if(SCREEN_HEIGHT < 240){
		screenOffsetY = (240 - SCREEN_HEIGHT) / 2;
	}
	screenMessage = "";
	preScreenMessage = "";
}

void OSD::release_screen()
{

}
scrntype_t* OSD::get_vm_screen_buffer(int y)
{
	return vm_screen_buffer.get_buffer(y);
}
int OSD::draw_screen()
{
	vm->draw_screen();
	draw_screen_buffer = &vm_screen_buffer;
	scrntype_t* lpBmp =  draw_screen_buffer->lpBmp;
#ifdef USE_DRAWBITMAP
	M5.Lcd.drawBitmap(screenOffsetX, screenOffsetY, vm_screen_width, vm_screen_height, (uint16_t *)lpBmp);
	if(preScreenMessage.equals(screenMessage)==false){
		M5.Lcd.fillRect(0, 220, 320,20,TFT_BLACK);
		preScreenMessage = screenMessage;
	}
	if(screenMessage.length()>0){
		M5.Lcd.setCursor(0, 220);
		M5.Lcd.setTextSize(2);
		M5.Lcd.setTextColor(TFT_WHITE);
		M5.Lcd.println(screenMessage);
	}
	
#else
	scrntype_t offset = 0;
	for(int y = 0;y < vm_screen_height;y++){
		for(int x = 0;x < vm_screen_width;x++){
			fb.drawPixel(x + screenOffsetX,y + screenOffsetY,*(lpBmp + offset));
			offset++;
		}
	}
	fb.pushSprite(0,0);
#endif
return 0;
}

void OSD::initialize_screen_buffer(bitmap_t *buffer, int width, int height, int mode)
{
	release_screen_buffer(buffer);
	buffer->height = height;
	buffer->width = width;
	buffer->lpBmp = (scrntype_t*)ps_malloc(width * height * sizeof(scrntype_t));	
}

void OSD::release_screen_buffer(bitmap_t *buffer)
{
	free(buffer->lpBmp);
}

//#ifdef USE_SCREEN_ROTATE
void OSD::rotate_screen_buffer(bitmap_t *source, bitmap_t *dest)
{
}
//#endif

void OSD::set_screen_message(String message){
	screenMessage = message;
}
