/* Make elips flower glow project.
 * Control chip code ver 0.0.1
 * ESP32
 */

#include <FastLED.h>

// flower led section
#define NUM_LEDS_FLOWER 2
#define FLOWER_LED_PIN 27
CRGB flowerLED[NUM_LEDS_FLOWER];
CRGB rainbowColor[7] = {CRGB::Red, CRGB::Orange, CRGB::Yellow, CRGB::Green, CRGB::Aqua, CRGB::Blue, CRGB::Purple};

//flow control
/* disable temp, maybe usefull
int refreshRate = 250;
unsigned int previousMillis = 0;
#define MICROSECOND 1e6
*/
bool lampIsChanged = false;

void setup() {
	
   	Serial.begin(9600);
	
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);	

	//flowerLED section
	FastLED.addLeds<WS2812B, FLOWER_LED_PIN, GRB>(flowerLED, NUM_LEDS_FLOWER);
	FastLED.setBrightness(64);//0~255
	FastLED.clear(true);
	FastLED.clear(true);
	
	Serial.println("Setup Complete."); 
}

void updateFlowerLamp(){
// nothing now
}

void setFlowerLampOff(){
	FastLED.clear(true);
	FastLED.clear(true);
	
	digitalWrite(LED_BUILTIN, HIGH);	
//	Serial.println("Lamp black");
}

void setFlowerLampRed(byte ledPosition){
	flowerLED[ledPosition] = CRGB::Red;
	FastLED.show();
	FastLED.show();
	
	digitalWrite(LED_BUILTIN, LOW);	
//	Serial.println("Lamp red");
}

void setFlowerLampWorstCase(){
	
	FastLED.setBrightness(255);
	FastLED.showColor(CRGB::White);
	
	digitalWrite(LED_BUILTIN, LOW);	
//	Serial.println("Lamp red");
}

void rainbowbBreath(int breath = 100){
	for(int step = 0; step < 7; step++){
		for(int brightnessStep = 0; brightnessStep < 64; brightnessStep++){
			FastLED.showColor(rainbowColor[step], (brightnessStep + 1) );
			delay(breath);
		}
		for(int brightnessStep = 63; brightnessStep > 0; brightnessStep--){
			FastLED.showColor(rainbowColor[step], (brightnessStep + 1) );
			delay(breath);
		}
	}
}

void loop() {
	rainbowbBreath(125);
}
