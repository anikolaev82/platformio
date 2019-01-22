#include "MHZ19b.h"
#include <SoftwareSerial.h>

const int pin_tx = A0;
const int pin_rx = A1;

SoftwareSerial ss(pin_rx, pin_tx);

MHZ19b co2(&ss);

void setup() {
  Serial.begin(115200);
  co2.init();
}

void loop() {
  Serial.write(co2.get_co2());
  if(high_air == co2.get_quality_air) {
  }
  delay(10000);
}