#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "fauxmoESP.h"
#include "credentials.h"
#include "Device.h"

#define SERIAL_BAUDRATE                 115200
#define DEVICES_NUMBER                  3

// Device id corresponds to its index
Device devices[DEVICES_NUMBER] = {
  { pin: D1 , name: "switch kitchen"    },
  { pin: D3 , name: "switch livingroom" },
  { pin: D4 , name: "switch bedroom"    }
};

fauxmoESP fauxmo;
bool stateAll = false;

void wifiSetup() {

    // Set WIFI module to STA mode
    WiFi.mode(WIFI_STA);

    // Connect
    Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    // Wait
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(100);
    }
    Serial.println();

    // Connected!
    Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());

}

void setup() {

    // Init serial port and clean garbage
    Serial.begin(SERIAL_BAUDRATE);
    Serial.println();
    Serial.println();

    // Wifi
    wifiSetup();

    // You can enable or disable the library at any moment
    // Disabling it will prevent the devices from being discovered and switched
    fauxmo.enable(true);

    // Add LED devices
    for (int i = 0; i < DEVICES_NUMBER; i++) {
      pinMode(devices[i].pin, OUTPUT);
      digitalWrite(devices[i].pin, HIGH);

      // Add virtual device
      fauxmo.addDevice(devices[i].name);
    }

    // Group virtual devices
    fauxmo.addDevice("switch everything");

    // fauxmoESP 2.0.0 has changed the callback signature to add the device_id, this WARRANTY
    // it's easier to match devices to action without having to compare strings.
    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state) {
        Serial.printf("[MAIN] Device #%d (%s) state: %s\n", device_id, device_name, state ? "ON" : "OFF");
        switch(device_id){
          case 0:
          case 1:
          case 2:
            digitalWrite(devices[device_id].pin, state);
            stateAll = state;
            break;
          case 3:
            for (int i = 0; i < DEVICES_NUMBER; i++) {
              digitalWrite(devices[i].pin, state);
            }
            stateAll = !state;
            break;
          default:
            Serial.printf("Unhandled device #%d\n", device_id);
            break;
        }
    });

    fauxmo.onGetState([](unsigned char device_id, const char * device_name) {
        switch(device_id){
          case 0:
          case 1:
          case 2:
            return digitalRead(devices[device_id].pin) == HIGH;
            break;
          case 3:
            return stateAll;
          default:
            Serial.printf("Unhandled device #%d\n", device_id);
            return false;
            break;
        }
    });

}

void loop() {
  fauxmo.handle();
}

