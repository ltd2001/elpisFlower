#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <FastLED.h>

#include "ColorMapping.h"
#include "LEDController.h"
#include "RFIDHandler.h"
#include "WebServerHandler.h"

// 硬體定義
#define LED_PIN 27
#define NUM_LEDS 2
#define RST_PIN 21
#define SS_PIN 22

// 全域物件
CRGB leds[NUM_LEDS];
LEDController ledController(leds, NUM_LEDS);
RFIDHandler rfidHandler(SS_PIN, RST_PIN);
ColorMapping colorMapping;
WebServerHandler webServer;

// RFID 狀態
bool rfid_present = false;
String rfid_uid = "";
String rfid_label = "";
unsigned long lastRFIDCheck = 0;
const unsigned long RFID_CHECK_INTERVAL = 1450;

// 寫入 RFID 的回呼函數
void onWriteRFID(String label) {
  if (rfidHandler.write(label)) {
    Serial.println(F("RFID 寫入成功"));
  } else {
    Serial.println(F("RFID 寫入失敗"));
  }
}

// 重新載入顏色設定的回呼函數
void onReloadColors() {
  if (colorMapping.loadFromFile()) {
    Serial.println(F("顏色設定重新載入成功"));
  } else {
    Serial.println(F("顏色設定重新載入失敗"));
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println(F("\n=== Elpis Flower 初始化 ==="));
  
  // 初始化 FastLED
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  ledController.init(128);
  Serial.println(F("LED 初始化完成"));
  
  // 初始化 RFID
  rfidHandler.init();
  
  // 初始化 LittleFS
  if (!LittleFS.begin(true)) {
    Serial.println(F("LittleFS 啟動失敗"));
    return;
  }
  Serial.println(F("LittleFS 初始化完成"));
  
  // 載入顏色對應表
  if (!colorMapping.loadFromFile()) {
    Serial.println(F("警告：顏色對應表載入失敗，將使用預設黑色"));
  }
  
  // 啟動 WiFi AP
  WiFi.softAP("ESP32-FLOWER-MAIN", "12345678");
  Serial.print(F("WiFi AP IP: "));
  Serial.println(WiFi.softAPIP());
  
  // 設定 Web Server
  webServer.setRFIDStatus(&rfid_present, &rfid_label);
  webServer.setWriteCallback(onWriteRFID);
  webServer.setReloadColorsCallback(onReloadColors);
  webServer.begin();
  
  Serial.println(F("=== 初始化完成 ===\n"));
}

void loop() {
  // 處理 Web Server 請求
  webServer.handleClient();
  
  // 更新 LED 漸變效果
  ledController.update();
  
  // 定期檢查 RFID
  if (millis() - lastRFIDCheck > RFID_CHECK_INTERVAL) {
    String uid, label;
    bool nowPresent = rfidHandler.read(uid, label);
    
    // 如果是新卡片或狀態改變
    if (nowPresent && uid != rfid_uid) {
      rfid_present = true;
      rfid_uid = uid;
      rfid_label = label;
      
      // 查詢顏色並開始漸變
      CRGB lower, upper;
      if (colorMapping.getColors(label, lower, upper)) {
        ledController.startFade(lower, upper);
      } else {
        Serial.print(F("未找到 label: "));
        Serial.print(label);
        Serial.println(F(" 的顏色設定，使用黑色"));
        ledController.startFade(CRGB::Black, CRGB::Black);
      }
    } else if (!nowPresent && rfid_present) {
      // 卡片移除
      rfid_present = false;
      rfid_uid = "";
      rfid_label = "";
    }
    
    lastRFIDCheck = millis();
  }
}