void initWifi(WiFiMode_t wifiMode, bool onSTAFailedStartAP = false);
bool startAP(String ap_ssid, String ap_password = "", bool ap_isHide = false);
bool connectSTA(String sta_ssid, String sta_password = "", String sta_hostname = "");

void initWifi(WiFiMode_t wifiMode, bool onSTAFailedStartAP){
  switch(wifiMode){
    case WIFI_STA:
      {
        bool success = connectSTA(sta_ssid,sta_password, sta_hostname);
        if(success == false && onSTAFailedStartAP == true)
        {
          Serial.println(F("=> Try to create access point..."));
          startAP(ap_ssid, ap_password);
        }
        else if(success == false && onSTAFailedStartAP == false)
        {
          Serial.println(F("=> Wait and Reboot"));
          delay(5000);
          Serial.println(F("Restart..."));
        }
        break;
      }
    case WIFI_AP:
      {
        startAP(ap_ssid,ap_password);
        break;
      }
    default:
      {
        //WiFi.mode(WIFI_AP_STA);
        Serial.println("Wifi Mode : "+ String(wifiMode) + " is not supported here, sorry !");
        break;
      }
  }
  
  Serial.println("My IP = " + (WiFi.getMode() != WIFI_AP ? WiFi.localIP().toString() : WiFi.softAPIP().toString()));
}

bool startAP(String ap_ssid, String ap_password, bool ap_isHide){
  WiFi.persistent(false); //  Disable the WiFi persistence to avoid any re-configuration that may erase static lease when starting softAP / STA
  WiFi.enableAP(true);
  WiFi.mode(WIFI_AP); // Set WiFi to station mode
  delay(500);
  
  Serial.printf("Creating Access Point named %s ...", ap_ssid.c_str());
  
  bool success = WiFi.softAP(
    ap_ssid.c_str(),
    (ap_password == F("") ? NULL : ap_password.c_str()),
    (ap_isHide ? 3 : 1), // 3rd param = channel : 3 when hide else 1 (default)
    (ap_isHide ? 1 : 0)  // 4th param = hide or not (default)
    );
    
  Serial.println(success ? F("Success") : F("Failed"));

  if (success == false) {
    Serial.println(F("Restart ESP..."));
    ESP.restart();
  }
  
  return success;
}

bool connectSTA(String sta_ssid, String sta_password, String sta_hostname){
  
  WiFi.persistent(false); //  Disable the WiFi persistence to avoid any re-configuration that may erase static lease when starting softAP / STA
  WiFi.enableSTA(true);
  WiFi.mode(WIFI_STA); // Set WiFi to station mode

  // set hostnanme : name which is displayed on router
  if (sta_hostname != F(""))
      WiFi.hostname(sta_hostname);

  
  Serial.printf("Connecting to %s ", sta_ssid.c_str());
  WiFi.begin(sta_ssid, sta_password);
  
  auto startTime = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - startTime) <= connectionTimeoutWifiSTA)
  {
    delay(500);
    Serial.print(F("."));
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("connected"));
    return true;
  }else{
    Serial.println(F("\nConnection failed !"));
    startAP(ap_ssid,ap_password);
    return false;
  }
}
