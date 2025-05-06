// PINS
// SDA -> 10
// SCK -> 12
// MOSI -> 11
// MISO -> 13
// IRQ -> NOTHING
// GND -> GND
// RST -> 0
// 3.3V -> 3.3V

// AIR TAG: hex = F7 36 E2 00, decimal = 247 54 226 00
// KEY CARD: hex = BF 94 26 1F, decimal = 191 148 38 31


#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 0
#define LED_PIN 47
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[4];
byte KEY_CARD_BYTES[4];

void setup() { 
  delay(2000);
  Serial0.begin(115200);
  /*while(!Serial0) {
    // Do nothing
    Serial0.print("NOT WORKING");
  }*/
  Serial0.println("Start!");
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  // Set key to KEY CARD
  KEY_CARD_BYTES[0] = 0xBF;
  KEY_CARD_BYTES[1] = 0x94;
  KEY_CARD_BYTES[2] = 0x26;
  KEY_CARD_BYTES[3] = 0x1F;
  //KEY_CARD_BYTES[]
  for (byte i = 0; i < 4; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial0.println(F("This code scan the MIFARE Classsic NUID."));
  Serial0.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);

  pinMode(LED_PIN, OUTPUT);
}
 
void loop() {

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial0.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial0.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  /*if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && 
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial0.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }*/

  if (rfid.uid.uidByte[0] != KEY_CARD_BYTES[0] || 
    rfid.uid.uidByte[1] != KEY_CARD_BYTES[1] || 
    rfid.uid.uidByte[2] != KEY_CARD_BYTES[2] || 
    rfid.uid.uidByte[3] != KEY_CARD_BYTES[3] ) {
    Serial0.println(F("Incorrect RFID"));
   
    Serial0.println(F("The NUID tag is:"));
    Serial0.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial0.println();
    Serial0.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial0.println();
  }
  else {
    Serial0.println(F("Correct RFID"));
    if(digitalRead(LED_PIN) == HIGH) {
      digitalWrite(LED_PIN, LOW);
    }
    else {
      digitalWrite(LED_PIN, HIGH);
    }
  }

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}


/**
 * Helper routine to dump a byte array as hex values to Serial0. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial0.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial0.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial0.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial0.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial0.print(buffer[i], DEC);
  }
}