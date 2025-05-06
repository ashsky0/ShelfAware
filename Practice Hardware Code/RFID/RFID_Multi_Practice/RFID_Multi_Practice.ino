// Filename: RFID_Multi_Practice
// Author: Ashley Guillard
// Date: May 2, 2025
// Components:
//             - (x1) Expressif ESP32-S3 
//             - (x6) MFRC522
// Desciption:
//             This code scnas form 6 different RC522 RFID readers using a daisy chain method.


// -------------------- Pin Configuration -------------------- //
// Data pins:                                                  //
//             SDA1 -> 6,     SDA2 -> 7,      SDA3 -> 15,      //
//             SDA4 -> 16,    SDA5 -> 17,     SDA6 -> 18       //
// Clock pin:                                                  //
//             SCK -> 12                                       //
// SPI pins:                                                   //
//             MOSI -> 11,    MISO -> 13                       //
// Interrupt pin:                                              //
//                IRQ -> NOTHING                               //
// Reset pin:                                                  //
//             RST -> 0                                        //
// Gound pin:                                                  //
//             GND -> G                                        //
// Power pin:                                                  //
//             3.3V -> 3.3V                                    //
// ----------------------------------------------------------- //

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN_1 6
#define SS_PIN_2 7
#define SS_PIN_3 15
#define SS_PIN_4 16
#define SS_PIN_5 17
#define SS_PIN_6 18
#define RST_PIN 0
 
MFRC522 rfid1(SS_PIN_1, RST_PIN); // Instance of the class
MFRC522 rfid2(SS_PIN_2, RST_PIN); // Instance of the class
MFRC522 rfid3(SS_PIN_3, RST_PIN); // Instance of the class
MFRC522 rfid4(SS_PIN_4, RST_PIN); // Instance of the class
MFRC522 rfid5(SS_PIN_5, RST_PIN); // Instance of the class
MFRC522 rfid6(SS_PIN_6, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key1; 
MFRC522::MIFARE_Key key2; 
MFRC522::MIFARE_Key key3; 
MFRC522::MIFARE_Key key4; 
MFRC522::MIFARE_Key key5; 
MFRC522::MIFARE_Key key6;

// Init array that will store new NUID 
byte nuidPICC[4];

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
  rfid3.PCD_Init(); // Init MFRC522 
  rfid4.PCD_Init(); // Init MFRC522 
  rfid5.PCD_Init(); // Init MFRC522 
  rfid6.PCD_Init(); // Init MFRC522 

  // Reader NUIDs
  for (byte i = 0; i < 6; i++) {
    key1.keyByte[i] = 0xFF;
    key2.keyByte[i] = 0xFF;
    key3.keyByte[i] = 0xFF;
    key4.keyByte[i] = 0xFF;
    key5.keyByte[i] = 0xFF;
    key6.keyByte[i] = 0xFF;
  }

  Serial0.println(F("This code scan the MIFARE NUID (12 bytes)."));
  Serial0.print(F("Using the following key:"));
  printHex(key1.keyByte, MFRC522::MF_KEY_SIZE);
  printHex(key2.keyByte, MFRC522::MF_KEY_SIZE);
  printHex(key3.keyByte, MFRC522::MF_KEY_SIZE);
  printHex(key4.keyByte, MFRC522::MF_KEY_SIZE);
  printHex(key5.keyByte, MFRC522::MF_KEY_SIZE);
  printHex(key6.keyByte, MFRC522::MF_KEY_SIZE);
  Serial0.println("\n"); // New line
}
 
void loop() {

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  // Verify if the NUID has been readed
  handleRFID(rfid1, F("The 1st NUID tag is:"));
  handleRFID(rfid2, F("The 2nd NUID tag is:"));
  handleRFID(rfid3, F("The 3rd NUID tag is:"));
  handleRFID(rfid4, F("The 4th NUID tag is:"));
  handleRFID(rfid5, F("The 5th NUID tag is:"));
  handleRFID(rfid6, F("The 6th NUID tag is:"));
}

/**
 * Helper function to read from each RFID
 */
void handleRFID(MFRC522 &rfid, const __FlashStringHelper* label) {
  if (rfid.PICC_IsNewCardPresent()) {
    if (rfid.PICC_ReadCardSerial()) {

      // Print information
      Serial0.println(label);
      Serial0.print(F("In hex: "));
      printHex(rfid.uid.uidByte, rfid.uid.size);
      Serial0.println();
      Serial0.print(F("In dec: "));
      printDec(rfid.uid.uidByte, rfid.uid.size);
      Serial0.println("\n");

    } 
  }
  rfid.PICC_HaltA(); // Halt PICC (to read stop reading this card)
  rfid.PCD_StopCrypto1(); // Stop encryption on PCD (to read new cards)
  return;
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