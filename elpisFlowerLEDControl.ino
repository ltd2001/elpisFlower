// Gemini Flash 2.5 Canvas Test Run for Color Correction Code

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#include <ArduinoJson.h> // 用於解析網頁發送的 JSON 數據

// --- Wi-Fi AP 模式設定 ---
const char* ssid = "ESP32_WS2812B_Controller";
const char* password = "12345678"; // 預設密碼

// --- FastLED 燈珠設定 ---
// 現在只有一個燈串，包含兩個燈珠
#define DATA_PIN 27 // 燈串的數據引腳 (GPIO27)
#define NUM_LEDS 2 // 燈串上燈珠的總數量

// 定義一個 FastLED 燈條陣列，包含 NUM_LEDS 個燈珠
CRGB leds[NUM_LEDS];

// --- 當前燈珠狀態變數 (用於網頁顯示和應用) ---
// 燈珠 0 (原燈珠 A)
CRGB currentLed0_Color = CRGB::Black; // 燈珠 0 當前顏色
CRGB currentLed0_Correction = CRGB(255, 255, 255); // 燈珠 0 當前校正值 (預設無校正)
String currentLed0_CorrectionType = "manual"; // 燈珠 0 當前校正類型 ("manual" 或 "preset_typical" 或 "preset_uncorrected")

// 燈珠 1 (原燈珠 B)
CRGB currentLed1_Color = CRGB::Black; // 燈珠 1 當前顏色
CRGB currentLed1_Correction = CRGB(255, 255, 255); // 燈珠 1 當前校正值 (預設無校正)
String currentLed1_CorrectionType = "manual"; // 燈珠 1 當前校正類型 ("manual" 或 "preset_typical" 或 "preset_uncorrected")

// 全局亮度
uint8_t currentGlobalBrightness = 50; // 當前全局亮度 (預設 50)

// --- 網頁伺服器實例 ---
AsyncWebServer server(80); // 在 80 埠上啟動網頁伺服器

