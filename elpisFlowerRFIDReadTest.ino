/* Make elips flower glow project.
 * RFID control code ver 0.0.0
 */
 
#include "MFRC522.h"
#include "ArduinoJson.h"

#define RST_PIN 21 //fixed RST_PIN 
#define SS_PIN 22 //SSorSDA pin for RC522 selection

void RFID_ReadID(MFRC522*, byte []);
void dump_byte_array(byte*, byte);
MFRC522 mfrc522Module(SS_PIN, RST_PIN);  // Create MFRC522 instance
MFRC522::StatusCode RFID_status;
byte    blockaddr = 0x06 ;

void setup()
{
  Serial.begin(9600);

  // Setup RC522
  	while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	SPI.begin();			// Init SPI bus
	mfrc522Module.PCD_Init();		// Init MFRC522
	delay(4);				// Optional delay. Some board do need more time after init to be ready, see Readme
	mfrc522Module.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
	Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

  
}


void rfidReadSection(){

  static byte read_buffer[18];
  
  
		
		// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if ( ! mfrc522Module.PICC_IsNewCardPresent()) {
			continue;
		}

		// Select one of the cards
	if ( ! mfrc522Module.PICC_ReadCardSerial()) {
			continue;
		}


		//String payload;
		RFID_ReadID(mfrc522Module, read_buffer);
		
		String bufferDump = read_buffer;

				// Halt PICC
		mfrc522Module.PICC_HaltA();
		// Stop encryption on PCD
		mfrc522Module.PCD_StopCrypto1();
				
		mfrc522Module.PCD_SoftPowerDown();

}

void loop() {
	rfidReadSection();
}

void RFID_ReadID(MFRC522 mfrc522Module, byte read_buffer[]) {
  byte size = 18;
  RFID_status = (MFRC522::StatusCode) mfrc522Module->MIFARE_Read(blockaddr, read_buffer, &size);
  if (RFID_status != MFRC522::STATUS_OK) {
    Serial.print("Read failed! Reason:");
    Serial.println(mfrc522Module->GetStatusCodeName(RFID_status));
  }
  Serial.println("Read Sector 1 Value in char:");
  dump_byte_array(read_buffer, 16); Serial.println();
}

void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.write(buffer[i]);
  }
}

