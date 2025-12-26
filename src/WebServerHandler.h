#ifndef WEBSERVER_HANDLER_H
#define WEBSERVER_HANDLER_H

#include <WebServer.h>
#include <LittleFS.h>

class WebServerHandler {
private:
  WebServer server;
  bool* rfidPresent;
  String* rfidLabel;
  
  void (*writeCallback)(String);
  void (*reloadColorsCallback)();
  
  void handleRoot() {
    File file = LittleFS.open("/index.html", "r");
    if (file) {
      server.streamFile(file, "text/html");
      file.close();
    } else {
      server.send(404, "text/plain", "index.html not found");
    }
  }
  
  void handleWritePage() {
    File file = LittleFS.open("/write.html", "r");
    if (file) {
      server.streamFile(file, "text/html");
      file.close();
    } else {
      server.send(404, "text/plain", "write.html not found");
    }
  }
  
  void handleJS() {
    File file = LittleFS.open("/app.js", "r");
    if (file) {
      server.streamFile(file, "application/javascript");
      file.close();
    } else {
      server.send(404, "text/plain", "app.js not found");
    }
  }
  
  void handleStatus() {
    String json = "{";
    json += "\"present\":" + String(*rfidPresent ? "true" : "false") + ",";
    json += "\"label\":\"" + *rfidLabel + "\"}";
    server.send(200, "application/json", json);
  }
  
  void handleWrite() {
    if (!server.hasArg("data")) {
      server.send(400, "text/plain", "缺少資料");
      return;
    }
    
    String data = server.arg("data");
    if (writeCallback) {
      writeCallback(data);
    }
    
    server.sendHeader("Location", "/");
    server.send(303);
  }
  
  void handleReloadColors() {
    if (reloadColorsCallback) {
      reloadColorsCallback();
      server.send(200, "text/plain", "顏色設定已重新載入");
    } else {
      server.send(500, "text/plain", "重新載入失敗");
    }
  }
  
  void handleGetColors() {
    File file = LittleFS.open("/colors.json", "r");
    if (file) {
      server.streamFile(file, "application/json");
      file.close();
    } else {
      server.send(404, "text/plain", "colors.json not found");
    }
  }
  
public:
  WebServerHandler() : server(80) {
    rfidPresent = nullptr;
    rfidLabel = nullptr;
    writeCallback = nullptr;
    reloadColorsCallback = nullptr;
  }
  
  void setRFIDStatus(bool* present, String* label) {
    rfidPresent = present;
    rfidLabel = label;
  }
  
  void setWriteCallback(void (*callback)(String)) {
    writeCallback = callback;
  }
  
  void setReloadColorsCallback(void (*callback)()) {
    reloadColorsCallback = callback;
  }
  
  void begin() {
    server.on("/", [this]() { handleRoot(); });
    server.on("/write.html", [this]() { handleWritePage(); });
    server.on("/app.js", [this]() { handleJS(); });
    server.on("/status", [this]() { handleStatus(); });
    server.on("/write", HTTP_POST, [this]() { handleWrite(); });
    server.on("/reload_colors", HTTP_POST, [this]() { handleReloadColors(); });
    server.on("/colors.json", [this]() { handleGetColors(); });
    
    server.begin();
    Serial.println(F("Web Server 已啟動"));
  }
  
  void handleClient() {
    server.handleClient();
  }
};

#endif