
#include <Arduino.h>
#include <stdlib.h>

#define LED_BUILTIN 2 // Fix for IRRemote lib warning
#define SEND_PWM_BY_TIMER // Fix for IRRemote lib warning

// Wifi
#include <WiFiManager.h>
WiFiManager wm;
int cpTimeout = 90; // Timeout for the configuration portal
char hit_duration[6] = "7000";
char switch_duration[6] = "1000";
char advertise_time[6] = "60000";
char demo_mode[2] = "1";
char target_name[30] = "InfinitagTarget_4";
WiFiManagerParameter custom_hit_duration("hit_duration", "Hit duration", hit_duration, 6);
WiFiManagerParameter custom_switch_duration("switch_duration", "Switch duration", switch_duration, 6);
WiFiManagerParameter custom_advertise_time("advertise_time", "Advertise time", advertise_time, 6);
WiFiManagerParameter custom_demo_mode("demo_mode", "Demo mode", demo_mode, 2);
WiFiManagerParameter custom_target_name("target_name", "Name", target_name, 30);

// Core
#include <Infinitag_Core.h>
Infinitag_Core infinitagCore;

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
int ledColorDemo[4] = {0, 20, 20, 0};

// General
bool targetHit = false;
int hitDuration = 1000;
float hitStep = 1000;
bool demoMode = false;
unsigned long lastHit;
#define TEST_LED_PIN 2
int advertiseTime = 10000;
int switchDuration = 1000;


//callback notifying us of the need to save config
void saveConfigCallback () {
  // Get saved vars
  loadSavedVars();
}

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

  // Set Switch Pins
  pinMode(SW_1, OUTPUT);
  pinMode(SW_2, OUTPUT);
  pinMode(SW_3, OUTPUT);
  pinMode(SW_4, OUTPUT);

  // Custom configuration
  wm.setSaveConfigCallback(saveConfigCallback);
  wm.addParameter(&custom_hit_duration);
  wm.addParameter(&custom_demo_mode);
  wm.addParameter(&custom_target_name);
  wm.addParameter(&custom_advertise_time);
  wm.addParameter(&custom_switch_duration);

  // Custom menu
  // Added param for custom configuration
  // Added exit to start loop after configuration
  std::vector<const char *> menu = {"wifi","info","param","close","sep","update","exit"}; // Added param to the menu
  wm.setMenu(menu); // custom menu, pass vector
  wm.setHostname(custom_target_name.getValue());
  
  // LED
  strip.begin();
  strip.show();
  strip.setBrightness(BRIGHTNESS);
  ledShowSetup();

  // Wifi - 2
  //wm.resetSettings(); // Only for Develop - Remove after
  // Auto Connect
  if (!wm.autoConnect(custom_target_name.getValue(), "12345678")) {
  }
    
  // IR
  irrecv.enableIRIn();

  // Get saved vars
  loadSavedVars();
  
  // General 
  hitStep = hitDuration / LED_COUNT;
  resetHit();
}

void loop() {
  
  irDecode();
    
  if (targetHit) {
    targetHitLoop();
  } else {
    targetNotHitLoop();
  }
}

void loadSavedVars() {
  hitDuration = atoi(custom_hit_duration.getValue());
  demoMode = (atoi(custom_demo_mode.getValue()) == 1) ? true : false;
  advertiseTime = atoi(custom_advertise_time.getValue());
  switchDuration = atoi(custom_switch_duration.getValue());
}

void resetHit() {
  targetHit = false;
  lastHit = millis();
  IrReceiver.resume();
}

void setHit() {
  targetHit = true;
  lastHit = millis();
  setAllSwitches(HIGH);
  ledFillFull(ledColorHit);
}

void targetNotHitLoop() {
  setAllSwitches(LOW);
  
  if (demoMode) {
    strip.clear();
    strip.setPixelColor(0, strip.Color(ledColorDemo[0], ledColorDemo[1], ledColorDemo[2], ledColorDemo[3]));
    strip.show();

    if ((lastHit + advertiseTime) < millis()) {
      setHit();
    }
  } else {
    ledFillFull(ledColorIdle);
  }
}

void targetHitLoop() {
  int diff = millis() - lastHit;
  if (diff > hitDuration) {
    resetHit();
    return;
  }

  if ((lastHit + switchDuration) < millis()) {
    setAllSwitches(LOW);
  }

  for (int i = 0; i < LED_COUNT; i++) {
    int currentIndex = (diff / hitStep);
    if (i < currentIndex) {
      strip.setPixelColor(i, strip.Color(255, 0, 0, 0));
    } else if (i == currentIndex) {
      int redValue = (diff - (i * hitStep)) * 255 / hitStep;
      strip.setPixelColor(i, strip.Color(redValue, 0, 0, 0));
    } else {
      strip.setPixelColor(i, strip.Color(0, 0, 0, 0));
    }
  }
  strip.show();
}

void setAllSwitches(int state) {
  digitalWrite(SW_1, state);
  digitalWrite(SW_2, state);
  digitalWrite(SW_3, state);
  digitalWrite(SW_4, state);
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
        if (!targetHit) {
          setHit();
        }
      } else if (infinitagCore.irRecvIsSystem == false && infinitagCore.irRecvGameId == 0
      && infinitagCore.irRecvTeamId == 0 && infinitagCore.irRecvPlayerId == 0
      && infinitagCore.irRecvCmd == 2 && infinitagCore.irRecvCmdValue == 255){
        ledShowSetup();
        wm.setConfigPortalTimeout(cpTimeout);
        wm.startConfigPortal(custom_target_name.getValue(), "12345678");
      }
    } else {
      if (!targetHit) {
        ledFillFull(ledColorMissHit);
        delay(150);
        strip.clear();
      }
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
