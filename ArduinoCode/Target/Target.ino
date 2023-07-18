/**
 * Infinitag Target
 * 
 * An ESP32 S3 based lasertag target with
 * RGBW LEDs and 5V + 3,3V + relay switches
 * 
 * @author Tobias Stewen
 * @version 3.0.0
 * @license CC BY-NC-SA 4.0
 * @url https://github.com/Infinitag/Target
 */

// Includes
#include <arduino-timer.h>
#include <Adafruit_NeoPixel.h>
#include <HTTPClient.h>
#include <Infinitag_Core.h>
#include <IRremote.hpp>
#include <WiFiManager.h>
#include <Preferences.h>

// Timer SetUp
auto timer = timer_create_default();

// Infinitag
Infinitag_Core infinitagCore;

// Wifi Manager
WiFiManager wm;
int wifiConfigurationPortalTimeout = 60;
char wifiName[32] = "InfinitagTarget";
char wifiPassword[32] = "YourDefaultPassword";
WiFiManagerParameter custom_sound_id("sound_id", "Sound ID", "", 3);
WiFiManagerParameter custom_hit_time("hit_time", "Hit Time", "", 10);

// HTTP Client
HTTPClient http;
unsigned long freeUpResourcesTime = 300000;
bool sendHttpReq = false;
int ipBlock;

// LEDs
#define LED_PIN     38
#define LED_COUNT   12
#define BRIGHTNESS  255
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

// Infrared
#define IR_PIN 15

// Switches
#define SW_PIN      21
#define SW_5V_PIN   46
#define SW_33V_PIN  48

// Config
Preferences preferences;

// Sound
int soundId = 0;
int hitTime = 0;
int defaultHitTime = 10000;

// Animation
unsigned long animationPreviousMillis = 0;
unsigned long animationInterval = 10;
long animationStep = 0;

// General
bool hit = false;
unsigned long timeOfHit = 0;

// Target Setup
void setup() {
  // Serial
  Serial.begin(115200);

  loadConfig();

  setupWifi();
  setupHttpClient();
  setupSwitches();
  setupIr();
  setupLeds();
  setupAnimationLoop();
}

// Main loop on main core
void loop() {
  while (true){
    // Check for IR signal
    checkIrData();
  }
}

// Animation loop on separated core
void loopAnimation (void* pvParameters) {
  // Initial enable the target
  enableTarget();

  //Continuously free up resources from Http cliet
  timer.every(freeUpResourcesTime, httpFreeResourceTimer);

  while (true){
    // Timer loop function
    timer.tick();

    // Check if next animation step is reached
    if ((unsigned long)(millis() - animationPreviousMillis) >= animationInterval) {
      // Set curret millis as previous step time
      animationPreviousMillis = millis();

      // Check which animation must be played
      if (hit) {
        // Hit animation
        animationHit();
      } else {
        // Idle animation
        rainbow();
      }
    }
  }
}

// Enable the Target for hits
void enableTarget() {
  // Deactivate switches
  digitalWrite(SW_PIN, LOW);
  digitalWrite(SW_5V_PIN, LOW);
  digitalWrite(SW_33V_PIN, LOW);

  // Reset the animation with 10 millis delay for rainbow animation
  resetAnimation(10);

  // Reset hit var
  hit = false;
}

// Change target to hit state
void targetHit() {
  // Set hit var
  hit = true;

  // Set time of hit
  timeOfHit = millis();

  // Enable switches
  digitalWrite(SW_PIN, HIGH);
  digitalWrite(SW_5V_PIN, HIGH);
  digitalWrite(SW_33V_PIN, HIGH);

  // Reset the animation with 80 millis delay for hit animation
  resetAnimation(80);
}

// Load previous saved parameters
void loadConfig() {
  // Open preferences
  preferences.begin("target-data", true);

  // Load sound id
  soundId = preferences.getUInt("soundid", 1);
  
  // Load hit time
  hitTime = preferences.getUInt("hittime", defaultHitTime);

  // Close preferences
  preferences.end();
}

// Check if IR was received
void checkIrData() {
  // IR Reveiced? Yes, then decode the signal
  if (IrReceiver.decode()) {
    // If hit = true > No action
    if (!hit) {
      // Decode the infinitag protocol
      if (infinitagCore.irDecode(IrReceiver.decodedIRData.decodedRawData)) {
        // No System + Command 1 = Hit
        if (infinitagCore.irRecvIsSystem == false && infinitagCore.irRecvCmd == 1) {
          hitAction();

        // System + Command 1 = Open Config Menu
        } else if(infinitagCore.irRecvIsSystem == true && infinitagCore.irRecvCmd == 1) {
          openConfigPortalAction();
        }
      }
    }

    // Enable receiving of the next value
    IrReceiver.resume(); 
  }
}

