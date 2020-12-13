#include <M5Stack.h>

#include <M5StackUpdater.h>  // https://github.com/tobozo/M5Stack-SD-Updater/
#include "emu.h"

void setup() {
  M5.begin();
  Wire.begin();
  if(digitalRead(BUTTON_A_PIN) == 0) {
     Serial.println("Will Load menu binary");
     updateFromFS(SD);
     ESP.restart();
  }
  emuMain();
}

void loop() {
  // put your main code here, to run repeatedly:
}