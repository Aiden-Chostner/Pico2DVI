#include <PicoDVI.h>
#include <Adafruit_GFX.h>
#include <cmath>

DVIGFX16 display(DVI_RES_320x240p60, Pico2DVI_cfg);

//pins
const int cooler = 28;

//extra io
const int IO1 = 11;
const int IO2 = 10;
const int IO3 = 9;
const int IO4 = 12;
const int IO5 = 27;

//random
int seed;

//memory
const int CLK = 13;
const int IN = 15;
const int OUT = 14;
const int MCS0 = 26;
const int MCS1 = 22;
const int MCS2 = 21;
const int MCS3 = 20;
const int MCS4 = 19;
const int MCS5 = 18;
const int MCS6 = 17;
const int MCS7 = 16;

//time
unsigned long gpuTime = 0;

//fps and ms
float deltaTime = 16.67;
float fps = 60;

//cooling
float temp = 30;
int targetTemp = 28;
float power = 0;
bool pwm = 0;
bool overide = 0;

//colors
const int black = 0x0000;
const int red = 0xF800;
const int green = 0x07E0;
const int blue = 0x001F;
const int yellow = 0xFFE0;
const int cyan = 0x07FF;
const int magenta = 0xF81F;
const int gray1 = 0x4208; 
const int gray2 = 0x8410;
const int gray3 = 0xC618;
const int white = 0xFFFF;

bool buffer0[320*240];
bool buffer1[320*240];

/* EXAMPLES
display

display.fillScreen(color);
display.width()
display.height()

text
display.setTextColor(color);
display.setTextSize(s);
display.print(text, color);
display.println(text, color);

shapes
display.drawPixel(int, int, color);
display.drawLine(x0, y0, x1, y1, color);
display.drawTriangle(x0, y0, x1, y1, x2, y2, color);
display.fillTriangle(x0, y0, x1, y1, x2, y2, color);
display.drawRect(x, y, width, height, color);
display.fillRect(x, y, width, height, color);
display.drawCircle(x0, y0, r, color);
display.fillCircle(x0, y0, r, color);

SPI instructions
SPIM(chipRandom(chip), type, instruction, address, data)

SPIM(chip, 1, 0x83, 0x00000(0-4), 0) returns identity 0x20, 0x00, 0x16, 0x00, or 0xFF
SPIM(chipRandom(chip), 0, 0x06, 0, 0); enables write

chip erase
SPIM(chipRandom(chip), 3, 0xC7, 0, 0); 32-Mbit/15 ms USE MINIMALLY BECAUSE ONLY 100 CYCLES

read/write
SPIM(chipRandom(chip), 2, 0x02, addressRandom(address), data); byte/2 ms
SPIM(chipRandom(chip), 1, 0x03, addressRandom(address), 0) read byte
*/
void setup() {
  display.begin();

  //seed for memory
  seed = get_rand_64();

  //pins
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  pinMode(cooler, OUTPUT);

  //memory
  pinMode(CLK, OUTPUT);
  pinMode(IN, INPUT);
  pinMode(OUT, OUTPUT);

  pinMode(MCS0, OUTPUT);
  pinMode(MCS1, OUTPUT);
  pinMode(MCS2, OUTPUT);
  pinMode(MCS3, OUTPUT);
  pinMode(MCS4, OUTPUT);
  pinMode(MCS5, OUTPUT);
  pinMode(MCS6, OUTPUT);
  pinMode(MCS7, OUTPUT);

  digitalWrite(CLK, LOW);
  digitalWrite(OUT, LOW);

  digitalWrite(MCS0, HIGH);
  digitalWrite(MCS1, HIGH);
  digitalWrite(MCS2, HIGH);
  digitalWrite(MCS3, HIGH);
  digitalWrite(MCS4, HIGH);
  digitalWrite(MCS5, HIGH);
  digitalWrite(MCS6, HIGH);
  digitalWrite(MCS7, HIGH);

  //check memory
  bool error = 0;
  display.fillScreen(red);
  display.setTextSize(1);
  display.setTextColor(black);
  display.setCursor(5, 13);
  if (!(SPIM(MCS0, 1, 0x83, 0x000000, 0) == 0x20)||
      !(SPIM(MCS0, 1, 0x83, 0x000001, 0) == 0x00)||
      !(SPIM(MCS0, 1, 0x83, 0x000002, 0) == 0x16)||
      !(SPIM(MCS0, 1, 0x83, 0x000003, 0) == 0x00)||
      !(SPIM(MCS0, 1, 0x83, 0x000004, 0) == 0xFF)
  ) {
    display.println("CHECK ORIENTATION OF M0");
    error = 1;
  }
  
  for (int i = 0; i < 7; i++) {
    if (!(SPIM(i+16, 1, 0x83, 0x000000, 0) == 0x20)||
        !(SPIM(i+16, 1, 0x83, 0x000001, 0) == 0x00)||
        !(SPIM(i+16, 1, 0x83, 0x000002, 0) == 0x16)||
        !(SPIM(i+16, 1, 0x83, 0x000003, 0) == 0x00)||
        !(SPIM(i+16, 1, 0x83, 0x000004, 0) == 0xFF)
    ) {
    display.setCursor(5, 21+(8*i));
    display.println("CHECK ORIENTATION OF M" + String(i+1));
    error = 1;
    }
  }

  if (error == 1) {
    display.setCursor(5, 5);
    display.println("MEMORY TEST FAILED");
    while(true) {}
  } else {
    display.fillScreen(black);
    display.setTextSize(1);
    display.setTextColor(green);
    display.setCursor(5, 5);
    display.println("MEMORY TEST PASSED");
    delay(5000);
    display.fillScreen(black);
  }

  //initial burst
  digitalWrite(cooler, HIGH);
  delay(1000);
  digitalWrite(cooler, LOW);

  for (int i=0; i<240*320; i++) {
    buffer0[i] = random(0,2);
    buffer1[i] = 0;
  }
}