// Action to activate config portal
void openConfigPortalAction() {
  wm.setConfigPortalTimeout(wifiConfigurationPortalTimeout);
  wm.startConfigPortal(wifiName, wifiPassword);
}

// Action for a hit
void hitAction() {
  // Set Hit
  targetHit();

  // Set timer to reactivate target after hit time
  timer.in(hitTime, enableTargetTimer);

  // Extract IP block from infinitag signal and
  // call origins api for a sound effect
  // with the sound id of this target
  ipBlock = infinitagCore.irRecvCmdValue;
  String serverPath = "http://192.168.1.";
  serverPath.concat(ipBlock);
  serverPath.concat(":8080/trigger_effect?sound=");
  serverPath.concat(soundId);
  Serial.println(serverPath);
  http.begin(serverPath.c_str());
  int httpResponseCode = http.GET();

  // Output error only for debug
  if (!httpResponseCode>0) {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
}

// Timer function for reactivating the target
bool enableTargetTimer(void *) {
  enableTarget();

  return true;
}

// Function to free uo Http client ressources
bool httpFreeResourceTimer(void *) {
  // Free resources
  http.end();

  return true;
}

// Setup for Wifi
void setupWifi() {
  // Add Customparameter and set default value
  wm.addParameter(&custom_sound_id);

  // Custom-Param: Sound ID
  char tmpCharSoundId[3];
  itoa(soundId, tmpCharSoundId, 10);
  custom_sound_id.setValue(tmpCharSoundId, 3);

  // Custom-Param: Hit time
  wm.addParameter(&custom_hit_time);
  char tmpCharHitTime[10];
  itoa(hitTime, tmpCharHitTime, 10);
  custom_hit_time.setValue(tmpCharHitTime, 10);

  // Custom menu
  // Added param for custom configuration
  // Added exit to start loop after configuration
  std::vector<const char *> menu = {"wifi","info","param","close","sep","update","exit"};
  wm.setMenu(menu);

  // Setup Callback to save custom params
  wm.setSaveParamsCallback(saveWifiParamsCallback);

  // Init Wifi
  bool res;
  res = wm.autoConnect(wifiName, wifiPassword);
  if(!res) {
    ESP.restart();
  }
}

// Setup Http Client
void setupHttpClient() {
  // Set Reuse to reduce latancy 
  http.setReuse(true);
}

// Setup Switches
void setupSwitches() {
  pinMode(SW_PIN, OUTPUT);
  pinMode(SW_5V_PIN, OUTPUT);
  pinMode(SW_33V_PIN, OUTPUT);
}

// Setup Infrared
void setupIr() {
  IrReceiver.begin(IR_PIN, false);
}

// Setup LEDs
void setupLeds() {
  strip.begin();
  strip.show();
  strip.setBrightness(BRIGHTNESS);
}

// Setup animation loop
void setupAnimationLoop() {
  xTaskCreatePinnedToCore (
  loopAnimation,      // Function to implement the task
    "loopAnimation",  // Name of the task
    10000,            // Stack size in bytes
    NULL,             // Task input parameter
    1,                // Priority of the task
    NULL,             // Task handle.
    1                 // Core where the task should run
  );
}

// Callback for Wifi to save custom params
void saveWifiParamsCallback () {
  // Open the storage
  preferences.begin("target-data", false);

  // Get, convert and save: Sound ID
  int newSoundId = atoi(custom_sound_id.getValue());
  soundId = newSoundId;
  preferences.putUInt("soundid", soundId);

  // Get, convert and save: Hit time
  int newHitTime = atoi(custom_hit_time.getValue());
  hitTime = newHitTime;
  preferences.putUInt("hittime", hitTime);

  // Close storage
  preferences.end();

  // Close Config portal
  wm.stopConfigPortal();
}

// Helper function to set all LEDs to one color
void setColor(uint32_t color) {
  for(int i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

// Reset the LED animation with a delay intervall
void resetAnimation(int interval) {
  animationPreviousMillis = millis();
  animationStep = 0;
  animationInterval = interval;
}

// LED animation for the rainbow
void rainbow() {
  strip.rainbow(animationStep);
  strip.show();

  animationStep = (animationStep < 65536) ? (animationStep + 256) : 0;
}

// LED animation for a hit
void animationHit() {
  strip.clear();
  int distance;
  long maxBrightness = map(millis(), timeOfHit, (timeOfHit + hitTime), 0, 255);
  
  for(int i=0; i<strip.numPixels(); i++) {
    distance = (i <= animationStep) ? (animationStep - i) : (strip.numPixels() - i + animationStep);
    strip.setPixelColor(i, strip.Color(map(distance, 0, 11, maxBrightness, 0), 0, 0, 0));
  }
  strip.show();

  animationStep = (animationStep < 11) ? (animationStep + 1) : 0;
}
