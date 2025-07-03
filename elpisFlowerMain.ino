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

#define RST_PIN 21
#define SS_PIN 22
MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

WebServer server(80);

int previousMillis = 0;
int refreshRate = 1450;

bool rfid_present = false;
String rfid_uid = "";
String rfid_label = "";

/* new function use but not stable
bool writingActive = false;
bool writeReady = false;
String currentUID = "";
String newUID = "";
String newLabel = "";
String pendingData = "";
*/

void setupRFID() {
  SPI.begin();
  rfid.PCD_Init();
  for (int i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
}

bool readRFID(String &uid, String &label) {
		
  if(!rfid.PICC_IsNewCardPresent()){
	Serial.println(F("Same Card.")); 
	return false;  
  }
  
  if (!rfid.PICC_ReadCardSerial()){
    Serial.println(F("Not able select card."));
    return false;
  }
  
  //有卡片才reset
  uid = "";
  
  // 提取卡片的 UID
  for (byte i = 0; i < rfid.uid.size; i++) uid += String(rfid.uid.uidByte[i], HEX);
  Serial.print(F("ReadRFID: uid= "));
  Serial.println(uid);

  byte buffer[18]; byte len = 18;
  MFRC522::StatusCode status; //宣告status項目
	
    // 嘗試進行身份驗證
    if (rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 7, &key, &(rfid.uid)) != MFRC522::STATUS_OK) {
      Serial.print(F("Authentication failed: "));
      Serial.println(rfid.GetStatusCodeName(status));  // 打印身份驗證錯誤
	  rfid.PICC_HaltA();
	  rfid.PCD_StopCrypto1();
      return false;
    }

    // 成功身份驗證後，讀取資料
    status = rfid.MIFARE_Read(4, buffer, &len);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("Read failed: "));
      Serial.println(rfid.GetStatusCodeName(status));  // 打印讀取錯誤
	  rfid.PICC_HaltA();
	  rfid.PCD_StopCrypto1();
      return false;
    }

    // 讀取標籤資料
    label = "";
    for (int i = 0; i < 16; i++) {
      if (buffer[i] == 0) break;
      label += (char)buffer[i];
    }
    Serial.print(F("ReadRFID: label= "));
    Serial.println(label);

  // 停止與卡片的通訊
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  return true;
}

void writeRFID(String label) {
	
	rfid.PCD_Init();

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
  } else if (label == "AZM") {
    leds[0] = CRGB::Orange;
    leds[1] = CRGB::Yellow;
  } else {
    leds[0] = CRGB::Black;
    leds[1] = CRGB::Black;
  }
  FastLED.show();
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
  String json = "{";
  json += "\"present\":" + String(rfid_present ? "true" : "false") + ",";
  json += "\"label\":\"" + rfid_label + "\"}";
  server.send(200, "application/json", json);
//  Serial.print("handleJson: "); //Debug use
//  Serial.println(json);
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
/*
void handleBeginWrite() {
  if (!server.hasArg("data")) {
    server.send(400, "text/plain", "缺少資料");
    return;
  }
  pendingData = server.arg("data");
  writingActive = true;
  writeReady = false;
  currentUID = "";
  newUID = "";
  newLabel = "";
  server.send(200, "text/plain", "開始寫入流程");
}

void handleWriteStatus() {
  String json = "{";
  json += "\"writingActive\":" + String(writingActive ? "true" : "false") + ",";
  json += "\"present\":" + String(rfid_present ? "true" : "false") + ",";
  json += "\"uid\":\"" + rfid_uid + "\",";
  json += "\"label\":\"" + rfid_label + "\",";
  json += "\"pending\":\"" + pendingData + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

void handleConfirmWrite() {
  if (writeReady && pendingData != "" && rfid_present && rfid_uid == newUID) {
    if (writeRFID(pendingData)) {
      writingActive = false;
      writeReady = false;
      server.send(200, "text/plain", "寫入成功");
    } else {
      server.send(500, "text/plain", "寫入失敗");
    }
  } else {
    server.send(400, "text/plain", "條件不符，無法寫入");
  }
}
*/

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(128);
  FastLED.clear(); FastLED.show();

  setupRFID();
  WiFi.softAP("ESP32-FLOWER-MAIN", "12345678");

  if (!LittleFS.begin()) {
    Serial.println(F("LittleFS 啟動失敗"));
    return;
  }

  server.on("/", handleRoot);
  server.on("/write.html", handleWritePage);
  server.on("/app.js", handleJS);
  server.on("/status", handleStatus);
  server.on("/write", HTTP_POST, handleWrite);
//  server.on("/begin_write", HTTP_POST, handleBeginWrite);
//  server.on("/write_status", handleWriteStatus);
//  server.on("/confirm_write", HTTP_POST, handleConfirmWrite);

  server.begin();
}


void loop() {
	
  server.handleClient();

  if(millis() - previousMillis > refreshRate){
	  String uid, label;
	  bool nowPresent = readRFID(uid, label);

	  // 更新快取
	  rfid_present = nowPresent;
	  rfid_uid = uid;
	  rfid_label = label;
	  previousMillis = millis();
  }

  // 燈光切換（首次放入新卡片）
  if (rfid_present && rfid_uid != "") {
    applyLED(rfid_label);
  }

/*  // 控制寫入狀態流程
  if (writingActive) {
    if (currentUID == "" && rfid_present) {
      currentUID = rfid_uid;  // 原始卡片
    } else if (currentUID != "" && !rfid_present) {
      currentUID = "";  // 卡片已移除
    } else if (currentUID == "" && rfid_present && !writeReady) {
      newUID = rfid_uid;
      newLabel = rfid_label;
      writeReady = true;
    }
  }*/
  
} // END OF LOOP