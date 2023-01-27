#include <M5Stack.h>
#include <M5StackUpdater.h>  // https://github.com/tobozo/M5Stack-SD-Updater/
#include "emu.h"

void setup() {
  
  if(digitalRead(BUTTON_A_PIN) == 0) {
     M5.begin();
     Serial.println("Will Load menu binary");
     updateFromFS(SD);
     ESP.restart();
  }

#ifdef _LGFX
  initLGFXLcd();
  Serial.begin(115200);
#else
  M5.begin();
#endif
  Wire.begin();

  emuMain();
}

void loop() {
  // put your main code here, to run repeatedly:
}