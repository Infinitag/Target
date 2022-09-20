
#include <Arduino.h>
#include <stdlib.h>

// Wifi
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
WiFiManager wm;
int cpTimeout = 90; // Timeout for the configuration portal
WiFiManagerParameter custom_led_delay("led_delay", "LED delay", "", 10);

// WifiManager configuration portal button
#define BTN_CP_START 15
int buttonState = 0;

// IR
#define DECODE_SAMSUNG
#include <IRremote.hpp>
#define IR_RECEIVE_PIN 16

// LED
#include <Adafruit_NeoPixel.h>
#define LED_PIN     18
#define LED_COUNT  12
#define BRIGHTNESS 255 // Set BRIGHTNESS to about 1/5 (max = 255)
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

// Switches
#define SW_1     19
#define SW_2     21
#define SW_3     22
#define SW_4     23

// Color Settings
int ledColorWifiReboot[4] = {0, 0, 0, 100};
int ledColorWifiWait[4] = {100, 100, 0, 0};
int ledColorIdle[4] = {100, 0, 100, 0};
int ledColorHit[4] = {50, 0, 0, 0};

// General
bool targetHit = false;
#define TEST_LED_PIN 2

void setup() {
  Serial.begin(9600);
  
  pinMode(TEST_LED_PIN, OUTPUT);
  digitalWrite(TEST_LED_PIN, HIGH);
  delay(1000);
  digitalWrite(TEST_LED_PIN, LOW);
  
  // Wifi - 1
  WiFi.mode(WIFI_STA);
  
  //Serial.begin(115200);
  //Serial.println(F("Starting APP"));

  // Set configuration portal PIN
  pinMode(BTN_CP_START, INPUT);

  // Set Switch Pins
  pinMode(SW_1, OUTPUT);
  pinMode(SW_2, OUTPUT);
  pinMode(SW_3, OUTPUT);
  pinMode(SW_4, OUTPUT);

  // Custom configuration
  wm.addParameter(&custom_led_delay);

  // Set the default value for the custom field
  custom_led_delay.setValue("1000",4);

  // Custom menu
  // Added param for custom configuration
  // Added exit to start loop after configuration
  std::vector<const char *> menu = {"wifi","info","param","close","sep","update","exit"}; // Added param to the menu
  wm.setMenu(menu); // custom menu, pass vector
  
  // LED
  strip.begin();
  strip.show();
  strip.setBrightness(BRIGHTNESS);
  ledShowSetup();

  // Wifi - 2
  //wm.resetSettings(); // Only for Develop - Remove after
  // Auto Connect
  if (!wm.autoConnect("WMAutoConnectAP","12345678")) {
    if (Serial) {
      Serial.println("failed to connect and hit timeout");
    }
  }
    
  // IR
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  // General 
  targetHit = false;
}

void loop() {
  irDecode();

  if (IrReceiver.isIdle()) {
    configPortalButton();
    
    if (targetHit) {
      targetHitLoop();
    } else {
      targetNotHitLoop();
    }
  }
}

void targetNotHitLoop() {
  digitalWrite(SW_1, LOW);
  digitalWrite(SW_2, LOW);
  digitalWrite(SW_3, LOW);
  digitalWrite(SW_4, LOW);
  strip.fill(strip.Color(ledColorIdle[0], ledColorIdle[1], ledColorIdle[2], ledColorIdle[3]));
  strip.show();
}

void targetHitLoop() {
  digitalWrite(SW_1, HIGH);
  digitalWrite(SW_2, HIGH);
  digitalWrite(SW_3, HIGH);
  digitalWrite(SW_4, HIGH);
  strip.fill(strip.Color(ledColorHit[0], ledColorHit[1], ledColorHit[2], ledColorHit[3]));
  strip.show();
  targetHit = false;
  delay(1000);
  IrReceiver.resume();
}

void configPortalButton() {
  buttonState = digitalRead(BTN_CP_START);

  if (buttonState == HIGH) {
    ledShowSetup();
    wm.setConfigPortalTimeout(cpTimeout);
    wm.startConfigPortal("WM_ConnectAP","12345678");
  }
}

void irDecode() {
  if (IrReceiver.decode()) {
    if (IrReceiver.decodedIRData.address == 0x707) {
      if (IrReceiver.decodedIRData.command == 0x7) {
        targetHit = true;
      }
    }
    IrReceiver.resume();
  }
}

void ledShowSetup() {
  strip.fill(strip.Color(ledColorWifiReboot[0], ledColorWifiReboot[1], ledColorWifiReboot[2], ledColorWifiReboot[3]));
  strip.show();
  delay(150);
  strip.fill(strip.Color(ledColorWifiWait[0], ledColorWifiWait[1], ledColorWifiWait[2], ledColorWifiWait[3]));
  strip.show();
}
