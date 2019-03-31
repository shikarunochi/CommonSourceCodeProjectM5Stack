/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.20-

	M5Stack version.
	modified by shikarunochi 2019.03.21 -

	[ M5Stack dependent ]
*/

#include "osd.h"

void OSD::initialize(int rate, int samples)
{
	initialize_input();
	initialize_screen();
	initialize_sound(rate, samples);
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	CoInitialize(NULL);
	initialize_video();
#endif
#ifdef USE_SOCKET
	initialize_socket();
#endif
}

void OSD::release()
{
	release_input();
	release_screen();
	release_sound();
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	release_video();
	CoUninitialize();
#endif
#ifdef USE_SOCKET
	release_socket();
#endif
}

void OSD::power_off()
{

}

void OSD::suspend()
{
#ifdef USE_MOVIE_PLAYER
	if(now_movie_play && !now_movie_pause) {
		pause_movie();
		now_movie_pause = false;
	}
#endif
	//mute_sound();
}

void OSD::restore()
{
#ifdef USE_MOVIE_PLAYER
	if(now_movie_play && !now_movie_pause) {
		play_movie();
	}
#endif
}

void OSD::lock_vm()
{
	lock_count++;
}

void OSD::unlock_vm()
{
	if(--lock_count <= 0) {
		force_unlock_vm();
	}
}

void OSD::force_unlock_vm()
{
	lock_count = 0;
}

void OSD::sleep(uint32_t ms)
{
	delay(ms);
}

#ifdef USE_DEBUGGER
FARPROC hWndProc = NULL;
OSD *my_osd = NULL;

