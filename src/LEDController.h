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
  CRGB currentLower;
  CRGB currentUpper;
  unsigned long fadeStartTime = 0;
  unsigned long fadeDuration = 0;
  
public:
  LEDController(CRGB* ledArray, int count) : leds(ledArray), numLeds(count) {
    currentLower = CRGB::Black;
    currentUpper = CRGB::Black;
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
      currentLower = lower;
      currentUpper = upper;
      isFading = false;
    }
  }
  
  void startFade(CRGB lower, CRGB upper) {
    targetLower = lower;
    targetUpper = upper;
    fadeStartTime = millis();
    // 使用 FastLED 的 random16 產生 3000~5000ms 的隨機時間
    fadeDuration = 3000 + random16(2001); // 3000 + (0~2000)
    isFading = true;
    
    Serial.print(F("開始漸變，持續時間: "));
    Serial.print(fadeDuration);
    Serial.println(F("ms"));
  }
  
  void update() {
    // 使用 FastLED 的 EVERY_N_MILLISECONDS 來更新漸變
    EVERY_N_MILLISECONDS(20) {
      if (isFading) {
        unsigned long elapsed = millis() - fadeStartTime;
        
        if (elapsed >= fadeDuration) {
          // 漸變完成
          currentLower = targetLower;
          currentUpper = targetUpper;
          isFading = false;
          
          if (numLeds >= 2) {
            leds[0] = currentLower;
            leds[1] = currentUpper;
            FastLED.show();
          }
          Serial.println(F("漸變完成"));
        } else {
          // 計算漸變進度 (0~255)
          uint8_t progress = (elapsed * 255) / fadeDuration;
          
          // 使用 FastLED 的 blend 函數進行顏色混合
          currentLower = blend(currentLower, targetLower, progress);
          currentUpper = blend(currentUpper, targetUpper, progress);
          
          if (numLeds >= 2) {
            leds[0] = currentLower;
            leds[1] = currentUpper;
            FastLED.show();
          }
        }
      }
    }
  }
  
  bool isFadingNow() { return isFading; }
};

#endif