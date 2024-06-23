#include <Arduino.h>
#include <WiFi.h>
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
  static uint8_t wifi_status_last = WL_NO_MODULE;
  uint8_t wifi_status = WiFi.status();
  if(wifi_status != wifi_status_last)
  {
    Serial.println(Network::WiFiStatusString(wifi_status));
  }
  if(wifi_status == WL_CONNECTED){
    integration.loop();
  }
  else{
    Serial.println(Network::WiFiStatusString(wifi_status));
    Network::disconnect();
    Network::connect();
  }

  wifi_status_last = wifi_status;
}
