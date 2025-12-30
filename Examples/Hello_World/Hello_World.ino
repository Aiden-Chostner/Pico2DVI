#include <PicoDVI.h>
#include <Adafruit_GFX.h>

//initialize display at 1bpp 640x480 60 fps no double buffer
DVIGFX1 display(DVI_RES_640x480p60, false, Pico2DVI_cfg);

void setup() {
  display.begin();
  display.fillScreen(0);
  display.setTextColor(1);
  display.setTextSize(1);
  display.setCursor(10, 10);
  display.print("Hello World!");

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(28, OUTPUT);
}

void loop() {
  //flash led
  digitalWrite(LED_BUILTIN, HIGH);
  delay(2000);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(28, LOW);
  delay(2000);
}