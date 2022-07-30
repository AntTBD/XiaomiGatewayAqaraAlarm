/*
 * Active an Alarm on Xiaomi Gateway
 * 
 * Code for ESP8266 that :
 * - Connect to a wifi or create an Access Point
 * - Connect to a Xiaomi Gateway (with IP only)
 * - Read all messages from this Xiaomi Gateway
 * - Send order to change ring light color and intensity of the Xiaomi Gateway
 * - Send order to change sound and volume of the Xiaomi Gateway
 *   (Send order need to encrypt the last token from heartbeat - with AES-128-CBC encryption)
 * 
 * - Active an alarm with blinking light at the first time with receive a heartbeat from our Xiaomi Gateway
 * - Active a blinking otherwise
 * 
 * Developped by https://github.com/anttbd
 * 
 * Encryptage code from official Xiaomi Gateway API https://forrestfung.gitbooks.io/lumi-gateway-local-api/content/
 * 
 * Tested configuration:
 * - Board :                  Node MCU ESP8266 / WEMOS D1 mini
 * - Upload speed :           921600
 * - CPU Frequency :          80 Hz
 * - Flash Size :             4MB (FS:2MB, OTA:~1019KB)
 * - SSL support :            All SSL cipher (most compatible)
 * - Xiaomi Gateway version : 1.1.2
 * 
 * TODO:
 * - Alarm ring when input gpio change state
 * - More decode ack messages from Gateway and its children devices connected in zigbee
 * - Possible web server to display stats and manage alarm (optional)
 * 
 */

#include "Declarations.h"

void setup() {
  // Start serial at 115200 bauds
  Serial.begin(115200);
  
  // Waiting for the serial port to open
  for (uint8_t t = 5; t > 0; t--) {
      Serial.print(F("BOOT WAIT "));
      Serial.print(t);
      Serial.println(F("..."));
      Serial.flush();
      delay(1000);
  }

  initWifi(wifiMode, true);
  initGatewayEncryptage(gateway_password);

  initUdp();
}

void loop() {

  if(toggleGatewayLight && gateway_isRinging == false){
    colorBlink(gateway_sid, gateway_lightToggleDelay, gateway_lightColor);
  }

  if(useGatewayAlarm){
    alarmRing(gateway_sid, gateway_alarmDuration, gateway_alarmSound, gateway_alarmSoundVol);
  }
}

// toggle color on with custom color in format aarrggbb and off after a delay
void colorBlink(String sid, uint32_t delay, String color){//aarrggbb
  if((millis() - gateway_lightTogglePreviousTime) > delay)
  {
    gateway_lightTogglePreviousTime = millis();
    if(gateway_lightOn){
      gateway_lightOn = false;
      changeColor(sid, F("0"));
    }else{
      gateway_lightOn = true;
      changeColor(sid, color);//aarrggbb
    }
  }
}

// ring alarm with custom sound and volume during spécific time, then stop for the same time and if ring 1 time, stop repetitions
void alarmRing(String sid, uint32_t duration, Sound sound, int vol){
  if(gateway_isRinging == false && (millis() - gateway_alarmPreviousTime) > duration){
    if(changeSoundAlarm(sid, sound, vol)){
      gateway_isRinging = true;
      gateway_alarmPreviousTime = millis();
    }
  }
  if(gateway_isRinging){
    colorBlink(sid, gateway_alarmLightToggleDelay, gateway_alarmLightColor);
  }
  if(gateway_isRinging && (millis() - gateway_alarmPreviousTime) > duration)
  {
    // reset values
    gateway_isRinging = false;
    gateway_alarmPreviousTime = millis();
    changeSoundAlarm(sid, Sound::Off, 0);
    gateway_lightOn = false;
    changeColor(sid, F("0"));
    
    if(gateway_alarmRing1time){
      useGatewayAlarm = false;
    }
  }
}
