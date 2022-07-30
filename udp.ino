
void initUdp(){
  if(udpMulticast.listenMulticast(multicastIP, multicastPort)){
    Serial.printf("UDP listen on multicast %s:%d\n", multicastIP.toString().c_str(), multicastPort);
    
    udpMulticast.onPacket(onUdpPacketEvent);
  }
  
  
  if(udpGateway.connect(gatewayIP, gatewayPort)) {
    Serial.printf("UDP connected with %s:%d\n", gatewayIP.toString().c_str(), gatewayPort);
    
    udpGateway.onPacket(onUdpPacketEvent);
  }
}

void onUdpPacketEvent(AsyncUDPPacket packet) {
  Serial.print(F("UDP Packet Type: "));
  Serial.print(packet.isBroadcast()?F("Broadcast"):packet.isMulticast()?F("Multicast"):F("Unicast"));
  Serial.print(F(", From: "));
  Serial.print(packet.remoteIP());
  Serial.print(F(":"));
  Serial.print(packet.remotePort());
  Serial.print(F(", To: "));
  Serial.print(packet.localIP());
  Serial.print(F(":"));
  Serial.print(packet.localPort());
  Serial.print(F(", Length: "));
  Serial.print(packet.length());
  Serial.print(F(", Data: "));
  Serial.write(packet.data(), packet.length());
  Serial.println();

  
  analysePacketData(String((char*)packet.data()));
}

void analysePacketData(String packetData){
  DynamicJsonDocument doc1(256);
  JsonObject root = deserializeJson(&doc1, packetData);
  if(root[F("cmd")])
  {
    if(root[F("cmd")].as<String>() == F("heartbeat"))
    {
      if(root[F("data")]){
        DynamicJsonDocument doc2(256);
        JsonObject data = deserializeJson(&doc2, root[F("data")].as<String>());
        // compare gateway ip send in data
        if(data[F("ip")] && data[F("ip")].as<String>() == gatewayIP.toString()){
          // if our gateway, save sid and token
          if(root[F("sid")]){
            bool newSid = gateway_sid != root[F("sid")].as<String>();
            gateway_sid = root[F("sid")].as<String>();
            if(newSid){
              // request all children sid connected to this gateway
              request_sids(gateway_sid);
            }
          }
          if(root[F("token")]){
            gateway_last_token = root[F("token")].as<String>();
          }
        }
      }
    }
    else if(root[F("cmd")].as<String>() == F("get_id_list_ack"))
    {
      if(root[F("sid")] && root[F("sid")].as<String>() == gateway_sid){
        if(root[F("token")]){
          gateway_last_token = root[F("token")].as<String>();
        }
      }
      
      // TODO
      /*DynamicJsonDocument doc2(256);
      JsonObject data = deserializeJson(&doc2, root[F("data")].as<String>());
      // display list of sid connected to this gateway */
    }
    else if(root[F("cmd")].as<String>() == F("write_ack"))
    {
      // TODO
    }
    else if(root[F("cmd")].as<String>() == F("read_ack"))
    {
      // TODO
    }
  }
}

JsonObject deserializeJson(DynamicJsonDocument *doc, String msg){

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(*doc, msg);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
  }

  return doc->as<JsonObject>();
}

/* execute action from a json as this one :
 * {
 *  "password":"16charLengthPswd",
 *  "model":"gateway",
 *  "sid":"0123456789ab",
 *  "short_id":0,
 *  "switch":"rgb",
 *  "request":0,
 *  "vol":50
 * }
*/
bool execute_action(DynamicJsonDocument data, String lastToken){
  
  String write_key = get_gateway_key_encrypted(lastToken);
  if(write_key.length() != sizeof(key_of_write)-1){ //32
    Serial.println("encrypted token has not the right length : " + String(write_key.length()) + " need : " + String(sizeof(key_of_write)-1));
    return false;
  }
  
  
  DynamicJsonDocument commandData(256);
  if (data[F("switch")].as<String>() == F("mid")){
    commandData[data[F("switch")].as<String>()] = data[F("request")].as<int>();
    commandData[F("vol")] = data[F("vol")].as<int>();
  }else{
    if (data[F("request")].is<int>()){
      commandData[data[F("switch")].as<String>()] = data[F("request")].as<int>();
    }else if (data[F("request")].is<unsigned long>()){
      commandData[data[F("switch")].as<String>()] = data[F("request")].as<unsigned long>();
    }else{
      commandData[data[F("switch")].as<String>()] = data[F("request")].as<String>();
    }
  }
  commandData[F("key")] = write_key;
  String commandDataSerialized;
  serializeJson(commandData, commandDataSerialized);
  
  DynamicJsonDocument commandWrite(256);
  commandWrite[F("cmd")] =      F("write");
  commandWrite[F("model")] =    data[F("model")].as<String>();
  commandWrite[F("sid")] =      data[F("sid")].as<String>();
  commandWrite[F("short_id")] = data[F("short_id")].as<int>();
  commandWrite[F("data")] =     commandDataSerialized;// data value must be a string with command and encrypted key
  String commandWriteSerialized;
  serializeJson(commandWrite, commandWriteSerialized);
  
  sendJson(commandWriteSerialized);
  return true;
}

// Request system ids (sid) from the gateway. Return also all sid of connected devices.
void request_sids(String sid){
  if(sid != F("")){
    String write_command = "{\"cmd\":\"get_id_list\",\"sid\":\"" + sid + "\"}";
    sendJson(write_command);
  }
}

// Request (read) the current status of the given device sid.
void request_current_status(String device_sid){
  if(device_sid != F("")){
    String write_command = "{\"cmd\":\"read\",\"sid\":\"" + device_sid + "\"}";
    sendJson(write_command);
  }
}

// Request to change color of the ring gateway. Color must be in aarrggbb format.
bool changeColor(String sid, String colorRGBR){
  if(sid == F("")) return false;
  
  DynamicJsonDocument data(256);
  data[F("model")] =    F("gateway");
  data[F("sid")] =      sid;
  data[F("short_id")] = 0;
  data[F("switch")] =   F("rgb");
  data[F("request")] =  hexToDec(colorRGBR);

  return execute_action(data, gateway_last_token);
}

// Request to change sound of the gateway with a custom volume (in percentage).
bool changeSoundAlarm(String sid, Sound sound, int vol){
  if(sid == F("")) return false;

  if (vol < 0 || vol > 100){
    Serial.println(F("Volume must be in range 0-100"));
    return false;
  }
  
  DynamicJsonDocument data(256);
  data[F("model")] =    F("gateway");
  data[F("sid")] =      sid;
  data[F("short_id")] = 0;
  data[F("switch")] =   F("mid");
  data[F("request")] =  (int)sound;
  data[F("vol")] =      vol;

  
  return execute_action(data, gateway_last_token);
}

void sendJson(String sendMsg){
  Serial.println("Send to " + gatewayIP.toString() + ": " + sendMsg);
  udpGateway.print(sendMsg);
}

// convert hexadecimal value to decimal
unsigned long hexToDec(String hexString) {
  
  unsigned long decValue = 0;
  int nextInt;
  
  for (uint8_t i = 0; i < hexString.length(); i++) {
    
    nextInt = int(hexString.charAt(i));
    if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);
    
    decValue = (decValue * 16) + nextInt;
  }
  
  return decValue;
}
