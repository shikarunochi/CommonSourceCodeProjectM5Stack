/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.26-

	M5Stack version.
	modified by shikarunochi 2019.03.21 -

	[ M5Stack input ]
*/

#include "osd.h"
#include "../fifo.h"



#define CARDKB_ADDR 0x5F
#define JOYSTICK_ADDR 0x52

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
  joyPadMode = JOYPAD_NONE;
  pressedKey = 0;
  keyInputConfig();
}

void OSD::release_input()
{
}

void OSD::update_input()
{
	//M5Stackからのキー入力によって状態を変更する
	checkKeyboard();
  if(joyPadMode != JOYPAD_NONE){
    checkJoyStick();
  }
	M5.update();
  if(joyPadMode == JOYPAD_NONE){
    if (M5.BtnB.wasReleased())
    {
      lock_vm();
      String file = selectFile();
      openFile(file);
      M5.Lcd.fillScreen(TFT_BLACK);
      unlock_vm();
    }
    if(pressedKey != 0){
      key_status[pressedKey] = 0x00;
      pressedKey = 0;
    }
  }else{
    //joystick #1, #2 (b0 = up, b1 = down, b2 = left, b3 = right, b4- = buttons
    if(M5.BtnB.wasPressed()){
      joy_status[0] |= 0b010000;
    }
    
    if(M5.BtnB.wasReleased()){
      joy_status[0] &= ~0b010000;
    }
    if(M5.BtnC.wasPressed()){
      joy_status[0] |= 0b100000;
    }
    if(M5.BtnC.wasReleased()){
      joy_status[0] &= ~0b100000;
    }
  }

  if(M5.BtnA.wasReleased()){
    joy_status[0] = 0x00;//initialize
    joyPadMode = joyPadMode + 1;
    if(joyPadMode > 2){
      joyPadMode = 0;
    }
    String joyPadStatusInfo = "";
    switch(joyPadMode){
      case JOYPAD_NONE:joyPadStatusInfo="NO JOYPAD";break;
      case JOYPAD_MODE1:joyPadStatusInfo="NORMAL JOYPAD";break;
      case JOYPAD_MODE2:joyPadStatusInfo="ROTATE JOYPAD";break;
    }
	  M5.Lcd.setCursor(0, 220);
    M5.Lcd.setTextSize(2);
    M5.Lcd.fillRect(0,210,320,30,TFT_BLACK);
    M5.Lcd.print(joyPadStatusInfo);
    delay(500);
    M5.Lcd.fillRect(0,210,320,30,TFT_BLACK);
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
          if(keyCheckFrameCount == 1){
            key_status[pressedVMKey] = 0x80;
            vm->key_down(pressedVMKey, false);
          }
          return;//キーPress状態で2フレームキープ
        }
        keyCheckFrameCount = 0;
        //vm->key_up(VK_SHIFT);
		    vm->key_up(pressedVMKey);
		    key_status[VK_SHIFT] &= 0x7f;
        key_status[VK_CONTROL] &= 0x7f;
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
			//vm->key_down(VK_SHIFT, false); 
			key_status[VK_SHIFT] = 0x80; //SHIFTを先行で入力しておく
		}else if (M5StackKeyMap[m5StackKeyCode][1] == 2){
      key_status[VK_CONTROL] = 0x80; //CTRLを先行で入力しておく
    }
	}

  return pressedVMKey;
}

//--------------------------------------------------------------
// joyStick
//--------------------------------------------------------------
//joystick #1, #2 (b0 = up, b1 = down, b2 = left, b3 = right, b4- = buttons
void OSD::checkJoyStick(){
  int joy1 = 0;
  int joy2 = 0;
  int joyPress = 0;
  int joyX = 0;
  int joyY = 0;

  if(Wire.requestFrom(JOYSTICK_ADDR,3) >= 3){
      if(Wire.available()){joy1 = Wire.read();}
      if(Wire.available()){joy2 = Wire.read();}
      if(Wire.available()){joyPress = Wire.read();}//Press
  }

  if(joyPadMode == JOYPAD_MODE1){
	  joyX = joy1;
	  joyY = joy2;
  }else{ //ROTETE
	  joyX = joy2;
	  joyY = joy1;
  }

  if(joyX == 0){ //NO JOYSTICK
    return; 
  }

  if(joyPadMode == JOYPAD_MODE1){
    //RIGHT  
    if(joyX < 80){
      joy_status[0] |= 0b001000;
    }else{
      joy_status[0] &= ~0b001000;
    }
    //LEFT
    if(joyX > 160){
      joy_status[0] |= 0b000100;
    }else{
      joy_status[0] &= ~0b000100;
    }
  }else{
    //LEFT
    if(joyX < 80){
      joy_status[0] |= 0b000100;
    }else{
      joy_status[0] &= ~0b000100;
    }
    //RIGHT
    if(joyX > 160){
      joy_status[0] |= 0b001000;
    }else{
      joy_status[0] &= ~0b001000;
    }
  }
  //UP
  if(joyY < 80){
    joy_status[0] |= 0b000001;
  }else{
    joy_status[0] &= ~0b000001;
  }
  //DOWN
  if(joyY > 160){
    joy_status[0] |= 0b000010;
  }else{
    joy_status[0] &= ~0b000010;
  }
}
//--------------------------------------------------------------
// openFile
//--------------------------------------------------------------
bool OSD::openFile(String file){
  if(file.length() > 0){
    String fileName = ("/" + String(CONFIG_NAME) + "ROM/" + file);
    const char *cFileName = fileName.c_str();
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setCursor(0, 0);

    //拡張子チェック
    fileName.toUpperCase();
    bool openFlag = false;
#ifdef USE_TAPE
    if(fileName.endsWith(".P6T")
      ||fileName.endsWith(".P6")
      ||fileName.endsWith(".CAS")
      ||fileName.endsWith(".MZT")
    ){
      vm->play_tape(0, cFileName);
      M5.Lcd.println("PLAY TAPE");
      openFlag = true;
    }
#endif
    //該当なければCARTに設定
    if(openFlag == false){        
      vm->open_cart(0, cFileName);
      M5.Lcd.println("SET CART");
    }
    //vm->reset();
    M5.Lcd.println(file);
    delay(2000);
  }
  return true;
}