void loop() {
  unsigned long currentTime = millis();
  
  overide = 0;
  temp = analogReadTemp();

  //update cooling every second
  if(currentTime - gpuTime >= 1000) {
    cooling();
  }

  //cooling pwm
  if(currentTime - trunc(currentTime) <= power) {
    digitalWrite(cooler, HIGH);
  } else {
    digitalWrite(cooler, LOW);
  }

  //render
  render(currentTime);
}

void cooling() {
  //overides
  if(temp > 40) {
    power = 0;
    overide = 1; 
  }

  if(temp > 30 && temp <= 40) {
    power = 0.5;
    overide = 1;
  }
  
  if(temp <= -0.25) {
    power = 0.75;
    overide = 1;
  }

  if(temp < targetTemp && overide == 0) {
    power = 0;
    overide = 1;
  }

  //fan speed brackets based on target temperature
  if(overide == 0) {
    if(targetTemp <= 30 && targetTemp >= 27) power = 0.2;
    else if(targetTemp < 27 && targetTemp >= 24) power = 0.3;
    else if(targetTemp < 24 && targetTemp >= 17) power = 0.4;
    else if(targetTemp < 17 && targetTemp >= 14) power = 0.5;
    else if(targetTemp < 14 && targetTemp >= 11) power = 0.6;
    else if(targetTemp < 11 && targetTemp >= 8) power = 0.7;
    else if(targetTemp < 8 && targetTemp >= 5) power = 0.8;
    else if(targetTemp < 5 && targetTemp >= 4) power = 0.9;
    else if(targetTemp < 4) power = 1;
  }
}

