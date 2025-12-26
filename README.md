# Make elpis flower actually glow

隨便做做,我真的拆了那朵花。
它在盒子裡的時候葉子就斷了。

---

## 專案說明

ESP32 RFID 智慧花盆專案 - 使用 FastLED 和 MFRC522 實現 RFID 感應與 LED 燈光控制。

### 硬體需求

- NodeMCU-32S (ESP32)
- MFRC522 RFID 模組
- WS2812B LED 燈條 (2 顆)
- RFID 卡片/標籤

---

## 開發環境

本專案使用 **PlatformIO** 進行開發。
本專案使用 Claude AI (Anthropic 開發的 AI 助理) 協助開發。
> **注意**: Arduino IDE 開發方式將於未來版本中棄用 (deprecated)。建議所有開發者遷移至 PlatformIO 環境。

### 安裝 PlatformIO

**需求**:
- [Visual Studio Code](https://code.visualstudio.com/)
- PlatformIO IDE 擴充套件

**安裝步驟**:
1. 安裝 Visual Studio Code
2. 在擴充套件市場搜尋並安裝 "PlatformIO IDE"
3. 重新啟動 VSCode

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

### VSCode 操作

在 VSCode 底部狀態列可使用快速按鈕:
- 勾選圖示: 編譯專案
- 箭頭圖示: 上傳程式
- 插頭圖示: 開啟序列監視器

上傳檔案系統:
1. 點擊左側 PlatformIO 圖示
2. 展開 PROJECT TASKS > Platform
3. 點擊 "Upload Filesystem Image"

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
│   └── app.js
├── include/               # 標頭檔
└── lib/                   # 自訂函式庫
```

---

## 功能說明

### RFID 讀取
自動偵測 RFID 卡片並讀取標籤資料。

### LED 顯示
根據不同標籤顯示對應顏色:
- PLD: 藍色/青色
- MCH: 綠色系
- AZM: 橙色/黃色

### WiFi 熱點
ESP32 會建立 WiFi 熱點供使用者連接。

### Web 介面
透過瀏覽器查看和寫入 RFID 卡片資料。

---

## 使用方式

1. 編譯並上傳程式到 ESP32
2. 上傳檔案系統內容 (data 資料夾)
3. ESP32 會建立 WiFi 熱點 `ESP32-FLOWER-MAIN`
4. 連接熱點,密碼為 `12345678`
5. 開啟瀏覽器訪問 `192.168.4.1`
6. 透過 Web 介面操作 RFID 讀寫功能

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
- ESP32 內建函式庫: WiFi, WebServer, LittleFS, SPI

---

## 開發注意事項

### 序列埠監視
預設鮑率為 9600。使用序列監視器時請確認設定正確。

### 檔案系統
專案使用 LittleFS 檔案系統。上傳 data 資料夾內容前,請確認檔案路徑正確。

### 編譯最佳化
`platformio.ini` 已針對 ESP32 進行編譯優化設定,包含:
- Release 建置模式
- CPU 頻率 240MHz
- Flash 頻率 80MHz
- QIO Flash 模式

---

## 疑難排解

### 編譯錯誤
確認 PlatformIO Core 已正確安裝,並檢查 Python 環境設定。

### 上傳失敗
檢查 USB 連接和序列埠權限。Linux 使用者需將使用者加入 dialout 群組:
```bash
sudo usermod -a -G dialout $USER
```

### 檔案系統上傳失敗
確認 data 資料夾位置正確,且檔案內容符合 LittleFS 限制。

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

README ver 0.2.0