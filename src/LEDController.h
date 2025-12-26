#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <FastLED.h>

class LEDController {
private:
  CRGB* leds;
  int numLeds;
  
  // 顏色狀態（優化為 4 個變數）
  CRGB currentLower;  // 當前顏色（邏輯顏色，滿亮度）
  CRGB currentUpper;  // 當前顏色（邏輯顏色，滿亮度）
  CRGB targetLower;   // 漸變目標顏色
  CRGB targetUpper;   // 漸變目標顏色
  
  // 時間控制
  bool isFading = false;
  unsigned long fadeStartTime = 0;
  unsigned long fadeDuration = 0;
  unsigned long breathStartTime = 0;
  
public:
  LEDController(CRGB* ledArray, int count) : leds(ledArray), numLeds(count) {
    currentLower = CRGB::Black;
    currentUpper = CRGB::Black;
    breathStartTime = millis();
  }
  
  void init(int brightness = 128) {
    FastLED.setBrightness(brightness);
    FastLED.clear();
    FastLED.show();
  }
  
  void setColorImmediate(CRGB lower, CRGB upper) {
    if (numLeds >= 2) {
      currentLower = lower;
      currentUpper = upper;
      leds[0] = lower;
      leds[1] = upper;
      FastLED.show();
      isFading = false;
    }
  }
  
  void startFade(CRGB lower, CRGB upper) {
    // 設定漸變目標（起點就是當前的 currentLower/Upper）
    targetLower = lower;
    targetUpper = upper;
    
    // 設定漸變參數
    fadeStartTime = millis();
    fadeDuration = 2000 + random16(1001); // 2000~3000ms
    isFading = true;
    
    Serial.print(F("開始漸變，持續時間: "));
    Serial.print(fadeDuration);
    Serial.println(F("ms"));
    Serial.print(F("從 RGB("));
    Serial.print(currentLower.r); Serial.print(F(","));
    Serial.print(currentLower.g); Serial.print(F(","));
    Serial.print(currentLower.b); Serial.print(F(") 到 RGB("));
    Serial.print(targetLower.r); Serial.print(F(","));
    Serial.print(targetLower.g); Serial.print(F(","));
    Serial.print(targetLower.b); Serial.println(F(")"));
  }
  
  void update() {
    // 使用 FastLED 的 EVERY_N_MILLISECONDS 來更新
    EVERY_N_MILLISECONDS(20) {
      if (isFading) {
        // === 顏色漸變模式 ===
        unsigned long elapsed = millis() - fadeStartTime;
        
        if (elapsed >= fadeDuration) {
          // 漸變完成，更新當前顏色
          currentLower = targetLower;
          currentUpper = targetUpper;
          leds[0] = currentLower;
          leds[1] = currentUpper;
          isFading = false;
          breathStartTime = millis(); // 重置呼吸燈計時
          FastLED.show();
          
          Serial.println(F("漸變完成，開始呼吸燈效果"));
        } else {
          // 計算漸變進度 (0~255)
          uint8_t progress = (elapsed * 255) / fadeDuration;
          
          // 從當前顏色漸變到目標顏色
          leds[0] = blend(currentLower, targetLower, progress);
          leds[1] = blend(currentUpper, targetUpper, progress);
          FastLED.show();
          
          // 除錯訊息（每0.5秒輸出一次）
          if (elapsed % 500 < 20) {
            Serial.print(F("漸變進度: "));
            Serial.print((elapsed * 100) / fadeDuration);
            Serial.println(F("%"));
          }
        }
      } else {
        // === 呼吸燈模式 ===
        unsigned long elapsed = millis() - breathStartTime;
        
        // 使用正弦波計算呼吸效果，週期 2500ms
        // sin 值從 -1 到 1，映射到亮度 50%~100%
        float angle = (elapsed % 2500) * 2.0 * PI / 2500.0;
        float breath = (sin(angle) + 1.0) / 2.0; // 0.0 ~ 1.0
        uint8_t brightness = 128 + (127 * breath); // 128 ~ 255 (50%~100%)
        
        // 應用呼吸效果到當前顏色（不修改 currentLower/Upper）
        leds[0] = currentLower;
        leds[1] = currentUpper;
        leds[0].nscale8(brightness);
        leds[1].nscale8(brightness);
        
        FastLED.show();
      }
    }
  }
  
  bool isFadingNow() { return isFading; }
};

#endif