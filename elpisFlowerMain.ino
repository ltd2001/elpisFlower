#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <FS.h>
#include <FastLED.h>
#include <SPI.h>
#include <MFRC522.h>

#define LED_PIN 27
#define NUM_LEDS 2
CRGB leds[NUM_LEDS];

#define RST_PIN 22
#define SS_PIN 21
MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

WebServer server(80);

String lastUID = "";
String lastLabel = "";

void setupRFID() {
  SPI.begin();
  rfid.PCD_Init();
  for (int i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
}

bool readRFID(String &uid, String &label) {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return false;

  uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) uid += String(rfid.uid.uidByte[i], HEX);

  byte buffer[18]; byte len = 18;
  if (rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(rfid.uid)) != MFRC522::STATUS_OK) return false;
  if (rfid.MIFARE_Read(4, buffer, &len) != MFRC522::STATUS_OK) return false;

  label = "";
  for (int i = 0; i < 16; i++) {
    if (buffer[i] == 0) break;
    label += (char)buffer[i];
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  
  Serial.println(label);
  return true;
}

void writeRFID(String label) {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;
  if (rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(rfid.uid)) != MFRC522::STATUS_OK) return;

  byte buffer[16] = {0};
  label.getBytes(buffer, 16);
  rfid.MIFARE_Write(4, buffer, 16);

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void applyLED(String label) {
  if (label == "PLD") {
    leds[0] = CRGB::Blue;
    leds[1] = CRGB::Aqua;
  } else if (label == "MCH") {
    leds[0] = CRGB::Aqua;
    leds[1] = CRGB::Teal;
  } else {
    leds[0] = CRGB::Black;
    leds[1] = CRGB::Black;
  }
  FastLED.show();
}

void handleRoot() {
  File file = LittleFS.open("/index.html", "r");
  server.streamFile(file, "text/html");
  file.close();
}

void handleWritePage() {
  File file = LittleFS.open("/write.html", "r");
  server.streamFile(file, "text/html");
  file.close();
}

void handleJS() {
  File file = LittleFS.open("/app.js", "r");
  server.streamFile(file, "application/javascript");
  file.close();
}

void handleStatus() {
  String uid, label;
  bool present = readRFID(uid, label);
  if (present && uid != lastUID) {
    lastUID = uid;
    lastLabel = label;
    applyLED(label);
  }
  String json = "{";
  json += "\"present\":" + String(present ? "true" : "false") + ",";
  json += "\"label\":\"" + lastLabel + "\"}";
  server.send(200, "application/json", json);
  Serial.println(json);
}

void handleWrite() {
  if (!server.hasArg("data")) {
    server.send(400, "text/plain", "缺少資料");
    return;
  }
  String data = server.arg("data");
  writeRFID(data);
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2812B, LED_PIN>(leds, NUM_LEDS);
  FastLED.clear(); FastLED.show();

  setupRFID();
  WiFi.softAP("ESP32-FLOWER-MAIN", "12345678");

  if (!LittleFS.begin()) {
    Serial.println("LittleFS 啟動失敗");
    return;
  }

  server.on("/", handleRoot);
  server.on("/write.html", handleWritePage);
  server.on("/app.js", handleJS);
  server.on("/status", handleStatus);
  server.on("/write", HTTP_POST, handleWrite);
  server.begin();
}

void loop() {
  server.handleClient();
}
