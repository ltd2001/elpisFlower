/* Make elips flower glow project.
 * Control chip code ver 0.0.1
 */

#include <FastLED.h>

// flower led section
#define NUM_LEDS_FLOWER 2
#define FLOWER_LED_PIN D1
CRGB flowerLED[NUM_LEDS_FLOWER];

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

void loop() {
	setFlowerLampOff();
	delay(250);
	setFlowerLampRed(0);
	delay(250);
	setFlowerLampOff();
	delay(250);
	setFlowerLampRed(1);
	delay(250);
	setFlowerLampRed(0);
	delay(2000);
}
