#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <FastLED.h>

class LEDController {
private:
  CRGB* leds;
  int numLeds;
  
  // 漸變狀態
  bool isFading = false;
  CRGB targetLower;
  CRGB targetUpper;
  CRGB startLower;   // 新增：記錄起始顏色
  CRGB startUpper;   // 新增：記錄起始顏色
  unsigned long fadeStartTime = 0;
  unsigned long fadeDuration = 0;
  
public:
  LEDController(CRGB* ledArray, int count) : leds(ledArray), numLeds(count) {
    startLower = CRGB::Black;
    startUpper = CRGB::Black;
  }
  
  void init(int brightness = 128) {
    FastLED.setBrightness(brightness);
    FastLED.clear();
    FastLED.show();
  }
  
  void setColorImmediate(CRGB lower, CRGB upper) {
    if (numLeds >= 2) {
      leds[0] = lower;
      leds[1] = upper;
      FastLED.show();
      startLower = lower;
      startUpper = upper;
      isFading = false;
    }
  }
  
  void startFade(CRGB lower, CRGB upper) {
    // 記錄起始顏色（當前顏色）
    startLower = leds[0];
    startUpper = leds[1];
    
    // 設定目標顏色
    targetLower = lower;
    targetUpper = upper;
    
    // 設定漸變參數
    fadeStartTime = millis();
    fadeDuration = 3000 + random16(2001); // 3000~5000ms
    isFading = true;
    
    Serial.print(F("開始漸變，持續時間: "));
    Serial.print(fadeDuration);
    Serial.println(F("ms"));
    Serial.print(F("從 RGB("));
    Serial.print(startLower.r); Serial.print(F(","));
    Serial.print(startLower.g); Serial.print(F(","));
    Serial.print(startLower.b); Serial.print(F(") 到 RGB("));
    Serial.print(targetLower.r); Serial.print(F(","));
    Serial.print(targetLower.g); Serial.print(F(","));
    Serial.print(targetLower.b); Serial.println(F(")"));
  }
  
  void update() {
    // 使用 FastLED 的 EVERY_N_MILLISECONDS 來更新漸變
    EVERY_N_MILLISECONDS(20) {
      if (isFading) {
        unsigned long elapsed = millis() - fadeStartTime;
        
        if (elapsed >= fadeDuration) {
          // 漸變完成
          leds[0] = targetLower;
          leds[1] = targetUpper;
          isFading = false;
          FastLED.show();
          
          Serial.println(F("漸變完成"));
        } else {
          // 計算漸變進度 (0~255)
          uint8_t progress = (elapsed * 255) / fadeDuration;
          
          // 從起始顏色漸變到目標顏色
          leds[0] = blend(startLower, targetLower, progress);
          leds[1] = blend(startUpper, targetUpper, progress);
          FastLED.show();
          
          // 除錯訊息（每秒輸出一次）
          if (elapsed % 1000 < 20) {
            Serial.print(F("漸變進度: "));
            Serial.print((elapsed * 100) / fadeDuration);
            Serial.println(F("%"));
          }
        }
      }
    }
  }
  
  bool isFadingNow() { return isFading; }
};

#endif