// --- 網頁 HTML/CSS/JavaScript 內容 ---
// 為了簡潔和直接嵌入，將所有內容放在一個字串中。
// 注意：在實際專案中，對於更複雜的網頁，可以考慮使用 LittleFS 或 SPIFFS 儲存文件。
const char* HTML_CONTENT = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WS2812B 控制器</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background-color: #f0f0f0; color: #333; }
        .container { max-width: 600px; margin: auto; background-color: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); }
        h1, h2 { text-align: center; color: #0056b3; }
        .led-section { border: 1px solid #ccc; border-radius: 5px; padding: 15px; margin-bottom: 20px; background-color: #f9f9f9; }
        .input-group { margin-bottom: 10px; display: flex; align-items: center; }
        .input-group label { flex: 1; margin-right: 10px; font-weight: bold; }
        .input-group input[type="color"], .input-group input[type="number"], .input-group select { flex: 2; padding: 8px; border: 1px solid #ddd; border-radius: 4px; }
        .input-group input[type="color"] { height: 40px; }
        .current-values { margin-top: 15px; padding: 10px; border: 1px dashed #bbb; background-color: #e9e9e9; border-radius: 4px; font-size: 0.9em; }
        .current-values p { margin: 5px 0; }
        button {
            display: block;
            width: 100%;
            padding: 12px 20px;
            background-color: #007bff;
            color: white;
            border: none;
            border-radius: 5px;
            font-size: 1.1em;
            cursor: pointer;
            margin-top: 20px;
            transition: background-color 0.3s ease;
        }
        button:hover { background-color: #0056b3; }
    </style>
</head>
<body>
    <div class="container">
        <h1>WS2812B 燈珠控制器</h1>

        <div class="led-section">
            <h2>燈珠 0</h2> <!-- 修正為燈珠 0 -->
            <div class="input-group">
                <label for="colorA">顏色選擇:</label>
                <input type="color" id="colorA" value="#000000">
            </div>
            <div class="input-group">
                <label>手動校正 (R,G,B):</label>
                <input type="number" id="corrAR" min="0" max="255" value="255">
                <input type="number" id="corrAG" min="0" max="255" value="255">
                <input type="number" id="corrAB" min="0" max="255" value="255">
            </div>
            <div class="input-group">
                <label for="corrAType">校正預設:</label>
                <select id="corrAType">
                    <option value="manual">手動輸入</option>
                    <option value="preset_typical">TypicalLEDStrip</option>
                    <option value="preset_uncorrected">UncorrectedColor</option>
                </select>
            </div>
            <div class="current-values">
                <p>當前顏色 (0): <span id="displayColorA">R:0 G:0 B:0</span></p>
                <p>當前校正 (0): <span id="displayCorrA">R:255 G:255 B:255 (手動)</span></p>
            </div>
        </div>

        <div class="led-section">
            <h2>燈珠 1</h2> <!-- 修正為燈珠 1 -->
            <div class="input-group">
                <label for="colorB">顏色選擇:</label>
                <input type="color" id="colorB" value="#000000">
            </div>
            <div class="input-group">
                <label>手動校正 (R,G,B):</label>
                <input type="number" id="corrBR" min="0" max="255" value="255">
                <input type="number" id="corrBG" min="0" max="255" value="255">
                <input type="number" id="corrBB" min="0" max="255" value="255">
            </div>
            <div class="input-group">
                <label for="corrBType">校正預設:</label>
                <select id="corrBType">
                    <option value="manual">手動輸入</option>
                    <option value="preset_typical">TypicalLEDStrip</option>
                    <option value="preset_uncorrected">UncorrectedColor</option>
                </select>
            </div>
            <div class="current-values">
                <p>當前顏色 (1): <span id="displayColorB">R:0 G:0 B:0</span></p>
                <p>當前校正 (1): <span id="displayCorrB">R:255 G:255 B:255 (手動)</span></p>
            </div>
        </div>

        <div class="input-group">
            <label for="globalBrightness">全局亮度 (0-255):</label>
            <input type="number" id="globalBrightness" min="0" max="255" value="50">
        </div>
        <div class="current-values">
            <p>當前全局亮度: <span id="displayGlobalBrightness">50</span></p>
        </div>

        <button onclick="submitSettings()">提交設定</button>
    </div>

    <script>
        // 將十六進位顏色轉換為 RGB 物件
        function hexToRgb(hex) {
            var r = parseInt(hex.substring(1, 3), 16);
            var g = parseInt(hex.substring(3, 5), 16);
            var b = parseInt(hex.substring(5, 7), 16);
            return { r, g, b };
        }

        // 提交設定到 ESP32
        async function submitSettings() {
            // 燈珠 0 數據 (原燈珠 A)
            const color0_hex = document.getElementById('colorA').value;
            const color0_rgb = hexToRgb(color0_hex);
            const corr0Type = document.getElementById('corrAType').value;
            const corr0R = parseInt(document.getElementById('corrAR').value);
            const corr0G = parseInt(document.getElementById('corrAG').value);
            const corr0B = parseInt(document.getElementById('corrAB').value);

            // 燈珠 1 數據 (原燈珠 B)
            const color1_hex = document.getElementById('colorB').value;
            const color1_rgb = hexToRgb(color1_hex);
            const corr1Type = document.getElementById('corrBType').value;
            const corr1R = parseInt(document.getElementById('corrBR').value);
            const corr1G = parseInt(document.getElementById('corrBG').value);
            const corr1B = parseInt(document.getElementById('corrBB').value);

            // 全局亮度
            const globalBrightness = parseInt(document.getElementById('globalBrightness').value);

            const data = {
                led0_r: color0_rgb.r,
                led0_g: color0_rgb.g,
                led0_b: color0_rgb.b,
                led0_correction_type: corr0Type,
                led0_corr_r: corr0R,
                led0_corr_g: corr0G,
                led0_corr_b: corr0B,

                led1_r: color1_rgb.r,
                led1_g: color1_rgb.g,
                led1_b: color1_rgb.b,
                led1_correction_type: corr1Type,
                led1_corr_r: corr1R,
                led1_corr_g: corr1G,
                led1_corr_b: corr1B,

                global_brightness: globalBrightness
            };

            try {
                const response = await fetch('/submit', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify(data)
                });

                if (response.ok) {
                    const result = await response.json();
                    if (result.status === "success") {
                        alert("設定已更新！");
                        updateDisplayValues(result.current_state); // 更新網頁顯示
                    } else {
                        alert("更新失敗: " + result.message);
                    }
                } else {
                    alert("發送請求失敗: " + response.statusText);
                }
            } catch (error) {
                console.error('錯誤:', error);
                alert("連接到 ESP32 失敗！");
            }
        }

        // 初始載入時從 ESP32 獲取當前設定
        async function fetchCurrentSettings() {
            try {
                const response = await fetch('/settings');
                if (response.ok) {
                    const settings = await response.json();
                    if (settings) {
                        // 更新輸入框的值
                        document.getElementById('colorA').value = `#${settings.led0_r.toString(16).padStart(2, '0')}${settings.led0_g.toString(16).padStart(2, '0')}${settings.led0_b.toString(16).padStart(2, '0')}`;
                        document.getElementById('corrAR').value = settings.led0_corr_r;
                        document.getElementById('corrAG').value = settings.led0_corr_g;
                        document.getElementById('corrAB').value = settings.led0_corr_b;
                        document.getElementById('corrAType').value = settings.led0_correction_type;

                        document.getElementById('colorB').value = `#${settings.led1_r.toString(16).padStart(2, '0')}${settings.led1_g.toString(16).padStart(2, '0')}${settings.led1_b.toString(16).padStart(2, '0')}`;
                        document.getElementById('corrBR').value = settings.led1_corr_r;
                        document.getElementById('corrBG').value = settings.led1_corr_g;
                        document.getElementById('corrBB').value = settings.led1_corr_b;
                        document.getElementById('corrBType').value = settings.led1_correction_type;

                        document.getElementById('globalBrightness').value = settings.global_brightness;

                        // 更新顯示值
                        updateDisplayValues(settings);
                    }
                }
            } catch (error) {
                console.error('無法獲取初始設定:', error);
            }
        }

        // 更新網頁上顯示的當前值
        function updateDisplayValues(state) {
            document.getElementById('displayColorA').innerText = `R:${state.led0_r} G:${state.led0_g} B:${state.led0_b}`;
            let corr0Text = `R:${state.led0_corr_r} G:${state.led0_corr_g} B:${state.led0_corr_b}`;
            if (state.led0_correction_type === "preset_typical") {
                corr0Text += " (TypicalLEDStrip)";
            } else if (state.led0_correction_type === "preset_uncorrected") {
                corr0Text += " (UncorrectedColor)";
            } else {
                corr0Text += " (手動)";
            }
            document.getElementById('displayCorrA').innerText = corr0Text;

            document.getElementById('displayColorB').innerText = `R:${state.led1_r} G:${state.led1_g} B:${state.led1_b}`;
            let corr1Text = `R:${state.led1_corr_r} G:${state.led1_corr_g} B:${state.led1_corr_b}`;
            if (state.led1_correction_type === "preset_typical") {
                corr1Text += " (TypicalLEDStrip)";
            } else if (state.led1_correction_type === "preset_uncorrected") {
                corr1Text += " (UncorrectedColor)";
            } else {
                corr1Text += " (手動)";
            }
            document.getElementById('displayCorrB').innerText = corr1Text;

            document.getElementById('displayGlobalBrightness').innerText = state.global_brightness;
        }

        // 頁面載入完成後獲取初始設定
        window.onload = fetchCurrentSettings;
    </script>
</body>
</html>
)rawliteral";

// --- 輔助函式：獲取當前燈珠狀態 (用於發送給網頁) ---
String getCurrentLedStateJson() {
  DynamicJsonDocument doc(512); // 根據數據量調整大小

  doc["led0_r"] = currentLed0_Color.r;
  doc["led0_g"] = currentLed0_Color.g;
  doc["led0_b"] = currentLed0_Color.b;
  doc["led0_correction_type"] = currentLed0_CorrectionType;
  doc["led0_corr_r"] = currentLed0_Correction.r;
  doc["led0_corr_g"] = currentLed0_Correction.g;
  doc["led0_corr_b"] = currentLed0_Correction.b;

  doc["led1_r"] = currentLed1_Color.r;
  doc["led1_g"] = currentLed1_Color.g;
  doc["led1_b"] = currentLed1_Color.b;
  doc["led1_correction_type"] = currentLed1_CorrectionType;
  doc["led1_corr_r"] = currentLed1_Correction.r;
  doc["led1_corr_g"] = currentLed1_Correction.g;
  doc["led1_corr_b"] = currentLed1_Correction.b;

  doc["global_brightness"] = currentGlobalBrightness;

  String output;
  serializeJson(doc, output);
  return output;
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n啟動 WS2812B 控制器...");

  // --- FastLED 初始化 ---
  // 為單一燈串添加 FastLED 實例
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  // 設定最大功耗限制 (5V, 120mA)，這是兩個燈珠在全亮狀態下的安全上限
  // FastLED 會自動調整亮度以確保總電流不超過此限制
  FastLED.setMaxPowerInVoltsAndMilliAmps(5, 120);
  Serial.println("FastLED 已初始化，最大功耗限制為 5V, 120mA");

  // 初始設定燈珠為黑色，亮度為預設值
  leds[0] = CRGB::Black; // 燈珠 0 初始為黑色
  leds[1] = CRGB::Black; // 燈珠 1 初始為黑色
  FastLED.setBrightness(currentGlobalBrightness);
  FastLED.show();

  // --- Wi-Fi AP 模式設定 ---
  Serial.print("設定 AP 模式 ");
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP 位址: ");
  Serial.println(IP);

  // --- 網頁伺服器路由設定 ---
  // 根路徑 "/" 返回 HTML 內容
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", HTML_CONTENT);
  });

  // "/submit" 路徑處理 POST 請求，用於更新燈珠設定
  server.on("/submit", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    String body = "";
    for (size_t i = 0; i < len; i++) {
      body += (char)data[i];
    }
    Serial.print("接收到 POST 請求數據: ");
    Serial.println(body);

    DynamicJsonDocument doc(512); // 根據預期 JSON 大小調整
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
      Serial.print(F("deserializeJson() 失敗: "));
      Serial.println(error.f_str());
      request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"無效的 JSON 數據\"}");
      return;
    }

    // --- 解析並儲存燈珠 0 的設定 (原燈珠 A) ---
    if (doc.containsKey("led0_r") && doc.containsKey("led0_g") && doc.containsKey("led0_b")) {
      currentLed0_Color.r = doc["led0_r"].as<uint8_t>();
      currentLed0_Color.g = doc["led0_g"].as<uint8_t>();
      currentLed0_Color.b = doc["led0_b"].as<uint8_t>();
      Serial.printf("儲存燈珠 0 顏色: R=%d G=%d B=%d\n", currentLed0_Color.r, currentLed0_Color.g, currentLed0_Color.b);
    }

    if (doc.containsKey("led0_correction_type")) {
      currentLed0_CorrectionType = doc["led0_correction_type"].as<String>();
      if (currentLed0_CorrectionType == "manual" && doc.containsKey("led0_corr_r") && doc.containsKey("led0_corr_g") && doc.containsKey("led0_corr_b")) {
        currentLed0_Correction.r = doc["led0_corr_r"].as<uint8_t>();
        currentLed0_Correction.g = doc["led0_corr_g"].as<uint8_t>();
        currentLed0_Correction.b = doc["led0_corr_b"].as<uint8_t>();
        Serial.printf("儲存燈珠 0 手動校正: R=%d G=%d B=%d\n", currentLed0_Correction.r, currentLed0_Correction.g, currentLed0_Correction.b);
      } else if (currentLed0_CorrectionType == "preset_typical") {
        currentLed0_Correction = TypicalLEDStrip; // 儲存 TypicalLEDStrip 的 CRGB 值
        Serial.println("儲存燈珠 0 預設校正: TypicalLEDStrip");
      } else if (currentLed0_CorrectionType == "preset_uncorrected") {
        currentLed0_Correction = UncorrectedColor; // 儲存 UncorrectedColor 的 CRGB 值
        Serial.println("儲存燈珠 0 預設校正: UncorrectedColor");
      }
    }

    // --- 解析並儲存燈珠 1 的設定 (原燈珠 B) ---
    if (doc.containsKey("led1_r") && doc.containsKey("led1_g") && doc.containsKey("led1_b")) {
      currentLed1_Color.r = doc["led1_r"].as<uint8_t>();
      currentLed1_Color.g = doc["led1_g"].as<uint8_t>();
      currentLed1_Color.b = doc["led1_b"].as<uint8_t>();
      Serial.printf("儲存燈珠 1 顏色: R=%d G=%d B=%d\n", currentLed1_Color.r, currentLed1_Color.g, currentLed1_Color.b);
    }

    if (doc.containsKey("led1_correction_type")) {
      currentLed1_CorrectionType = doc["led1_correction_type"].as<String>();
      if (currentLed1_CorrectionType == "manual" && doc.containsKey("led1_corr_r") && doc.containsKey("led1_corr_g") && doc.containsKey("led1_corr_b")) {
        currentLed1_Correction.r = doc["led1_corr_r"].as<uint8_t>();
        currentLed1_Correction.g = doc["led1_corr_g"].as<uint8_t>();
        currentLed1_Correction.b = doc["led1_corr_b"].as<uint8_t>();
        Serial.printf("儲存燈珠 1 手動校正: R=%d G=%d B=%d\n", currentLed1_Correction.r, currentLed1_Correction.g, currentLed1_Correction.b);
      } else if (currentLed1_CorrectionType == "preset_typical") {
        currentLed1_Correction = TypicalLEDStrip;
        Serial.println("儲存燈珠 1 預設校正: TypicalLEDStrip");
      } else if (currentLed1_CorrectionType == "preset_uncorrected") {
        currentLed1_Correction = UncorrectedColor;
        Serial.println("儲存燈珠 1 預設校正: UncorrectedColor");
      }
    }

    // --- 解析並應用全局亮度 ---
    if (doc.containsKey("global_brightness")) {
      currentGlobalBrightness = doc["global_brightness"].as<uint8_t>();
      FastLED.setBrightness(currentGlobalBrightness);
      Serial.printf("設定全局亮度: %d\n", currentGlobalBrightness);
    }

    // --- 在這裡應用每個燈珠的顏色和獨立校正 ---
    // 燈珠 0
    CRGB tempColor0 = currentLed0_Color;
    if (currentLed0_CorrectionType == "manual") {
        tempColor0.nscale8(currentLed0_Correction); // 手動 R,G,B 比例校正
    } else if (currentLed0_CorrectionType == "preset_typical") {
        tempColor0.nscale8(TypicalLEDStrip); // 套用 TypicalLEDStrip 校正
    } else if (currentLed0_CorrectionType == "preset_uncorrected") {
        // UncorrectedColor 是 CRGB(255,255,255)，nscale8(UncorrectedColor) 效果等同於不校正
        // tempColor0.nscale8(UncorrectedColor); // 可選，為了明確性
    }
    leds[0] = tempColor0; // 將校正後的顏色賦值給燈珠 0

    // 燈珠 1
    CRGB tempColor1 = currentLed1_Color;
    if (currentLed1_CorrectionType == "manual") {
        tempColor1.nscale8(currentLed1_Correction); // 手動 R,G,B 比例校正
    } else if (currentLed1_CorrectionType == "preset_typical") {
        tempColor1.nscale8(TypicalLEDStrip); // 套用 TypicalLEDStrip 校正
    } else if (currentLed1_CorrectionType == "preset_uncorrected") {
        // tempColor1.nscale8(UncorrectedColor); // 可選，為了明確性
    }
    leds[1] = tempColor1; // 將校正後的顏色賦值給燈珠 1

    // --- 更新燈珠顯示 ---
    FastLED.show();
    Serial.println("FastLED.show() 已執行。");

    // 返回當前狀態給網頁，用於更新顯示
    request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"設定已更新！\",\"current_state\":" + getCurrentLedStateJson() + "}");
  });

  // "/settings" 路徑處理 GET 請求，用於獲取當前設定
  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", getCurrentLedStateJson());
  });

  // 啟動伺服器
  server.begin();
  Serial.println("HTTP 伺服器已啟動。");
}

void loop() {
  // 在這裡可以放置其他非阻塞的任務。
  // AsyncWebServer 會在後台處理網絡請求，無需在 loop() 中頻繁呼叫 server.handleClient()。
}
