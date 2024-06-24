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
        uint32_t last_start_time;
    ZoneData(uint8_t led_pin, uint8_t ssr_pin) : 
        led_pin(led_pin), 
        ssr_pin(ssr_pin)
    {
        last_start_time = 0;
    }
};

WiFiClient client;
HADevice device; // Setting uniqueId to "FlowBot-" + MAC address in HAIntegration::configure()
HAMqtt mqtt(client, device);
HASwitch led((void*) new ZoneData( LED_PIN, LED_PIN)); // unique identifier must not contain spaces

////////////////////////////////////////////////////////////////////
// These array's define connect the GP# to the specific zone.
// todo: 5 remove these arrays and just set the correct pins when
//       making the zone[].
////////////////////////////////////////////////////////////////////
const uint8_t MAX_ZONES = 10;
uint8_t zone_pins[MAX_ZONES] = { 11, 12, 13, 10, 14,  9, 15,  8,  7,  6 };
uint8_t zled_pins[MAX_ZONES] = { 16, 17, 18, 19, 20, 21, 22, 26, 27, 28 };
HASwitch zone[MAX_ZONES] = { 
    HASwitch((void*) new ZoneData(zled_pins[0], zone_pins[0])),
    HASwitch((void*) new ZoneData(zled_pins[1], zone_pins[1])),
    HASwitch((void*) new ZoneData(zled_pins[2], zone_pins[2])),
    HASwitch((void*) new ZoneData(zled_pins[3], zone_pins[3])),
    HASwitch((void*) new ZoneData(zled_pins[4], zone_pins[4])),
    HASwitch((void*) new ZoneData(zled_pins[5], zone_pins[5])),
    HASwitch((void*) new ZoneData(zled_pins[6], zone_pins[6])),
    HASwitch((void*) new ZoneData(zled_pins[7], zone_pins[7])),
    HASwitch((void*) new ZoneData(zled_pins[8], zone_pins[8])),
    HASwitch((void*) new ZoneData(zled_pins[9], zone_pins[9])),
};

HANumber floodPreventionTimeoutSeconds((HABaseDeviceType::NumberPrecision)0); //PrecisionP0);
HANumeric floodPreventionTimeoutSecondsValue = HANumeric((uint32_t)301, (HABaseDeviceType::NumberPrecision)0);

