// ESP8266 WiFi Relay board
// by Michele <o-zone@zerozone.it> Pinassi 
//
// Generic ESP8266 Board - 4MB Flash Size - FS: 1MB
//
// v0.0.1 - Pre-production release, just for testing
//

#define __DEBUG__

// Firmware data
const char BUILD[] = __DATE__ " " __TIME__;
#define FW_NAME         "wifirelay"
#define FW_VERSION      "0.0.1"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ESP8266httpUpdate.h>
#include <EEPROM.h>

#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// ArduinoJson
// https://arduinojson.org/
#include <ArduinoJson.h>

// NTP ClientLib
// https://github.com/gmag11/NtpClient
#include <NtpClientLib.h>

// NTP
NTPSyncEvent_t ntpEvent;
bool syncEventTriggered = false; // True if a time even has been triggered

// Task Scheduler
#include <TaskScheduler.h>

// Scheduler and tasks...
Scheduler runner;

#define MQTT_INTERVAL 60000*15 // Every 15 minutes...
void mqttRunnerCB();
Task mqttTask(MQTT_INTERVAL, TASK_FOREVER, &mqttRunnerCB);

// MQTT PubSub client
// https://github.com/knolleary/pubsubclient
#include <PubSubClient.h>

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Board pinouts
#define LED_PIN    2  // Blue LED- GPIO2
#define RELAY_PIN    4  // Relay control - GPIO4
#define OPTO_IN_PIN    5  // Optocoupler input  - GPIO5

// File System
#include <FS.h>

// Format SPIFFS if mount failed
#define FORMAT_SPIFFS_IF_FAILED 1

// Web server
AsyncWebServer server(80);

// Config
struct Config {
  // WiFi config
  char wifi_essid[24];
  char wifi_password[24];
  // NTP Config
  char ntp_server[16];
  int8_t ntp_timezone;
  // MQTT broker
  char broker_host[24];
  unsigned int broker_port;
  char client_id[24];
  // Host config
  char hostname[16];
  bool ota_enable;
};

File configFile;
Config config; // Global config object

#define CONFIG_FILE "/config.json"

unsigned int last=0;
DynamicJsonDocument env(128);

// ************************************
// DEBUG()
//
// send message to Serial
// ************************************
void DEBUG(String message) {
#ifdef __DEBUG__
  Serial.println(message);
#endif
}

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println();
  Serial.println();
  Serial.print(FW_NAME);
  Serial.print(" ");
  Serial.print(FW_VERSION);
  Serial.print(" ");
  Serial.println(BUILD);

  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    DEBUG("[SPIFFS] Mount Failed. Try formatting...");
    if(SPIFFS.format()) {
      DEBUG("[SPIFFS] Initialized successfully");
    } else {
      DEBUG("[SPIFFS] Fatal error: restart!");
      ESP.restart();
    }
  } else {
    DEBUG("[SPIFFS] OK");
  }

  DEBUG("[BOARD] Initialize PINs...");
  pinMode(RELAY_PIN, OUTPUT);       // Relay control pin.
  pinMode(OPTO_IN_PIN, INPUT_PULLUP); // Input pin.
  pinMode(LED_PIN, OUTPUT);         // ESP8266 module blue LED.
  digitalWrite(RELAY_PIN, 0);       // Set relay control pin low.

  // Load configuration
  loadConfigFile();

  // Scan WiFi network
  scanWifi();
  
  // Connect to WiFi network
  connectToWifi();

  // Add MQTT task
  runner.addTask(mqttTask);
  mqttTask.enable();
  
  // Setup OTA
  ArduinoOTA.onStart([]() {
    Serial.println("[OTA] Update Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("[OTA] Update End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    char p[32];
    sprintf(p, "[OTA] Progress: %u%%\n", (progress/(total/100)));
    DEBUG(p);
  });
  ArduinoOTA.onError([](ota_error_t error) {
    if(error == OTA_AUTH_ERROR) DEBUG("[OTA] Auth Failed");
    else if(error == OTA_BEGIN_ERROR) DEBUG("[OTA] Begin Failed");
    else if(error == OTA_CONNECT_ERROR) DEBUG("[OTA] Connect Failed");
    else if(error == OTA_RECEIVE_ERROR) DEBUG("[OTA] Recieve Failed");
    else if(error == OTA_END_ERROR) DEBUG("[OTA] End Failed");
  });
  ArduinoOTA.setHostname(config.hostname);
  ArduinoOTA.begin();

  // Initialize web server on port 80
  initWebServer();

  // Set default values
  env["relay_1"] = "open";
}

void loop() {
  if(config.ota_enable) {
    // Handle OTA
    ArduinoOTA.handle();
  }

  // Scheduler
  runner.execute();

  // NTP ?
  if(syncEventTriggered) {
    processSyncEvent(ntpEvent);
    syncEventTriggered = false;
  }

  if((millis() - last) > 5100) {
    if(WiFi.status() != WL_CONNECTED) {
      connectToWifi();
    }

    bool relay = digitalRead(RELAY_PIN);
    if(relay) {
      // TRUE - CHIUSO
      env["relay_1"] = "closed";
    } else {
      // FALSE - APERTO
      env["relay_1"] = "open";
    }

    env["uptime"] = millis() / 1000;

    last = millis();
  }
  delay(10);
}
