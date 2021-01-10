// Written by Michele <o-zone@zerozone.it> Pinassi
// Released under GPLv3 - No any warranty

String templateProcessor(const String& var)
{
  //
  // System values
  //
  if(var == "hostname") {
    return String(config.hostname);
  }
  if(var == "fw_name") {
    return String(FW_NAME);
  }
  if(var=="fw_version") {
    return String(FW_VERSION);
  }
  if(var=="uptime") {
    return String(millis()/1000);
  }
  if(var=="timedate") {
    return NTP.getTimeDateString();
  }
  if(var=="relay_1_status") {
    return env["relay_1"];
  }
  
  //
  // Config values
  //
  if(var=="wifi_essid") {
    return String(config.wifi_essid);
  }
  if(var=="wifi_password") {
    return String(config.wifi_password);
  }
  if(var=="wifi_rssi") {
    return String(WiFi.RSSI());
  }
  if(var=="ntp_server") {
    return String(config.ntp_server);
  }
  if(var=="ntp_timezone") {
    return String(config.ntp_timezone);
  }
  if(var=="ota_enable") {
    if(config.ota_enable) {
      return String("checked");  
    } else {
      return String("");
    }
  }
  return String();
}

// ************************************
// initWebServer
//
// initialize web server
// ************************************
void initWebServer() {
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html").setTemplateProcessor(templateProcessor);

  server.on("/restart", HTTP_POST, [](AsyncWebServerRequest *request) {
    ESP.restart();
  });
    
  server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request) {
    // 
    if(request->hasParam("wifi_essid", true)) {
        strcpy(config.wifi_essid,request->getParam("wifi_essid", true)->value().c_str());
    }
    if(request->hasParam("wifi_password", true)) {
        strcpy(config.wifi_password,request->getParam("wifi_password", true)->value().c_str());
    }
    if(request->hasParam("ntp_server", true)) {
        strcpy(config.ntp_server, request->getParam("ntp_server", true)->value().c_str());
    }
    if(request->hasParam("ntp_timezone", true)) {
        config.ntp_timezone = atoi(request->getParam("ntp_timezone", true)->value().c_str());
    } 
    if(request->hasParam("ota_enable", true)) {
      config.ota_enable=true;        
    } else {
      config.ota_enable=false;
    } 
    if(saveConfigFile()) {
      request->redirect("/?result=ok");
    } else {
      request->redirect("/?result=error");      
    }
  });
  
  server.on("/ajax", HTTP_POST, [] (AsyncWebServerRequest *request) {
    String action,value,response="";

    if (request->hasParam("action", true)) {
      action = request->getParam("action", true)->value();
      Serial.print("ACTION: ");
      if(action.equals("relay_1")) {
        value = request->getParam("value", true)->value();
        if(value.equals("open")) { // Open the RELAY
            digitalWrite(RELAY_PIN, 0);       // Set relay control pin low=open
            response="Relay OPENED";
        } else if(value.equals("closed")) { // Close the RELAY
            digitalWrite(RELAY_PIN, 1);       // Set relay control pin high=close
            response="Relay CLOSED";
        }
      }
    }
    request->send(200, "text/plain", response);
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    Serial.printf("404 NOT FOUND - http://%s%s\n", request->host().c_str(), request->url().c_str());
    request->send(404);
  });

  server.begin();
}
