
#include <Arduino.h>
#include <stdlib.h>

#define LED_BUILTIN 2 // Fix for IRRemote lib warning
#define SEND_PWM_BY_TIMER // Fix for IRRemote lib warning

// Wifi
#include <WiFiManager.h>
WiFiManager wm;
int cpTimeout = 90; // Timeout for the configuration portal
WiFiManagerParameter custom_led_delay("led_delay", "LED delay", "", 10);

// Core
#include <Infinitag_Core.h>
Infinitag_Core infinitagCore;

// WifiManager configuration portal button
#define BTN_CP_START 17
int buttonState = 0;

// IR
#define DECODE_RC5
#include <IRremote.hpp>
#define IR_RECEIVE_PIN 16
IRrecv irrecv(IR_RECEIVE_PIN);
decode_results results;

// LED
#include <Adafruit_NeoPixel.h>
#define LED_PIN     18
#define LED_COUNT  12
#define BRIGHTNESS 255
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

// Switches
#define SW_1     19
#define SW_2     21
#define SW_3     22
#define SW_4     23

// Color Settings
int ledColorWifiReboot[4] = {0, 0, 0, 255};
int ledColorWifiWait[4] = {255, 255, 0, 0};
int ledColorIdle[4] = {255, 0, 255, 0};
int ledColorHit[4] = {255, 0, 0, 0};
int ledColorMissHit[4] = {0, 255, 255, 0};

// General
bool targetHit = false;
#define TEST_LED_PIN 2

void setup() {
  // Disable Log to prevent Serial Log
  wm.setDebugOutput(false); 

  // Test blink 
  pinMode(TEST_LED_PIN, OUTPUT);
  digitalWrite(TEST_LED_PIN, HIGH);
  delay(1000);
  digitalWrite(TEST_LED_PIN, LOW);
  
  // Wifi - 1
  WiFi.mode(WIFI_STA);

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
  wm.setHostname("InfinitagTarget_1");
  
  // LED
  strip.begin();
  strip.show();
  strip.setBrightness(BRIGHTNESS);
  ledShowSetup();

  // Wifi - 2
  //wm.resetSettings(); // Only for Develop - Remove after
  // Auto Connect
  if (!wm.autoConnect("InfinitagTarget_1","12345678")) {
  }
    
  // IR
  irrecv.enableIRIn();

  // General 
  targetHit = false;
}

void loop() {
  irDecode();

  configPortalButton();
  
  if (targetHit) {
    targetHitLoop();
  } else {
    targetNotHitLoop();
  }
}

void targetNotHitLoop() {
  setAllSwitches(LOW);
  ledFillFull(ledColorIdle);
}

void targetHitLoop() {
  setAllSwitches(HIGH);
  
  ledFillFull(ledColorHit);
  
  targetHit = false;
  delay(1000);
  IrReceiver.resume();
}

void setAllSwitches(int state) {
  digitalWrite(SW_1, state);
  digitalWrite(SW_2, state);
  digitalWrite(SW_3, state);
  digitalWrite(SW_4, state);
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
  // IR signal received?
  if (IrReceiver.decode()) {
    
    // Is the IR signal a Infinitag signal?
    if (infinitagCore.irDecode(IrReceiver.decodedIRData.decodedRawData)) {
      
      // Validate all required Infinitag parameters for a valid hit
      if (infinitagCore.irRecvIsSystem == false && infinitagCore.irRecvGameId == 0
        && infinitagCore.irRecvTeamId == 0 && infinitagCore.irRecvPlayerId == 0
        && infinitagCore.irRecvCmd == 1 && infinitagCore.irRecvCmdValue == 255) {
          targetHit = true;
      }
    } else {
      ledFillFull(ledColorMissHit);
      delay(150);
      strip.clear();
    }
    
    IrReceiver.resume();
  }
}

void ledShowSetup() {
  ledFillFull(ledColorWifiReboot);
  delay(150);
  ledFillFull(ledColorWifiWait);
}

void ledFillFull(int color[]) {
  strip.fill(strip.Color(color[0], color[1], color[2], color[3]));
  strip.show();
}
