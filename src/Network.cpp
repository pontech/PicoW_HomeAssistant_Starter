#include <WiFi.h>
#include "Network.h"
#include "Credentials.h"

uint8_t status;

void Network::connect() {
  do {

    Serial.print("Attempting to connect to WPA SSID: ");

    Serial.println(WIFI_SSID);

    // Connect to WPA/WPA2 network:

    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD); //Set these credentials

    // wait to connect:

    delay(5000);

  } while (status != WL_CONNECTED);

  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
}

void Network::disconnect() {
  do {
    Serial.println("Attempting to disconnect from WiFi");
    status = WiFi.disconnect();
    delay(1000);
  } while (status != WL_DISCONNECTED);

  Serial.print("Disconnected from ");
  Serial.println(WIFI_SSID);
}

const char* Network::WiFiStatusString(uint8_t wifi_status)
{
  switch (wifi_status)
  {
  case WL_NO_MODULE: // Same as WL_NO_SHIELD:
    return "WL_NO_MODULE";
  case WL_IDLE_STATUS:
    return "WL_IDLE_STATUS";
  case WL_NO_SSID_AVAIL:
    return "WL_NO_SSID_AVAIL";
  case WL_SCAN_COMPLETED:
    return "WL_SCAN_COMPLETED";
  case WL_CONNECTED:
    return "WL_CONNECTED";
  case WL_CONNECT_FAILED:
    return "WL_CONNECT_FAILED";
  case WL_CONNECTION_LOST:
    return "WL_CONNECTION_LOST";
  case WL_DISCONNECTED:
    return "WL_DISCONNECTED";
  case WL_AP_LISTENING:
    return "WL_AP_LISTENING";
  case WL_AP_CONNECTED:
    return "WL_AP_CONNECTED";
  case WL_AP_FAILED:
    return "WL_AP_FAILED";
  default:
    return "Unknown Status Code";
  }
}
