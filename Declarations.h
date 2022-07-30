/* 
 *  Here you can set all variable depending on your requirement and configuration
 */

//////////////////////////////////// json ////////////////////////////////////
#include <ArduinoJson.h>

//////////////////////////////////// wifi ////////////////////////////////////
#include <ESP8266WiFi.h>


WiFiMode_t wifiMode = WIFI_STA;                 // or WIFI_AP
const char* sta_ssid = "***********";           // KEEP SECRET !
const char* sta_password = "***********";       // KEEP SECRET !
const char* sta_hostname = "AlarmEsp";

const char* ap_ssid = "AP_" + ESP.getChipId();
const char* ap_password = "ESP_0123456789";     // KEEP SECRET !

uint32_t connectionTimeoutWifiSTA = 10000;      // WiFi connect timeout per STA in ms. Increase when connecting takes longer.  = WIFI_CONNECT_TIMEOUT_MS 5000

//////////////////////////////////// UDP ////////////////////////////////////
#include <ESPAsyncUDP.h>
IPAddress gatewayIP(192,168,1,10);              // TO CUSTOM
unsigned int gatewayPort = 9898;                // port to send on

IPAddress multicastIP(224,0,0,50);
unsigned int multicastPort = 9898;              // port to listen on

// read only
AsyncUDP udpMulticast;
AsyncUDP udpGateway;
String gateway_sid = "";
String gateway_last_token = "";

//////////////////////////////////// Xiaomi gateway ////////////////////////////////////
#include <stdlib.h>
#include "XiaomiGatewayUtils.h"

// This key is the same as gateway key which is setted in MI-Home App by user.
String gateway_password = "**************";     // KEEP SECRET !


bool toggleGatewayLight = true;
bool     gateway_lightOn = true;
uint32_t gateway_lightToggleDelay = 5000;
String   gateway_lightColor = "0100ff00";       // aarrggbb


bool useGatewayAlarm = true;
bool     gateway_isRinging = false;
bool     gateway_alarmRing1time = true;
uint32_t gateway_alarmDuration = 5000;          // 5s
uint32_t gateway_alarmLightToggleDelay = 500;
Sound    gateway_alarmSound = Sound::Bark;
int      gateway_alarmSoundVol = 5;             // in percentage
String   gateway_alarmLightColor = "ffff0000";  // aarrggbb

// read only
uint32_t gateway_lightTogglePreviousTime = 0;
uint32_t gateway_alarmPreviousTime = 0;
