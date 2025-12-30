#include <cmath>

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

void setup() {
  Serial.begin(9600);

  //seed for memory
  seed = get_rand_64();

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
}

/*
SPI instructions
SPIM(chipRandom(chip), type, instruction, address, data)

SPIM(chipRandom(chip), 1, 0x83, 0x00000(0-4), 0) returns identity 0x20, 0x00, 0x16, 0x00, or 0xFF
SPIM(chipRandom(chip), 0, 0x06, 0, 0); enables write

chip erase
SPIM(chipRandom(chip), 3, 0xC7, 0, 0); 32-Mbit/15 ms

read/write
SPIM(chipRandom(chip), 2, 0x02, addressRandom(address), data); byte/2 ms
SPIM(chipRandom(chip), 1, 0x03, addressRandom(address), 0) read byte
*/

void loop() {
  //read identification
  Serial.println("M0");
  Serial.println(SPIM(MCS0, 1, 0x83, 0x000000, 0x00));
  Serial.println(SPIM(MCS0, 1, 0x83, 0x000001, 0x00));
  Serial.println(SPIM(MCS0, 1, 0x83, 0x000002, 0x00));
  Serial.println(SPIM(MCS0, 1, 0x83, 0x000003, 0x00));
  Serial.println(SPIM(MCS0, 1, 0x83, 0x000004, 0x00));

  SPIM(MCS0, 0, 0x06, 0, 0);
  SPIM(MCS0, 3, 0xDB, addressRandom(0x000000), 0);
  delay(3);

  SPIM(MCS0, 0, 0x06, 0, 0);
  SPIM(MCS0, 2, 0x02, addressRandom(0x000000), 1);
  delay(3);

  SPIM(MCS0, 0, 0x06, 0, 0);
  SPIM(MCS0, 2, 0x02, addressRandom(0x000001), 2);
  delay(3);

  SPIM(MCS0, 0, 0x06, 0, 0);
  SPIM(MCS0, 2, 0x02, addressRandom(0x000002), 3);
  delay(3);

  SPIM(MCS0, 0, 0x06, 0, 0);
  SPIM(MCS0, 2, 0x02, addressRandom(0x000003), 4);
  delay(3);

  Serial.println(SPIM(MCS0, 1, 0x03, addressRandom(0x000000), 0));
  Serial.println(SPIM(MCS0, 1, 0x03, addressRandom(0x000001), 0));
  Serial.println(SPIM(MCS0, 1, 0x03, addressRandom(0x000002), 0));
  Serial.println(SPIM(MCS0, 1, 0x03, addressRandom(0x000003), 0));
  delay(1000);
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