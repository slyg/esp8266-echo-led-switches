#include <Arduino.h>
#include "fauxmoESP.h"
#include "device.h"
#include "wifi.h"

#define SERIAL_BAUDRATE   115200
#define DEVICES_NUMBER    3

// Device id corresponds to its index
device devices[DEVICES_NUMBER] = {
  { pin: D1 , name: "switch kitchen"    },
  { pin: D3 , name: "switch livingroom" },
  { pin: D4 , name: "switch bedroom"    }
};

fauxmoESP fauxmo;

void setup() {

  // Init serial port and clean garbage
  Serial.begin(SERIAL_BAUDRATE);
  Serial.println();
  Serial.println();

  wifiSetup();
  fauxmo.enable(true);

  // Add LED devices
  for (int i = 0; i < DEVICES_NUMBER; i++) {
    pinMode(devices[i].pin, OUTPUT);
    digitalWrite(devices[i].pin, HIGH);

    // Add virtual device
    fauxmo.addDevice(devices[i].name);
  }

  // Add virtual groups
  fauxmo.addDevice("switch everything");

  fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state) {
    Serial.printf("[MAIN] Device #%d (%s) state: %s\n", device_id, device_name, state ? "ON" : "OFF");
    switch(device_id){
      case 0:
      case 1:
      case 2:
        digitalWrite(devices[device_id].pin, state);
        break;
      case 3:
        for (int i = 0; i < DEVICES_NUMBER; i++) {
          digitalWrite(devices[i].pin, state);
        }
        break;
      default:
        Serial.printf("Unhandled device #%d\n", device_id);
        break;
    }
  });

}

void loop() {
  fauxmo.handle();
}
