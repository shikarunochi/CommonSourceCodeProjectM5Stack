/*
	M5Stack version.
	shikarunochi 2019.03.21 -
*/	
#include <M5Stack.h>
#include "emu.h"

void setup() {
  M5.begin();
  Wire.begin();
  Serial.println("Start!");
  emuMain();
}

void loop() {
  // put your main code here, to run repeatedly:
}