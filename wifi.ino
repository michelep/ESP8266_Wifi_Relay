// 
// WiFi
//


// ************************************
// scanWifi()
//
// scan available WiFi networks
// ************************************
void scanWifi() {
  WiFi.mode(WIFI_STA);
  Serial.println("--------------");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("--------------");
}

// ************************************
// connectToWifi()
//
// connect to configured WiFi network
// ************************************
bool connectToWifi() {
  uint8_t timeout=0;
  if(strlen(config.wifi_essid) > 0) {
    DEBUG("[INIT] Connecting to "+String(config.wifi_essid));
    
    WiFi.mode(WIFI_STA);
    WiFi.hostname(config.hostname);
    WiFi.begin(config.wifi_essid, config.wifi_password);

    while((WiFi.status() != WL_CONNECTED) && (timeout < 10)) {
      delay(250);
      DEBUG(".");
      delay(250);
      timeout++;
    }
    
    if(WiFi.status() == WL_CONNECTED) {
      DEBUG("OK. IP:"+WiFi.localIP().toString());
      DEBUG("Signal strength (RSSI):"+String(WiFi.RSSI())+" dBm");

      if (MDNS.begin(config.hostname)) {
        DEBUG("[INIT] MDNS responder started");
        // Add service to MDNS-SD
        MDNS.addService("http", "tcp", 80);
      }
      
      NTP.onNTPSyncEvent([](NTPSyncEvent_t event) {
        ntpEvent = event;
        syncEventTriggered = true;
      });

      // NTP
      DEBUG("[INIT] NTP sync time on "+String(config.ntp_server));
      NTP.begin(config.ntp_server, config.ntp_timezone, true, 0);
      NTP.setInterval(63);

      return true;  
    } else {
      DEBUG("[ERROR] Failed to connect to WiFi");
      WiFi.printDiag(Serial);
      return false;
    }
  } else {
      DEBUG("[ERROR] WiFi configuration missing!");
      return false;
  }
}
