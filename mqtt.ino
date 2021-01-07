//
// mqtt
//

// ************************************
// mqttConnect()
//
//
// ************************************
void mqttConnect() {
  uint8_t timeout=10;
  if(strlen(config.broker_host) > 0) {
    DEBUG("[MQTT] Attempting connection to "+String(config.broker_host)+":"+String(config.broker_port));
    while((!mqttClient.connected())&&(timeout > 0)) {
      // Attempt to connect
      if (mqttClient.connect(config.client_id)) {
        // Once connected, publish an announcement...
        DEBUG("[MQTT] Connected as "+String(config.client_id));
      } else {
        timeout--;
        delay(500);
      }
    }
    if(!mqttClient.connected()) {
      DEBUG("[MQTT] Connection failed");    
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  DEBUG("Message arrived: "+String(topic));
}

// ************************************
// mqttRunnerCB()
//
// 
// ************************************
void mqttRunnerCB() {
  char topic[32];
    
  // MQTT not connected? Connect!
  if (!mqttClient.connected()) {
    mqttConnect();
  }

  // Now prepare MQTT message...
  JsonObject root = env.as<JsonObject>();
 
  for (JsonPair keyValue : root) {
    sprintf(topic,"%s/%s",config.client_id,keyValue.key().c_str());
    String value = keyValue.value().as<String>();
    if(mqttClient.publish(topic,value.c_str())) {
      DEBUG("[MQTT] Publish "+String(topic)+":"+value);
    } else {
      DEBUG("[MQTT] Publish failed!");
    }
  }
}
