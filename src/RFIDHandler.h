#ifndef RFID_HANDLER_H
#define RFID_HANDLER_H

#include <SPI.h>
#include <MFRC522.h>

class RFIDHandler {
private:
  MFRC522 rfid;
  MFRC522::MIFARE_Key key;
  
public:
  RFIDHandler(int ssPin, int rstPin) : rfid(ssPin, rstPin) {}
  
  void init() {
    SPI.begin();
    rfid.PCD_Init();
    for (int i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF;
    }
    Serial.println(F("RFID 初始化完成"));
  }
  
  bool read(String &uid, String &label) {
    // 檢查是否有新卡片
    if (!rfid.PICC_IsNewCardPresent()) {
      return false;
    }
    
    if (!rfid.PICC_ReadCardSerial()) {
      Serial.println(F("無法選擇卡片"));
      return false;
    }
    
    // 重置 UID
    uid = "";
    
    // 讀取 UID
    for (byte i = 0; i < rfid.uid.size; i++) {
      uid += String(rfid.uid.uidByte[i], HEX);
    }
    
    // 進行身份驗證
    MFRC522::StatusCode status = rfid.PCD_Authenticate(
      MFRC522::PICC_CMD_MF_AUTH_KEY_A, 
      7, 
      &key, 
      &(rfid.uid)
    );
    
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("驗證失敗: "));
      Serial.println(rfid.GetStatusCodeName(status));
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
      return false;
    }
    
    // 讀取 Block 4 的資料
    byte buffer[18];
    byte len = 18;
    status = rfid.MIFARE_Read(4, buffer, &len);
    
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("讀取失敗: "));
      Serial.println(rfid.GetStatusCodeName(status));
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
      return false;
    }
    
    // 解析 Label
    label = "";
    for (int i = 0; i < 16; i++) {
      if (buffer[i] == 0) break;
      label += (char)buffer[i];
    }
    
    Serial.print(F("讀取成功 - UID: "));
    Serial.print(uid);
    Serial.print(F(", Label: "));
    Serial.println(label);
    
    // 停止通訊
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    
    return true;
  }
  
  bool write(String label) {
    rfid.PCD_Init();
    
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
      return false;
    }
    
    if (rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(rfid.uid)) != MFRC522::STATUS_OK) {
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
      return false;
    }
    
    byte buffer[16] = {0};
    label.getBytes(buffer, 16);
    
    MFRC522::StatusCode status = rfid.MIFARE_Write(4, buffer, 16);
    
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    
    if (status == MFRC522::STATUS_OK) {
      Serial.print(F("寫入成功: "));
      Serial.println(label);
      return true;
    }
    
    Serial.print(F("寫入失敗: "));
    Serial.println(rfid.GetStatusCodeName(status));
    return false;
  }
};

#endif