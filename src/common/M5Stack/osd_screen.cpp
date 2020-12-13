/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.20-

	M5Stack version.
	modified by shikarunochi 2019.03.21 -

	[ M5Stack screen ]
*/
#pragma GCC optimize ("O3")

#include "osd.h"

//#define USE_DMA_DRAW //SDと同時使用無理なので、ちょっと厳しい

#ifdef USE_DMA_DRAW
#include "DMADrawer.h"
#endif

#ifdef USE_128X64OLED
#include <Adafruit_SSD1306.h>
#include <Adafruit_I2CDevice.h>
#define OLED_SCREEN_WIDTH 128 // OLED display width, in pixels
#define OLED_SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET);
int oledRefreshCount = 0;
#endif

#define REC_VIDEO_SUCCESS	1
#define REC_VIDEO_FULL		2
#define REC_VIDEO_ERROR		3

#define USE_DRAWBITMAP

#ifndef USE_DRAWBITMAP
static TFT_eSprite fb = TFT_eSprite(&M5.Lcd);
#endif

#ifdef USE_DMA_DRAW
DMADrawer _dma;
TFT_eSPI &Tft = M5.Lcd;
TFT_eSPI* _tft = &Tft;
DMA_eSprite _sprites[2];
static uint16_t tft_width = 320;
static uint16_t tft_height = 240;
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
#ifdef USE_DMA_DRAW
    setup_t s;
    _tft->getSetup(s);
	Serial.println("init dma");
    _dma.init(s);
	Serial.println("init dma OK");
	tft_width = _tft->width();
    tft_height = _tft->height();
	//uint16_t h = max(80, (tft_height + 2) / 3);
    //_sprites[0].createDMA(tft_width, h);
    //_sprites[1].createDMA(tft_width, h);
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
	#ifdef USE_SCREEN_FILTER
	screen_skip_line = false;
	#endif
	for(int i = 0;i < 4;i++){
		diskStatus[i] = 0;
	}

	#ifdef USE_128X64OLED
  		if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    	for(;;); // Don't proceed, loop forever
	  }
	  display.clearDisplay();
  		display.setTextSize(1);             // Normal 1:1 pixel scale
  		display.setTextColor(SSD1306_WHITE);        // Draw white text
  		display.setCursor(0,0);             // Start at top-left corner
  		//display.println(F("Initializing"));
		display.drawRect(0, 0, 106, OLED_SCREEN_HEIGHT, SSD1306_WHITE);
		display.drawLine(0, 0, 106, OLED_SCREEN_HEIGHT, SSD1306_WHITE);
		display.drawLine(0, OLED_SCREEN_HEIGHT, 106, 0, SSD1306_WHITE);
  		display.display();
	#endif
}

void OSD::release_screen()
{

}

void OSD::set_vm_screen_lines(int lines)
{
//	set_vm_screen_size(vm_screen_width, lines, vm_window_width, vm_window_height, vm_window_width_aspect, vm_screen_height);
}
void OSD::set_vm_screen_size(int, int, int, int, int, int){
	//set_vm_screen_size(vm_screen_width, lines, vm_window_width, vm_window_height, vm_window_width_aspect, vm_screen_height);
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

#ifdef USE_DMA_DRAW
	dmaDraw(draw_screen_buffer);
#else
	M5.Lcd.drawBitmap(screenOffsetX, screenOffsetY, vm_screen_width, vm_screen_height, (uint16_t *)lpBmp);
#endif

#ifdef USE_128X64OLED
//320x200 を、128x64 に表示する。 縦横 1/3 で表示。
	oledRefreshCount++;
	//まずは遅くても表示。
	if(oledRefreshCount > 2){
		display.clearDisplay();
		int displayXWidth = vm_screen_width / 3;
		int OLED_XOffset = (vm_screen_width - displayXWidth * 3) / 2;
		for(int y = 0;y < OLED_SCREEN_HEIGHT;y++){
			
			for(int x = 0;x < displayXWidth;x++){
				bool oledPixel = false;
				//対応する3*3ドットのどこかが0じゃなければ点灯
				for(int dotY = 0;dotY < 3;dotY++){				
					for(int dotX = 0;dotX < 3;dotX++){
						if(*(lpBmp + (x * 3 + dotX) + (y * 3 + dotY ) * vm_screen_width )> 0){
							oledPixel = true;
							break;
						}
					}
					if(	oledPixel == true){
						break;
					}
				}	
				if(	oledPixel == true){
					display.drawPixel(x + OLED_XOffset, y, SSD1306_WHITE);
				}
			}
		}

		//display.drawBitmap(0, 0, oled_bitmap, OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT,SSD1306_WHITE);
		display.display();
		oledRefreshCount = 0;
	}
#endif
	
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
	for(int i = 0;i < 4;i++){
		if(diskStatus[i] != 0){
			uint32_t color = TFT_BLACK;
			switch(diskStatus[i]){
				case 1:
					color = TFT_GREEN;
					diskStatus[i] = 3;
					break;
				case 2:
					color = TFT_RED;
					diskStatus[i] = 3;
					break;
				case 3:
					diskStatus[i] = 0;
					break;
			}
			M5.Lcd.fillRect(1 + 0 + i * 10,235,8,4,color);
		}
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

void OSD::set_disk_status(int drvNo, int status){
	//Serial.printf("set_disk_status:%d :%d\n",drvNo, status);
	if(diskStatus[drvNo] != status){
		diskStatus[drvNo] = status;
	}else{
		diskStatus[drvNo] = 3;//同じstatusが連続で来たら一旦消して点滅に見せる
	}
}

#ifdef USE_DMA_DRAW
void OSD::dmaDraw(bitmap_t *draw_screen_buffer){
	//Serial.println("draw dma");
	for(int height = 0;height < vm_screen_height;height = height + 80){
		int drawHeight = 80;
		if(height + drawHeight >= vm_screen_height){
			drawHeight = vm_screen_height - height;
		}
		_dma.draw(screenOffsetX, screenOffsetY + height, vm_screen_width, drawHeight, (uint16_t *)(draw_screen_buffer->get_buffer(height)));
	}
}
#endif