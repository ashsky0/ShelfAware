// PINS
// SDA1 -> 10
// SDA2 -> 17
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

#define SS_PIN_1 10
#define SS_PIN_2 17
#define RST_PIN 0
#define LED_PIN 47
 
MFRC522 rfid1(SS_PIN_1, RST_PIN); // Instance of the class
MFRC522 rfid2(SS_PIN_2, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key1; 
MFRC522::MIFARE_Key key2; 

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
  rfid1.PCD_Init(); // Init MFRC522 
  rfid2.PCD_Init(); // Init MFRC522 

  // Set key to KEY CARD
  KEY_CARD_BYTES[0] = 0xBF;
  KEY_CARD_BYTES[1] = 0x94;
  KEY_CARD_BYTES[2] = 0x26;
  KEY_CARD_BYTES[3] = 0x1F;
  //KEY_CARD_BYTES[]
  for (byte i = 0; i < 4; i++) {
    key1.keyByte[i] = 0xFF;
    key2.keyByte[i] = 0xFF;
  }

  Serial0.println(F("This code scan the MIFARE Classsic NUID."));
  Serial0.print(F("Using the following key:"));
  printHex(key1.keyByte, MFRC522::MF_KEY_SIZE);
  printHex(key2.keyByte, MFRC522::MF_KEY_SIZE);

  pinMode(LED_PIN, OUTPUT);
}
 
void loop() {

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! (rfid1.PICC_IsNewCardPresent() | rfid2.PICC_IsNewCardPresent()))
    return;

  // Verify if the NUID has been readed
  if ( ! (rfid1.PICC_ReadCardSerial() | rfid2.PICC_ReadCardSerial()))
    return;


  // Print out data
  Serial0.println(F("The 1st NUID tag is:"));
  Serial0.print(F("In hex: "));
  printHex(rfid1.uid.uidByte, rfid1.uid.size);
  Serial0.println();
  Serial0.print(F("In dec: "));
  printDec(rfid1.uid.uidByte, rfid1.uid.size);
  Serial0.println();

  Serial0.println(F("The 2nd NUID tag is:"));
  Serial0.print(F("In hex: "));
  printHex(rfid2.uid.uidByte, rfid2.uid.size);
  Serial0.println();
  Serial0.print(F("In dec: "));
  printDec(rfid2.uid.uidByte, rfid2.uid.size);
  Serial0.println();

  // Halt PICC
  rfid1.PICC_HaltA();
  rfid2.PICC_HaltA();

  // Stop encryption on PCD
  rfid1.PCD_StopCrypto1();
  rfid2.PCD_StopCrypto1();
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