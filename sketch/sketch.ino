#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "fauxmoESP.h"
#include "credentials.h"

#define SERIAL_BAUDRATE                 115200
#define LED_RED                         D1
#define LED_GREEN                       D3
#define LED_BLUE                        D4

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

    // LED
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_BLUE, HIGH);

    // You can enable or disable the library at any moment
    // Disabling it will prevent the devices from being discovered and switched
    fauxmo.enable(true);

    // Add virtual devices
    fauxmo.addDevice("switch kitchen");
    fauxmo.addDevice("switch livingroom");
    fauxmo.addDevice("switch bedroom");

    // Group virtual devices
    fauxmo.addDevice("switch all");

    // fauxmoESP 2.0.0 has changed the callback signature to add the device_id, this WARRANTY
    // it's easier to match devices to action without having to compare strings.
    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state) {
        Serial.printf("[MAIN] Device #%d (%s) state: %s\n", device_id, device_name, state ? "ON" : "OFF");
        switch(device_id){
          case 0:
            digitalWrite(LED_RED, state);
            stateAll = state;
            break;
          case 1:
            digitalWrite(LED_GREEN, state);
            stateAll = state;
            break;
          case 2:
            digitalWrite(LED_BLUE, state);
            stateAll = state;
            break;
          case 3:
            digitalWrite(LED_RED, state);
            digitalWrite(LED_GREEN, state);
            digitalWrite(LED_BLUE, state);
            stateAll = !state;
            break;
          default:
            Serial.printf("Unhandled device #%d\n", device_id);
            break;
        }
    });

    // Callback to retrieve current state (for GetBinaryState queries)
    fauxmo.onGetState([](unsigned char device_id, const char * device_name) {
        switch(device_id){
          case 0:
            return digitalRead(LED_RED) == HIGH;
            break;
          case 1:
            return digitalRead(LED_GREEN) == HIGH;
            break;
          case 2:
            return digitalRead(LED_BLUE) == HIGH;
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

    // Since fauxmoESP 2.0 the library uses the "compatibility" mode by
    // default, this means that it uses WiFiUdp class instead of AsyncUDP.
    // The later requires the Arduino Core for ESP8266 staging version
    // whilst the former works fine with current stable 2.3.0 version.
    // But, since it's not "async" anymore we have to manually poll for UDP
    // packets
    fauxmo.handle();

    static unsigned long last = millis();
    if (millis() - last > 2000) {
        last = millis();
        Serial.printf("[MAIN] Free heap: %d bytes\n", ESP.getFreeHeap());
    }

}