LRESULT CALLBACK MyWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg) {
	case WM_CLOSE:
		return 0;
	case WM_PAINT:
		if(my_osd) {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
#ifdef ONE_BOARD_MICRO_COMPUTER
			my_osd->reload_bitmap();
#endif
			my_osd->update_screen(hdc);
			EndPaint(hWnd, &ps);
		}
		return 0;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

void OSD::start_waiting_in_debugger()
{
	HMENU hMenu = GetMenu(main_window_handle);
	
	if(hMenu != NULL) {
		for(int i = 0;; i++) {
			if(EnableMenuItem(hMenu, i, MF_BYPOSITION | MF_GRAYED) == -1) {
				break;
			}
		}
	}
	hWndProc = (FARPROC)GetWindowLong(main_window_handle, GWL_WNDPROC);
	SetWindowLong(main_window_handle, GWL_WNDPROC, (LONG)MyWndProc);
	my_osd = this;
}

void OSD::finish_waiting_in_debugger()
{
	HMENU hMenu = GetMenu(main_window_handle);
	
	if(hMenu != NULL) {
		for(int i = 0;; i++) {
			if(EnableMenuItem(hMenu, i, MF_BYPOSITION | MF_ENABLED) == -1) {
				break;
			}
		}
	}
	SetWindowLong(main_window_handle, GWL_WNDPROC, (LONG)hWndProc);
	my_osd = NULL;
}

void OSD::process_waiting_in_debugger()
{
	MSG msg;
	
	while(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
		if(GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}
#endif

#define MAX_FILES 256
String OSD::selectFile()
{
    File fileRoot;
    String fileList[MAX_FILES];

    M5.Lcd.fillScreen(TFT_BLACK);
	M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextSize(2);
    
    String fileDir = "/" + String(CONFIG_NAME) + "ROM" + String(config.filePathSuffix) ;
    if (fileDir.endsWith("/") == true)
    {
        fileDir = fileDir.substring(0, fileDir.length() - 1);
    }

	if(SD.exists(fileDir)==false){
		M5.Lcd.println("DIR NOT EXIST");
		M5.Lcd.printf("[SD:%s/]",fileDir.c_str());
		delay(5000);
		M5.Lcd.fillScreen(TFT_BLACK);
		return "";
	}
    fileRoot = SD.open(fileDir);
    int fileListCount = 0;

    while (1)
    {
        File entry = fileRoot.openNextFile();
        if (!entry)
        { // no more files
            break;
        }
        //ファイルのみ取得
        if (!entry.isDirectory())
        {
            String fullFileName = entry.name();
            String fileName = fullFileName.substring(fullFileName.lastIndexOf("/") + 1);
            fileList[fileListCount] = fileName;
            fileListCount++;
            //Serial.println(fileName);
        }
        entry.close();
    }
    fileRoot.close();

    int startIndex = 0;
    int endIndex = startIndex + 10;
    if (endIndex > fileListCount)
    {
        endIndex = fileListCount;
    }

    sortList(fileList, fileListCount);

    boolean needRedraw = true;
    int selectIndex = 0;
	int preStartIndex = 0;
    bool isLongPress = false;
    while (true)
    {

        if (needRedraw == true)
        {
            M5.Lcd.setCursor(0, 0);
            startIndex = selectIndex - 5;
            if (startIndex < 0)
            {
                startIndex = 0;
            }
            endIndex = startIndex + 12;
            if (endIndex > fileListCount)
            {
                endIndex = fileListCount;
                startIndex = endIndex - 12;
                if(startIndex < 0){
                    startIndex = 0;
                }
            }
			if(preStartIndex != startIndex){
            	M5.Lcd.fillRect(0,0,320,220,0);
				preStartIndex = startIndex;				
			}
            for (int index = startIndex; index < endIndex + 1; index++)
            {
                if (index == selectIndex)
                {
                    M5.Lcd.setTextColor(TFT_GREEN);
                }
                else
                {
                    M5.Lcd.setTextColor(TFT_WHITE);
                }
                if (index == 0)
                {
                    M5.Lcd.println("[BACK]");
                }
                else
                {
                    M5.Lcd.println(fileList[index - 1].substring(0,26));
                }
            }
            M5.Lcd.setTextColor(TFT_WHITE);

            M5.Lcd.drawRect(0, 240 - 19, 100, 18, TFT_WHITE);
            M5.Lcd.drawCentreString("U P", 53, 240 - 17, 1);
            //M5.Lcd.setCursor(35, 240 - 17);
            //M5.Lcd.print("U P");
            M5.Lcd.drawRect(110, 240 - 19, 100, 18, TFT_WHITE);
            M5.Lcd.drawCentreString("SELECT", 159, 240 - 17, 1);
            //M5.Lcd.setCursor(125, 240 - 17);
            //M5.Lcd.print("SELECT");
            M5.Lcd.drawRect(220, 240 - 19, 100, 18, TFT_WHITE);
            M5.Lcd.drawCentreString("DOWN", 266, 240 - 17, 1);
            //M5.Lcd.setCursor(245, 240 - 17);
            //M5.Lcd.print("DOWN");
            needRedraw = false;
        }
        M5.update();
        if (M5.BtnA.pressedFor(500)){
            isLongPress = true;
            selectIndex--;
            if (selectIndex < 0)
            {
                selectIndex = fileListCount;
            }
            needRedraw = true;
            delay(100);
        }
        if (M5.BtnA.wasReleased())
        {
            if(isLongPress == true)
            {
                isLongPress = false;
            }else{
                selectIndex--;
                if (selectIndex < 0)
                {
                    selectIndex = fileListCount;
                }
                needRedraw = true;
            }
        }
        if (M5.BtnC.pressedFor(500)){
            isLongPress = true;
            selectIndex++;
            if (selectIndex > fileListCount)
            {
                selectIndex = 0;
            }
            needRedraw = true;
            delay(100);
        }
        if (M5.BtnC.wasReleased())
        {
            if(isLongPress == true)
            {
                isLongPress = false;
            }else{
                selectIndex++;
                if (selectIndex > fileListCount)
                {
                    selectIndex = 0;
                }
                needRedraw = true;
            }
        }

        if (M5.BtnB.wasReleased())
        {
            if (selectIndex == 0)
            {
                //何もせず戻る
                M5.Lcd.fillScreen(TFT_BLACK);
                delay(10);
                return "";
            }
            else
            {
                delay(10);
                String selectFileName = fileList[selectIndex - 1];
                if(SD.exists(fileDir + "/" + selectFileName)==false){
                    M5.Lcd.fillScreen(TFT_BLACK);
                    M5.Lcd.setCursor(0, 0);
                    M5.Lcd.println("FILE NOT EXIST");
                    M5.Lcd.println(fileList[selectIndex - 1]);
                    delay(2000);
                    M5.Lcd.fillScreen(TFT_BLACK);
                    needRedraw = true;
                }else{
				    return selectFileName;
                }
            }
        }
        delay(100);
    }
}
/* bubble sort filenames */
//https://github.com/tobozo/M5Stack-SD-Updater/blob/master/examples/M5Stack-SD-Menu/M5Stack-SD-Menu.ino
void OSD::sortList(String fileList[], int fileListCount) { 
  bool swapped;
  String temp;
  String name1, name2;
  do {
    swapped = false;
    for(int i = 0; i < fileListCount-1; i++ ) {
      name1 = fileList[i];
      name1.toUpperCase();
      name2 = fileList[i+1];
      name2.toUpperCase();
      if (name1.compareTo(name2) > 0) {
        temp = fileList[i];
        fileList[i] = fileList[i+1];
        fileList[i+1] = temp;
        swapped = true;
      }
    }
  } while (swapped);
}
