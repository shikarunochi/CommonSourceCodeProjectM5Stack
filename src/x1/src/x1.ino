#if defined(_M5Core2)
	#include <M5Core2.h>
#else
	#include<M5Stack.h>
#include <M5StackUpdater.h>  // https://github.com/tobozo/M5Stack-SD-Updater/
#endif
#include "emu.h"

void setup() {
#if defined(_M5Core2)
  M5.begin(true,true,true,true);
#else
  M5.begin();
  Wire.begin();
  if(digitalRead(BUTTON_A_PIN) == 0) {
     Serial.println("Will Load menu binary");
     updateFromFS(SD);
     ESP.restart();
  }
#endif
  emuMain();
}

void loop() {
  // put your main code here, to run repeatedly:
}