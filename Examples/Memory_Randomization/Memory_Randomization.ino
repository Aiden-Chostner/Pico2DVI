#include <cmath>

int seed;

void setup() {
  Serial.begin(9600);
  seed = get_rand_64();
  delay(4000);
}

void loop() {
  if(address == 8) {
    address = 0;
    chip++;
  }
  if(chip == 8) {
    chip = 0;
  }
  Serial.println(chip);
  Serial.println(chipRandom(chip));
  Serial.println(address);
  Serial.println(addressRandom(address));
  address++;
  delay(100);
}

//random access functions
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