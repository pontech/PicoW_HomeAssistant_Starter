#include "HAIntegration.h"
#include "Credentials.h"

#include <ArduinoHA.h>
#include <WiFi.h>

//Adapted via:
//  https://github.com/dawidchyrzynski/arduino-home-assistant/blob/main/examples/nano33iot/nano33iot.ino

#define LED_PIN     LED_BUILTIN

char zone_name[][7] = {
    "Zone 0",
    "Zone 1",
    "Zone 2",
    "Zone 3",
    "Zone 4",
    "Zone 5",
    "Zone 6",
    "Zone 7",
    "Zone 8",
    "Zone 9",
};

class ZoneData {
    public:
        uint8_t led_pin;
        uint8_t ssr_pin;
    ZoneData(uint8_t led_pin, uint8_t ssr_pin) : 
        led_pin(led_pin), 
        ssr_pin(ssr_pin)
    {
    }
};

WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);
HASwitch led("led", (void*) new ZoneData( LED_PIN, LED_PIN));
HASwitch zone[] = { 
    HASwitch("Zone0", (void*) new ZoneData( 6, 16)),
    HASwitch("Zone1", (void*) new ZoneData( 7, 17)),
    HASwitch("Zone2", (void*) new ZoneData( 8, 18)),
    HASwitch("Zone3", (void*) new ZoneData( 9, 19)),
    HASwitch("Zone4", (void*) new ZoneData(10, 20)),
    HASwitch("Zone5", (void*) new ZoneData(11, 21)),
    HASwitch("Zone6", (void*) new ZoneData(12, 22)),
    HASwitch("Zone7", (void*) new ZoneData(13, 26)),
    HASwitch("Zone8", (void*) new ZoneData(14, 27)),
    HASwitch("Zone9", (void*) new ZoneData(15, 28)),
};

uint8_t zone_pins[] = {  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 };
uint8_t zled_pins[] = { 16, 17, 18, 19, 20, 21, 22, 26, 27, 28 };

void HAIntegration::configure() {

    // Prepare LED:

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);    

    // Prepare Zones
    for(uint8_t i = 0; i < 10; i++)
    {
        pinMode(zone_pins[i], OUTPUT);
        digitalWrite(zone_pins[i], LOW);
        pinMode(zled_pins[i], OUTPUT);
        digitalWrite(zled_pins[i], LOW);
    }

    digitalWrite(LED_PIN, HIGH);    
    delay(1000);
    digitalWrite(LED_PIN, LOW);    
    delay(1000);
    for(uint8_t i = 0; i < 10; i++)
    {
        digitalWrite(zled_pins[i], HIGH);
        delay(100);
        digitalWrite(zled_pins[i], LOW);
    }

    //Set device ID as MAC address

    byte mac[WL_MAC_ADDR_LENGTH];
    WiFi.macAddress(mac);
    device.setUniqueId(mac, sizeof(mac));

    //Device metadata:

    device.setName("FlowBot by PONTECH");
    device.setSoftwareVersion("0.1");

    // handle switch state
    led.onCommand(switchHandler);
    led.setName("FlowBot LED"); // optional
    led.setIcon("mdi:led-outline"); // optional

    // Zone state
    for(uint8_t i = 0; i < 10; i++)
    {
        zone[i].onCommand(switchHandler);
        // char str[10];
        // sprintf(str, "Zone %d", i);
        // Serial.printf("Allocating %d bytes for string '%s'\n", strlen(str) + 1, str);
        // char* heap = (char*)malloc(strlen(str) + 1);
        // zone[i].setName(heap); // optional
        zone[i].setName(zone_name[i]); // optional
        zone[i].setIcon("mdi:sprinkler-variant");
    }

    Serial.print("Connecting to MQTT\n");
    
    if (mqtt.begin(MQTT_BROKER, MQTT_LOGIN, MQTT_PASSWORD) == true) {
        Serial.print("Connected to MQTT broker\n");
    } else {
        Serial.print("Could (re)instantiate MQTT broker\n");
    }
}

void HAIntegration::switchHandler(bool state, HASwitch* sender) {
    digitalWrite(((ZoneData*)(sender->getData()))->led_pin, (state ? HIGH : LOW));
    digitalWrite(((ZoneData*)(sender->getData()))->ssr_pin, (state ? HIGH : LOW));

    // digitalWrite(LED_PIN, (state ? HIGH : LOW));
    sender->setState(state);  // report state back to Home Assistant
    Serial.printf("%s new state: %s (%d, %d)\n", 
        sender->getName(), state ? "on" : "off",
        ((ZoneData*)(sender->getData()))->led_pin,
        ((ZoneData*)(sender->getData()))->ssr_pin
        );
}


void HAIntegration::loop() {
    mqtt.loop();
}
