#include <SPI.h>
#include <MFRC522.h>
#include <HX711.h>
#include <WiFi.h>
#include <HTTPClient.h>

// WiFi Setup
const char* ssid = "ShelfAware";
const char* password = "icecoldengineering";
const char* itemEventApiUrl = "https://shelfaware-app.fly.dev/api/item-event";

// Scale Setup
#define DOUT  1
#define SCK   2
HX711 scale;
#define SCALE_COEFF 338.310394
#define SCALE_OFFSET 1412837

// RFID Setup
#define RST_PIN 0
const uint8_t ssPins[8] = {4, 5, 6, 7, 15, 16, 17, 18};
MFRC522 rfids[8] = {
  {4, RST_PIN}, {5, RST_PIN}, {6, RST_PIN}, {7, RST_PIN},
  {15, RST_PIN}, {16, RST_PIN}, {17, RST_PIN}, {18, RST_PIN}
};

String previousTags[8];  // Store last tag seen by each reader
long previousWeight = 0;
long previousSentWeight = 0;
const long weightThreshold = 5;  // Minimum change to consider

void setup() {
  delay(2000);
  Serial0.begin(115200);
  SPI.begin();

  // Initialize RFID readers
  for (int i = 0; i < 8; i++) {
    rfids[i].PCD_Init();
    delay(50);
  }

  // Initialize scale
  scale.begin(DOUT, SCK);
  scale.set_scale(SCALE_COEFF);
  scale.set_offset(SCALE_OFFSET);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial0.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial0.print(".");
  }
  Serial0.println("\nWiFi connected");
}

void loop() {
  long currentWeight = scale.get_units();
  Serial0.println(currentWeight);
  long weightDelta = currentWeight - previousWeight;

  String currentTags[8];
  for (int i = 0; i < 8; i++) {
    currentTags[i] = scanRFID(rfids[i]);
  }

  for (int i = 0; i < 8; i++) {
    String oldTag = previousTags[i];
    String newTag = currentTags[i];

    // Regrab weight
    long currentWeight = scale.get_units();

    if (oldTag == "" && newTag != "") {
      // Tag added
      sendEvent("in", newTag.c_str(), currentWeight);
      Serial0.println("Item Added");
    } else if (oldTag != "" && newTag == "") {
      // Tag removed
      sendEvent("out", oldTag.c_str(), currentWeight);
      Serial0.println("Item Removed");
    } else if (oldTag != "" && newTag != "" && oldTag != newTag) {
      // Tag changed
      sendEvent("moved", newTag.c_str(), currentWeight);
      Serial0.println("Item Moved");
    }
  }

  // Update previous states
  memcpy(previousTags, currentTags, sizeof(currentTags));
  previousWeight = currentWeight;

  delay(300);
}

// Returns UID as hex string or empty string if no tag
String scanRFID(MFRC522& reader) {
  uint8_t atqa[2], sz = sizeof(atqa);
  MFRC522::Uid mfrc522_uid;
  
  if (reader.PICC_IsNewCardPresent() && reader.PICC_ReadCardSerial()) {
    String uid = "";
    Serial0.println("Scanning RFID");
    for (byte i = 0; i < reader.uid.size; i++) {
      if (reader.uid.uidByte[i] < 0x10) uid += "0";
      uid += String(reader.uid.uidByte[i], HEX);
    }
    reader.PICC_HaltA();
    reader.PCD_StopCrypto1();
    return uid;
  }
  else if (reader.PICC_WakeupA(atqa, &sz) == MFRC522::STATUS_OK) {
    if(reader.PICC_Select(&mfrc522_uid) == MFRC522::STATUS_OK && reader.PICC_ReadCardSerial() == MFRC522::STATUS_OK) {
      String uid = "";
      Serial0.println("Re-scanning RFID");
      for (byte i = 0; i < reader.uid.size; i++) {
        if (reader.uid.uidByte[i] < 0x10) uid += "0";
        uid += String(reader.uid.uidByte[i], HEX);
      }
      reader.PICC_HaltA();
      reader.PCD_StopCrypto1();
      return uid;
    }
    return "";
  }
  return "";
}

void sendEvent(const char* eventType, const char* tag, long weight) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial0.println("WiFi not connected");
    return;
  }

  HTTPClient http;
  http.begin(itemEventApiUrl);
  http.addHeader("Content-Type", "application/json");

  String json = String("{\"eventType\":\"") + eventType + "\",\"uid\":\"" + tag + "\",\"weight\":" + weight + "}";
  Serial0.println("POST: " + json);

  int code = http.POST(json);
  if (code > 0) {
    String response = http.getString();
    Serial0.printf("Response [%d]: %s\n", code, response.c_str());
  } else {
    Serial0.printf("POST failed: %s\n", http.errorToString(code).c_str());
  }

  http.end();
}
