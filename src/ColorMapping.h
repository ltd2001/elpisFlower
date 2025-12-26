#ifndef COLOR_MAPPING_H
#define COLOR_MAPPING_H

#include <LittleFS.h>
#include <ArduinoJson.h>
#include <FastLED.h>

class ColorMapping {
private:
  struct ColorPair {
    CRGB lower;
    CRGB upper;
  };
  
  static const int MAX_LABELS = 20;
  String labels[MAX_LABELS];
  ColorPair colors[MAX_LABELS];
  int labelCount = 0;
  
  CRGB parseColor(JsonArray arr) {
    if (arr.size() >= 3) {
      return CRGB(arr[0], arr[1], arr[2]);
    }
    return CRGB::Black;
  }

public:
  bool loadFromFile(const char* filename = "/colors.json") {
    File file = LittleFS.open(filename, "r");
    if (!file) {
      Serial.println(F("無法開啟 colors.json"));
      return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
      Serial.print(F("JSON 解析失敗: "));
      Serial.println(error.c_str());
      return false;
    }

    labelCount = 0;
    JsonArray mappings = doc["mappings"].as<JsonArray>();
    
    for (JsonObject mapping : mappings) {
      if (labelCount >= MAX_LABELS) break;
      
      String label = mapping["label"].as<String>();
      JsonArray lower = mapping["lower"].as<JsonArray>();
      JsonArray upper = mapping["upper"].as<JsonArray>();
      
      labels[labelCount] = label;
      colors[labelCount].lower = parseColor(lower);
      colors[labelCount].upper = parseColor(upper);
      labelCount++;
      
      Serial.print(F("載入: "));
      Serial.print(label);
      Serial.print(F(" -> RGB("));
      Serial.print(colors[labelCount-1].lower.r);
      Serial.print(F(","));
      Serial.print(colors[labelCount-1].lower.g);
      Serial.print(F(","));
      Serial.print(colors[labelCount-1].lower.b);
      Serial.println(F(")"));
    }
    
    Serial.print(F("共載入 "));
    Serial.print(labelCount);
    Serial.println(F(" 組顏色對應"));
    return true;
  }

  bool getColors(String label, CRGB &lower, CRGB &upper) {
    for (int i = 0; i < labelCount; i++) {
      if (labels[i] == label) {
        lower = colors[i].lower;
        upper = colors[i].upper;
        return true;
      }
    }
    // 找不到時回傳黑色
    lower = CRGB::Black;
    upper = CRGB::Black;
    return false;
  }
  
  int getCount() { return labelCount; }
  
  String getLabel(int index) {
    if (index >= 0 && index < labelCount) {
      return labels[index];
    }
    return "";
  }
};

#endif