void HAIntegration::configure() {

    ///////////////////////////////////////////
    // Configure GPIO
    ///////////////////////////////////////////

    // Prepare LED:
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);    

    // Prepare Zones
    for(uint8_t i = 0; i < MAX_ZONES; i++)
    {
        pinMode(((ZoneData*)(zone[i].getData()))->led_pin, OUTPUT);
        digitalWrite(((ZoneData*)(zone[i].getData()))->led_pin, LOW);
        pinMode(((ZoneData*)(zone[i].getData()))->ssr_pin, OUTPUT);
        digitalWrite(((ZoneData*)(zone[i].getData()))->ssr_pin, LOW);
    }

    ///////////////////////////////////////////
    // Startup LED Sequence
    ///////////////////////////////////////////

    digitalWrite(LED_PIN, HIGH);    
    delay(1000);
    for(uint8_t i = 0; i < MAX_ZONES; i++)
    {
        digitalWrite(zled_pins[i], HIGH);
        delay(100);
    }

    // The setDataPrefix needs to be called before the enableSharedAvailability()/enableLastWill() are called otherwise the default value is used.
    //mqtt.setDiscoveryPrefix("homeassistant"); // [OPTIONAL] Default values is "homeassistant" and it must be this for automatic discovery
    mqtt.setDataPrefix("plp"); // [OPTIONAL] This defaults to "aha" but you can change it without issue

    ///////////////////////////////////////////
    // Set device ID as MAC address
    ///////////////////////////////////////////
    byte mac[WL_MAC_ADDR_LENGTH];
    WiFi.macAddress(mac);
    char mac_string[30];
    snprintf(mac_string,30,"FlowBot-%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], mac[6]);
    Serial.printf("%s\n", mac_string);

    device.setUniqueId(mac_string);    // [ALTERNATIVE] device.setUniqueId(mac, sizeof(mac));
    device.enableSharedAvailability(); // [OPTIONAL] Enables the ability to set the on/off line status of the device
    device.setAvailability(true); // Set the device to on-line
    device.enableLastWill(); // [OPTIONAL] Allows HA to detect when the device losses power (and thus disable the controls in the HA UI)

    //Device metadata:

    device.setName("FlowBot");
    device.setSoftwareVersion("0.1");
    device.setManufacturer("PONTECH.com");
    device.setModel("FlowBot100");

    // handle switch state
    led.HABaseSetUniqueId("LED"); // Set Unique ID becuase it was not set when instantiated
    led.onCommand(switchHandler);
    led.setName("FlowBot LED"); // optional
    led.setIcon("mdi:led-outline"); // optional (Used to set the icon used in HA)

    // Zone state
    for(uint8_t i = 0; i < MAX_ZONES; i++)
    {
        char Zone[] = "Zone";
        char* ZoneId = new char[strlen(Zone) + 2]; // include Number + null terminator
        snprintf(ZoneId, strlen(Zone) + 2, "%s%d", Zone, i);
        zone[i].HABaseSetUniqueId(ZoneId); // Set Unique ID becuase it was not set when instantiated

        zone[i].onCommand(switchHandler);
        // char str[10];
        // sprintf(str, "Zone %d", i);
        // Serial.printf("Allocating %d bytes for string '%s'\n", strlen(str) + 1, str);
        // char* heap = (char*)malloc(strlen(str) + 1);
        // zone[i].setName(heap); // optional
        zone[i].setName(zone_name[i]); // optional
        zone[i].setIcon("mdi:sprinkler-variant");
    }

    floodPreventionTimeoutSeconds.HABaseSetUniqueId("FPT"); // Set Unique ID becuase it was not set when instantiated
    floodPreventionTimeoutSeconds.onCommand(numberHandler);
    floodPreventionTimeoutSeconds.setName("Flood Prevention Timeout"); // [OPTIONAL] Defualts to "MQTT Number"
    floodPreventionTimeoutSeconds.setUnitOfMeasurement("seconds");
    floodPreventionTimeoutSeconds.setMin(1);
    floodPreventionTimeoutSeconds.setMax(65535);
    //floodPreventionTimeoutSeconds.setCurrentState(HANumeric((uint32_t)600, (HABaseDeviceType::NumberPrecision)0)); // This set the starting value reported to HA
    floodPreventionTimeoutSeconds.setCurrentState((uint32_t)(5*60)); // This set the starting value reported to HA

    Serial.print("Connecting to MQTT\n");

    if (mqtt.begin(MQTT_BROKER, MQTT_LOGIN, MQTT_PASSWORD) == true) {
        Serial.print("Connected to MQTT broker\n");
    } else {
        Serial.print("Could (re)instantiate MQTT broker\n");
    }

    for(uint8_t i = 0; i < MAX_ZONES; i++)
    {
        digitalWrite(zled_pins[i], LOW);
        delay(100);
    }
    digitalWrite(LED_PIN, LOW);
    delay(1000);
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
        ((ZoneData*)(sender->getData()))->last_start_time = millis();
}

void HAIntegration::numberHandler(HANumeric number, HANumber* sender) {
    sender->setState(number.toUInt32());  // report state back to Home Assistant
    Serial.printf("%s new value: %d\n", sender->getName(), number.toUInt32());
    floodPreventionTimeoutSecondsValue = number;
}

void HAIntegration::loop() {
    mqtt.loop();

    static uint32_t last = 0;
    if( last < millis() )
    {
        last = millis() + 1; // Test this every 1 millisecond

        uint32_t timeout = floodPreventionTimeoutSecondsValue.toInt32() * 1000;
        uint32_t now = millis();
        //Serial.printf("FPT=%ld, now= %ld, ", floodPreventionTimeoutSecondsValue.toInt32(), now);

        for(uint8_t i = 0; i < MAX_ZONES; i++)
        {
            uint32_t start = ((ZoneData*)(zone[i].getData()))->last_start_time;
            //Serial.printf("zone[%d]=%ld, ", i, start);
            if( (now - start) > timeout ) // todo: 2 This should handle millis() rollovers (untested)
            {
                digitalWrite(((ZoneData*)(zone[i].getData()))->led_pin, LOW);
                digitalWrite(((ZoneData*)(zone[i].getData()))->ssr_pin, LOW);
                zone[i].setState(false);
            }
        }
        //Serial.println();
    }

}
