# Make Elpis flower glow with colors

## 前言
畢竟那朵花陪了我整主線，看到官方真的出了商品，就有想法了，如果可以改造我就拆了它。

拿到商品就動手開始拆，以下就是這個專案的說明。

---

## 專案說明
### 硬體需求

- NodeMCU-32S (ESP32)
- MFRC522 RFID 模組
- WS2812B LED 燈條 (2 顆)
- RFID 卡片/標籤

以及最重要的

- [那朵花](https://na.store.square-enix-games.com/final-fantasy-xiv-elpis-flower-light)
> 這個專案需要對花的實體進行**破壞性**改造，請確保擁有足夠知識及工具進行安全的操作。

#### 為什麼需要破壞？
```
你需要在
1.內部構造中鑽一個0.5mm的洞
2.拆卸原有的電源開關按鈕
以提供LED控制走線的空間，幸好這個破壞在外觀上並不影響視覺效果。
```
**不能接受破壞性改造的可以不用往下看了。**

---
## 花朵結構及拆卸步驟
你確定要拆了你的花？

認真的？

那請 [看這裡](https://github.com/ltd2001/elpisFlower/blob/main/TEARDOWN.md)

## 軟體開發環境

本專案使用 **PlatformIO** 進行開發。

本專案使用 **Claude AI (Anthropic 開發的 AI 助理)** 協助開發。

### 安裝 PlatformIO

**需求**:
- [Visual Studio Code](https://code.visualstudio.com/)
- PlatformIO IDE 擴充套件

### 建置專案

**Clone 倉庫並開啟**:
```bash
git clone https://github.com/ltd2001/elpisFlower.git
cd elpisFlower
code .
```

PlatformIO 會自動安裝所需的函式庫和工具鏈。

**編譯**:
```bash
pio run
```

**上傳程式到開發板**:
```bash
pio run --target upload
```

**上傳檔案系統 (data 資料夾)**:
```bash
pio run --target uploadfs
```

---

## 專案結構

```
elpisFlower/
├── platformio.ini          # PlatformIO 專案設定
├── src/
│   └── main.cpp           # 主程式
├── data/                  # 網頁檔案 (需上傳至 LittleFS)
│   ├── index.html
│   ├── write.html
│   ├── color.html
│   ├── colors.json
│   └── app.js
├── include/               # 標頭檔
└── lib/                   # 自訂函式庫
```

---

## 功能說明

### RFID 讀取
自動偵測 RFID 卡片並讀取標籤資料。

### LED 顯示

未讀取RFID標籤時預設為白色。（包含開機時）

讀取RFID時根據標籤顯示對應顏色:
- PLD: 青色
- MCH: 綠色
- AZM: 橙色
- (自行增加對應)

### WiFi 熱點
ESP32 會建立 WiFi 熱點供使用者連接。

### Web 介面
- 查看和寫入 RFID 卡片資料。
- 手動指定LED顏色。

---

## 使用方式

1. 編譯並上傳程式到 ESP32
2. 上傳檔案系統內容 (data 資料夾)
3. ESP32 會建立 WiFi 熱點 `ESP32-FLOWER-MAIN`
4. 連接熱點,預設密碼為 `12345678`
5. 開啟瀏覽器訪問 `192.168.4.1`
6. 透過 Web 介面操作 RFID 讀寫功能、LED 顯示功能

---

## 硬體接線

### MFRC522 RFID 模組

| MFRC522 Pin | ESP32 GPIO |
|-------------|------------|
| SDA/SS      | 22         |
| SCK         | 18         |
| MOSI        | 23         |
| MISO        | 19         |
| RST         | 21         |
| 3.3V        | 3.3V       |
| GND         | GND        |

### WS2812B LED 燈條

| LED Pin | ESP32 GPIO |
|---------|------------|
| DIN     | 27         |
| 5V      | 5V         |
| GND     | GND        |

---

## 函式庫依賴

專案使用以下函式庫 (由 PlatformIO 自動管理):
- FastLED (v3.7.0+)
- MFRC522 (v1.4.11+)
- ArduinoJson
- ESP32 內建函式庫: WiFi, WebServer, LittleFS, SPI

---

## Copyright & Credits 版權與聲明

### This is a fan-made project using assets and references from FINAL FANTASY XIV.  
© SQUARE ENIX CO., LTD. All Rights Reserved.  
FINAL FANTASY XIV and related materials are the property of SQUARE ENIX CO., LTD.

This project is not affiliated with or endorsed by SQUARE ENIX.

---

### 本專案為基於《FINAL FANTASY XIV》的二次創作。  
© SQUARE ENIX CO., LTD. All Rights Reserved.  
《FINAL FANTASY XIV》及其相關素材版權均屬於 SQUARE ENIX CO., LTD.

本專案與 SQUARE ENIX 並無任何關聯或官方授權。

---

README ver 1.0.1