void render(unsigned long currentTime) {
  for (int y = 0; y < display.height(); y++) {
    for (int x = 0; x < display.width(); x++) {
      if (!(x < 170 && y < 66)) { //draw every pixel except on overlay to prevent flickering
        int index = y*320+x;
        int neighbors = buffer0[index-321] + buffer0[index-320] + buffer0[index-319] + buffer0[index-1] + buffer0[index+1] + buffer0[index+319] + buffer0[index+320] + buffer0[index+321];
        if (neighbors<=1 || neighbors>=4) {
          buffer1[index] = 0;
        }
        if(neighbors>=2 && neighbors <=3 && buffer0[index]==1) {
          buffer1[index] = 1;
        }
        if(neighbors==3 && buffer0[index]==0) {
          buffer1[index] = 1;
        }
        display.drawPixel(x, y, buffer0[index]*65535);
      }
    }
  }
  
  for (int i=0; i<240*320; i++) {
    buffer0[i] = buffer1[i];
  }
  delay(50);
  overlay(currentTime);
}

void overlay (unsigned long currentTime) {
  //fps and ms
  deltaTime = millis() - currentTime;
  fps = 1000/deltaTime;

  //overlay box
  display.setTextSize(1);
  display.fillRect(0, 0, 170, 66, gray1);

  //title
  display.setTextColor(white);
  display.setCursor(5, 5);
  display.println("Performance Overlay");

  //stats
  display.setTextColor(gray3);
  display.setCursor(5, 13); display.println("FPS");
  display.setCursor(5, 21); display.println("Render Latency");
  display.setCursor(5, 29); display.println("GPU Temperature");
  display.setCursor(5, 37); display.println("GPU Clock");
  display.setCursor(5, 45); display.println("Memory Clock");
  display.setCursor(5, 53); display.println("Fan Speed");
  
  //numbers
  display.setTextColor(green);
  display.setCursor(100, 13); display.println(String(fps, 3) + " FPS");
  display.setCursor(100, 21); display.println(String(deltaTime, 0) + " ms");
  display.setCursor(100, 29); display.println(String(temp) + " C");
  display.setCursor(100, 37); display.println(String(rp2040.f_cpu()/1000000) + " MHz");
  display.setCursor(100, 45); display.println("500 KHz");
  display.setCursor(100, 53); display.println(String(power*100) + "%");
}

int SPIM(int chip, int type, int instruction, uint32_t address, int data) {

  //chip enable
  digitalWrite(chip, LOW);

  //instruction
  for (int i = 0; i < 8; i++) {
    digitalWrite(OUT, (instruction >> 7-i) & 1);
    digitalWrite(CLK, HIGH);
    delayMicroseconds(1);
    digitalWrite(CLK, LOW);
    delayMicroseconds(1);
  }
  
  //address
  if(type == 1 || type == 2 || type == 3) {
    for (int i = 0; i < 24; i++) {
      digitalWrite(OUT, (address >> 23-i) & 1);
      digitalWrite(CLK, HIGH);
      delayMicroseconds(1);
      digitalWrite(CLK, LOW);
      delayMicroseconds(1);
    }
  }

  //exit for instruction or erase
  if(type == 0 || type == 3) {
    digitalWrite(chip, HIGH);
  }

  //read
  if (type == 1) {
    data = 0;

    for (int i = 0; i < 8; i++) {
      data = data + digitalRead(IN)*pow(2, 7-i);
      digitalWrite(CLK, HIGH);
      delayMicroseconds(1);
      digitalWrite(CLK, LOW);
      delayMicroseconds(1);
    }
    digitalWrite(chip, HIGH);
    return(data);
  }

  //write
  if (type == 2) {
    for (int i = 0; i < 8; i++) {
      digitalWrite(OUT, (data >> 7-i) & 1);
      digitalWrite(CLK, HIGH);
      delayMicroseconds(1);
      digitalWrite(CLK, LOW);
      delayMicroseconds(1);
    }
    digitalWrite(chip, HIGH);
  }

  return(0);
}

//randomize memory access
int chipRandom (int c) {
  randomSeed(seed*(c+1));
  c = random(0, 8);
  if(c == 7) {
    return(26);
  } else {
    return(c+16);
  }
}

int addressRandom (int a) {
  randomSeed(seed*(a+1));
  return(random(0x000000, 0x400000));
}