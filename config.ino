// Written by Michele <o-zone@zerozone.it> Pinassi
// Released under GPLv3 - No any warranty

#include <ArduinoJson.h>

// ************************************
// Config, save and load functions
//
// save and load configuration from config file in SPIFFS. JSON format (need ArduinoJson library)
// ************************************
bool loadConfigFile() {
  DynamicJsonDocument root(256);
  
  DEBUG("[CONFIG] Load config...");
  
  configFile = SPIFFS.open(CONFIG_FILE, "r");
  if (!configFile) {
    DEBUG("[CONFIG] Config file not available");
    return false;
  } else {
    // Get the root object in the document
    DeserializationError err = deserializeJson(root, configFile);
    if (err) {
      DEBUG("[CONFIG] Failed to read config file:"+String(err.c_str()));
      return false;
    } else {
      // WIFI
      strlcpy(config.wifi_essid, root["wifi_essid"], sizeof(config.wifi_essid));
      strlcpy(config.wifi_password, root["wifi_password"], sizeof(config.wifi_password));
      strlcpy(config.hostname, root["hostname"] | "wifi-switch", sizeof(config.hostname));
      // NTP
      strlcpy(config.ntp_server, root["ntp_server"] | "time.ien.it", sizeof(config.ntp_server));
      config.ntp_timezone = root["ntp_timezone"] | 1;
      // MQTT broker
      strlcpy(config.broker_host, root["broker_host"] | "", sizeof(config.broker_host));
      config.broker_port = root["broker_port"] | 1883;
      strlcpy(config.client_id, root["client_id"] | "wifi-switch", sizeof(config.client_id));
      // OTA
      config.ota_enable = root["ota_enable"] | true; // OTA updates enabled by default
      DEBUG("[CONFIG] Configuration loaded");
    }
  }
  configFile.close();
  return true;
}

bool saveConfigFile() {
  DynamicJsonDocument root(256);
  
  DEBUG("[CONFIG] Save config...");
  // WIFI
  root["wifi_essid"] = config.wifi_essid;
  root["wifi_password"] = config.wifi_password;
  root["hostname"] = config.hostname;
  // NTP
  root["ntp_server"] = config.ntp_server;
  root["ntp_timezone"] = config.ntp_timezone;
  // MQTT
  root["broker_host"] = config.broker_host;
  root["broker_port"] = config.broker_port;
  root["client_id"] = config.client_id;
  // OTA
  root["ota_enable"] = config.ota_enable;

  configFile = SPIFFS.open(CONFIG_FILE, "w");
  if(!configFile) {
    DEBUG("[CONFIG] Failed to create config file !");
    return false;
  }
  serializeJson(root,configFile);

  configFile.close();
  return true;
}
