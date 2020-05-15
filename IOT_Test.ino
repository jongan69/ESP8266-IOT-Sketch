
#include <Arduino.h>
#include <ESP8266WiFi.h>
#define HighFan 16
#define FanOff 4
#define FanLight 2
#include "fauxmoESP.h"
#define SERIAL_BAUDRATE 115200

#define WIFI_SSID "Your Network SSID"
#define WIFI_PASS "Your Network Password"

#define FAN "Fan"
#define FAN_LAMP "Fan Lamp"

// Variables will change :
int ledState = HIGH;             // ledState used to set the LED
// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated
// constants won't change :
const long interval = 5000;           // interval at which to blink (milliseconds)


fauxmoESP fauxmo;

// Wi-Fi Connection
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

  // Wi-Fi connection
  wifiSetup();

  //Fan Speeds and Modes
  pinMode(HighFan, OUTPUT);
  digitalWrite(HighFan, HIGH);

  pinMode(FanOff, OUTPUT);
  digitalWrite(FanOff, HIGH);

  //Fan Light
  pinMode(FanLight, OUTPUT);
  digitalWrite(FanLight, HIGH);


  // By default, fauxmoESP creates it's own webserver on the defined port
  // The TCP port must be 80 for gen3 devices (default is 1901)
  // This has to be done before the call to enable()

  fauxmo.createServer(true); // not needed, this is the default value
  fauxmo.setPort(80); // This is required for gen3 devices

  // You have to call enable(true) once you have a WiFi connection
  // You can enable or disable the library at any moment
  // Disabling it will prevent the devices from being discovered and switched

  fauxmo.enable(true);

  // You can use different ways to invoke alexa to modify the devices state:
  // "Alexa, turn lamp two on"

  // Add virtual devices
  fauxmo.addDevice(FAN);
  fauxmo.addDevice(FAN_LAMP);

  fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
    // Callback when a command from Alexa is received.
    // You can use device_id or device_name to choose the element to perform an action onto (relay, LED,...)
    // State is a boolean (ON/OFF) and value a number from 0 to 255 (if you say "set kitchen light to 50%" you will receive a 128 here).
    // Just remember not to delay too much here, this is a callback, exit as soon as possible.
    // If you have to do something more involved here set a flag and process it in your main loop.

    Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
    if ( (strcmp(device_name, FAN) == 0) ) {
      // this just sets a variable that the main loop() does something about
      Serial.println("Fan switched by Alexa");
      //digitalWrite(HighFan, !digitalRead(RELAY_PIN_1));
      if (state) {
        digitalWrite(FanOff, HIGH);
        digitalWrite(FanLight, HIGH);
        digitalWrite(HighFan, LOW);
      } else {
        digitalWrite(HighFan, HIGH);
        digitalWrite(FanLight, HIGH);
        digitalWrite(FanOff, LOW);
      }
    }    if ( (strcmp(device_name, FAN_LAMP) == 0) ) {
      // this just sets a variable that the main loop() does something about
      Serial.println("Fan Lamp switched by Alexa");
      if (state) {
        digitalWrite(HighFan, HIGH);
        digitalWrite(FanOff, HIGH);
        digitalWrite(FanLight, LOW);
      } else {
        digitalWrite(HighFan, HIGH);
        digitalWrite(FanOff, HIGH);
        digitalWrite(FanLight, LOW);
      }
    }
  });

}

void loop() {
  // fauxmoESP uses an async TCP server but a sync UDP server
  // Therefore, we have to manually poll for UDP packets
  fauxmo.handle();

  static unsigned long last = millis();
  if (millis() - last > 5000) {
    last = millis();
    Serial.printf("[MAIN] Free heap: %d bytes\n", ESP.getFreeHeap());
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the Fan
    previousMillis = currentMillis;
    // set the Fan Light with the ledState of the variable:
    digitalWrite(FanLight, ledState);
  }
}
