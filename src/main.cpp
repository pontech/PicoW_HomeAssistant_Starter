#include <Arduino.h>
#include "HAIntegration.h"
#include "Network.h"

HAIntegration integration;

void setup() {
  Serial.begin();

  delay(3000); //Give the serial terminal a chance to connect, if present

  Serial.println("Pico Pi Serial Connected");

  Network::connect();

  Serial.println("integration.configure()");
  integration.configure();
}

void loop() {
  integration.loop();
}
