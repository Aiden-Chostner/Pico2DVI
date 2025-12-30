#include <PicoDVI.h>
#include <Adafruit_GFX.h>
#include <cmath>

//initialize display 1bpp 640x480 60 fps no double buffer
DVIGFX1 display(DVI_RES_640x480p60, false, Pico2DVI_cfg);

//calibration
float temp;
float temp1 = 0;
float temp2 = 0;
unsigned long previousTime = 0;
float power = 0;

void setup() {
  //setup text
  display.begin();
  display.setTextColor(1);
  display.setTextSize(1);
  display.setCursor(10, 10);

  //setup pins
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(28, OUTPUT);
}

void loop() {
  //cooling test
  unsigned long currentTime = millis();

  //run cooling
  if(currentTime - previousTime <= 90000) {
    if(((float(currentTime - previousTime)/1000) - trunc(float(currentTime - previousTime)/1000)) >= power) {
      digitalWrite(28, LOW);
    } else {
      digitalWrite(28, HIGH);
    }
  } else { //stop cooling
    digitalWrite(28, LOW);
  }

  //read temp1
  if((88000 <= currentTime - previousTime) && (currentTime - previousTime <= 89000)) {
    temp1 = temp;
  }

  //read temp2
  if((118000 <= currentTime - previousTime) && (currentTime - previousTime <= 119000)) {
    temp2 = temp;
  }

  //reset millis
  if(currentTime - previousTime > 120000) {
    previousTime = currentTime;
    power = power + 0.1;
  }

  //reset power
  if(power >= 1.1) {
    power = 0;
  }

  //read temp
  temp=analogReadTemp();
  display.fillScreen(0);
  display.setCursor(0, 0);
  display.println(String(float(currentTime - previousTime)/1000) + " Seconds");
  display.println(String(temp) + "c Current");
  display.println(String(temp1) + "c Temp1");
  display.println(String(temp2) + "c Temp2");
  display.println(String(power*100) + "% Power");
  delay(50);
}