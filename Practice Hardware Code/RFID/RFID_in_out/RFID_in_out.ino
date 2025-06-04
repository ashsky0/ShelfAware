#define SHELFAWARE_DEBUG 1
#define USE_MFRC522_V2 1

#include <stdint.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <SPI.h>
#if USE_MFRC522_V2
#include <MFRC522v2.h>
#include <MFRC522Constants.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#else
#include <MFRC522.h>
#endif

#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a > b) ? a : b)

#define MAX_UID_SIZE 14
#define STABLE_N 5

#define LED_PIN 0

struct UID {
  uint8_t size;
  uint8_t data[MAX_UID_SIZE];
};

struct Pos {
  uint8_t plate;
  uint8_t row;
  uint8_t col;
};

// --- RFID --- //
#define RFID_N_READERS 8

#define SS_PIN_0 4
#define SS_PIN_1 5
#define SS_PIN_2 6
#define SS_PIN_3 7
#define SS_PIN_4 15
#define SS_PIN_5 16
#define SS_PIN_6 17
#define SS_PIN_7 18
#if !USE_MFRC522_V2
// #define RST_PIN 0
#endif

#if SHELFAWARE_DEBUG
static const uint8_t ssPins[RFID_N_READERS] = {
  SS_PIN_0, SS_PIN_1, SS_PIN_2, SS_PIN_3,
  SS_PIN_4, SS_PIN_5, SS_PIN_6, SS_PIN_7
};
#endif

struct RfidReaderInfo {
  bool present;
  uint64_t n_stable;
  struct UID uid;
  struct UID tmp_uid;
};

#if USE_MFRC522_V2
static MFRC522DriverPinSimple ssDrivers[RFID_N_READERS] = {
  { SS_PIN_0 },
  { SS_PIN_1 },
  { SS_PIN_2 },
  { SS_PIN_3 },
  { SS_PIN_4 },
  { SS_PIN_5 },
  { SS_PIN_6 },
  { SS_PIN_7 }
};
static MFRC522DriverSPI spiDrivers[RFID_N_READERS] = {
  { ssDrivers[0] },
  { ssDrivers[1] },
  { ssDrivers[2] },
  { ssDrivers[3] },
  { ssDrivers[4] },
  { ssDrivers[5] },
  { ssDrivers[6] },
  { ssDrivers[7] }
};
static MFRC522 rfids[RFID_N_READERS] = {
  { spiDrivers[0] },
  { spiDrivers[1] },
  { spiDrivers[2] },
  { spiDrivers[3] },
  { spiDrivers[4] },
  { spiDrivers[5] },
  { spiDrivers[6] },
  { spiDrivers[7] }
};
#else
// Array of MFRC522 instances
static MFRC522 rfids[RFID_N_READERS] = {
  { SS_PIN_0, RST_PIN }, 
  { SS_PIN_1, RST_PIN }, 
  { SS_PIN_2, RST_PIN }, 
  { SS_PIN_3, RST_PIN }, 
  { SS_PIN_4, RST_PIN }, 
  { SS_PIN_5, RST_PIN }, 
  { SS_PIN_6, RST_PIN }, 
  { SS_PIN_7, RST_PIN }
};
#endif

static struct RfidReaderInfo readers[RFID_N_READERS];
static const struct Pos positions[RFID_N_READERS] = {
  {0, 0, 0},
  {0, 0, 1},
  {0, 1, 0},
  {0, 1, 1},
  {1, 0, 0},
  {1, 0, 1},
  {1, 1, 0},
  {1, 1, 1}
};

void RFID_init(void);
void RFID_loop(void);

static void setup_RFIDs(void);
static void handleRFID(MFRC522 &rfid, struct RfidReaderInfo *info, const struct Pos *pos, uint8_t idx);
static void onTagDetected(MFRC522 &rfid, struct RfidReaderInfo *info, const struct Pos *pos, uint8_t idx);
static void onTagRemoved(MFRC522 &rfid, struct RfidReaderInfo *info, const struct Pos *pos, uint8_t idx);
static void onTagReplaced(MFRC522 &rfid, struct RfidReaderInfo *info, const struct Pos *pos, uint8_t idx);
static void onTagIdle(struct RfidReaderInfo *info, const struct Pos *pos, uint8_t idx);
static void onTagReady(struct RfidReaderInfo *info, const struct Pos *pos, uint8_t idx);
static void onTagEmpty(struct RfidReaderInfo *info, const struct Pos *pos, uint8_t idx);
static void onTagReadyStable(struct RfidReaderInfo *info, const struct Pos *pos, uint8_t idx);
static void onTagEmptyStable(struct RfidReaderInfo *info, const struct Pos *pos, uint8_t idx);

void bytesToHexStr(char *str, size_t len, const uint8_t *bytes, size_t n_bytes);

void RFID_init() {
  setup_RFIDs();
  // xTaskCreatePinnedToCore(
  //   Task_RFID, "Task_RFID", 4096, NULL, 2, NULL, 1
  // );
#if SHELFAWARE_DEBUG
  Serial.printf("RFID module initialized.\n\n");
#endif
}

static void setup_RFIDs() {
  SPI.begin();  // Init SPI bus
  for (uint8_t i = 0; i < RFID_N_READERS; i++) {
    rfids[i].PCD_Init();  // Init each MFRC522
  }
}

void RFID_loop() {
  for (uint8_t i = 0; i < RFID_N_READERS; i++) {
    handleRFID(rfids[i], &readers[i], &positions[i], i);
  }
  vTaskDelay(pdMS_TO_TICKS(100));
}

static void handleRFID(
  MFRC522 &rfid,
  struct RfidReaderInfo *info,
  const struct Pos *pos,
  uint8_t idx
) {
  // Answer To Request, Type A
  uint8_t atqa[2], sz = sizeof(atqa);

  if (!info->present) {
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      // New card tapped after long time
      onTagDetected(rfid, info, pos, idx);

      // release for next detection
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    } else {
      // no card still...
      onTagEmpty(info, pos, idx);
    }
  } else {  // A card was present last time this reader is handled
    if (
#if USE_MFRC522_V2
      rfid.PICC_WakeupA(atqa, &sz) == MFRC522Constants::STATUS_OK
#else
      rfid.PICC_WakeupA(atqa, &sz) == MFRC522::STATUS_OK
#endif
    ) {

      MFRC522::Uid mfrc522_uid;
      if (
#if USE_MFRC522_V2
        rfid.PICC_Select(&mfrc522_uid) == MFRC522Constants::STATUS_OK
        && rfid.PICC_ReadCardSerial() == MFRC522Constants::STATUS_OK
#else
        rfid.PICC_Select(&mfrc522_uid) == MFRC522::STATUS_OK
        && rfid.PICC_ReadCardSerial() == MFRC522::STATUS_OK
#endif
      ) {
        if (
          rfid.uid.size == info->uid.size
          && memcmp(rfid.uid.uidByte, info->uid.data, rfid.uid.size) == 0
        ) {
          // The same card is still there
          onTagIdle(info, pos, idx);
        } else if (
          rfid.uid.size == info->tmp_uid.size
          && memcmp(rfid.uid.uidByte, info->tmp_uid.data, rfid.uid.size) == 0
        ) {
          // The recently detected card is still there
          onTagReady(info, pos, idx);
        } else {
#if SHELFAWARE_DEBUG
          Serial.printf("Spooky stuff, the awaken card is neither the recently detected one nor the confirmed one.\n\n");
#endif
        }
      }

      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();

    } else if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {

      if (
        rfid.uid.size == info->uid.size
        && memcmp(rfid.uid.uidByte, info->uid.data, rfid.uid.size) == 0
      ) {
        // The same card is still there, the reader needs caffeine
        onTagIdle(info, pos, idx);
      } else if (
        rfid.uid.size == info->tmp_uid.size
        && memcmp(rfid.uid.uidByte, info->tmp_uid.data, rfid.uid.size) == 0
      ) {
        // The recently detected card is still there
        onTagReady(info, pos, idx);
      } else {
        // User is faster than The Flash or our scheduling can't keep up
        onTagReplaced(rfid, info, pos, idx);
      }

      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();

    } else {
      // A card got removed
      onTagRemoved(rfid, info, pos, idx);
    }
  }
}

static void onTagDetected(
  MFRC522 &rfid,
  struct RfidReaderInfo *info,
  const struct Pos *pos,
  uint8_t idx
) {
  info->present = true;
  info->n_stable = 0;
  info->tmp_uid.size = rfid.uid.size;
  memcpy(info->tmp_uid.data, rfid.uid.uidByte, rfid.uid.size);

#if SHELFAWARE_DEBUG
  char uid_str[MAX_UID_SIZE * 2 + 1] = { 0 };
  bytesToHexStr(uid_str, sizeof(uid_str), info->tmp_uid.data, info->tmp_uid.size);
  Serial.printf("New card at reader #%d\n", idx);
  Serial.printf("UID: %s\n\n", uid_str);
#endif
}

static void onTagRemoved(
  MFRC522 &rfid,
  struct RfidReaderInfo *info,
  const struct Pos *pos,
  uint8_t idx
) {
  info->present = false;
  info->n_stable = 0;
  info->tmp_uid.size = 0;
  memset(info->tmp_uid.data, 0, sizeof(info->tmp_uid.data));

#if SHELFAWARE_DEBUG
  char uid_str[MAX_UID_SIZE * 2 + 1] = { 0 };
  bytesToHexStr(uid_str, sizeof(uid_str), info->uid.data, info->uid.size);
  Serial.printf("Card removed from reader #%d\n", idx);
  Serial.printf("UID: %s\n\n", uid_str);
#endif
}

static void onTagReplaced(
  MFRC522 &rfid,
  struct RfidReaderInfo *info,
  const struct Pos *pos,
  uint8_t idx
) {
  info->present = true;
  info->n_stable = 0;
  info->tmp_uid.size = rfid.uid.size;
  memcpy(info->tmp_uid.data, rfid.uid.uidByte, rfid.uid.size);

#if SHELFAWARE_DEBUG
  Serial.printf("Card replaced at reader #%d\n", idx);

  char uid_str[MAX_UID_SIZE * 2 + 1] = { 0 };
  bytesToHexStr(uid_str, sizeof(uid_str), info->uid.data, info->uid.size);
  Serial.printf("Old UID: %s\n", uid_str);

  memset(uid_str, 0, sizeof(uid_str));
  bytesToHexStr(uid_str, sizeof(uid_str), info->tmp_uid.data, info->tmp_uid.size);
  Serial.printf("New UID: %s\n\n", uid_str);
#endif
}

static void onTagIdle(
  struct RfidReaderInfo *info,
  const struct Pos *pos,
  uint8_t idx
) {
  info->present = true;
  info->n_stable++;

#if SHELFAWARE_DEBUG
  if (info->n_stable % 20 == 0) {
    char uid_str[MAX_UID_SIZE * 2 + 1] = { 0 };
    bytesToHexStr(uid_str, sizeof(uid_str), info->uid.data, info->uid.size);
    Serial.printf("Reader #%d idle for %llu polls\n", idx, info->n_stable);
    Serial.printf("UID: %s\n\n", uid_str);
  }
#endif
}

static void onTagReady(struct RfidReaderInfo *info, const struct Pos *pos, uint8_t idx) {
  info->present = true;
  if (++info->n_stable == STABLE_N)
    onTagReadyStable(info, pos, idx);
}

static void onTagEmpty(
  struct RfidReaderInfo *info,
  const struct Pos *pos,
  uint8_t idx
) {
  info->present = false;
  if (++info->n_stable == STABLE_N)
    onTagEmptyStable(info, pos, idx);
}

static void onTagReadyStable(
  struct RfidReaderInfo *info,
  const struct Pos *pos,
  uint8_t idx
) {
#if SHELFAWARE_DEBUG
  struct UID old_uid;
  memcpy(&old_uid, &info->uid, sizeof(info->uid));
  struct UID tmp_uid;
  memcpy(&tmp_uid, &info->tmp_uid, sizeof(info->tmp_uid));
#endif

  info->present = true;
  info->uid.size = info->tmp_uid.size;
  memcpy(info->uid.data, info->tmp_uid.data, info->tmp_uid.size);
  info->tmp_uid.size = 0;
  memset(info->tmp_uid.data, 0, sizeof(info->tmp_uid.data));

#if SHELFAWARE_DEBUG
  char uid_str[MAX_UID_SIZE * 2 + 1] = { 0 };
  if (old_uid.size == 0) {
    bytesToHexStr(uid_str, sizeof(uid_str), tmp_uid.data, tmp_uid.size);
    Serial.printf("Confirmed new card at reader #%d\n", idx);
    Serial.printf("UID: %s\n\n", uid_str);
  } else {
    bytesToHexStr(uid_str, sizeof(uid_str), old_uid.data, old_uid.size);
    Serial.printf("Confirmed replacement at reader #%d\n", idx);
    Serial.printf("Old UID: %s\n", uid_str);
    bytesToHexStr(uid_str, sizeof(uid_str), tmp_uid.data, tmp_uid.size);
    Serial.printf("New UID: %s\n", uid_str);
  }
#endif
}

static void onTagEmptyStable(
  struct RfidReaderInfo *info,
  const struct Pos *pos,
  uint8_t idx
) {
#if SHELFAWARE_DEBUG
  struct UID old_uid;
  memcpy(&old_uid, &info->uid, sizeof(info->uid));
  struct UID tmp_uid;
  memcpy(&tmp_uid, &info->tmp_uid, sizeof(info->tmp_uid));
#endif

  info->present = false;
  info->uid.size = 0;
  memset(info->uid.data, 0, sizeof(info->uid.data));
  info->tmp_uid.size = 0;
  memset(info->tmp_uid.data, 0, sizeof(info->tmp_uid.data));

#if SHELFAWARE_DEBUG
  if (old_uid.size != 0) {
    char uid_str[MAX_UID_SIZE * 2 + 1] = { 0 };
    bytesToHexStr(uid_str, sizeof(uid_str), old_uid.data, old_uid.size);
    Serial.printf("Confirmed removal at reader #%d\n", idx);
    Serial.printf("UID: %s\n\n", uid_str);
  }
#endif
}


// Assuming len > 2 * n_bytes
// else the function writes single null to *str
void bytesToHexStr(char *str, size_t len, const uint8_t *bytes, size_t n_bytes) {
  static const char hex[] = "0123456789abcdef";
  if (len == 0)
    return;
  if (len < 2 * n_bytes + 1) {
    *str = '\0';  // str[0]
    return;
  }

  for (size_t i = 0; i < n_bytes; i++) {
    const uint8_t byte = bytes[i];
    *str++ = hex[byte >> 4];   // str[2*i]
    *str++ = hex[byte & 0x0F]; // str[2*i+1]
  }
  *str = '\0';  // str[2*n_bytes]
}



void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.printf("\n\n");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  vTaskDelay(pdMS_TO_TICKS(100));
  digitalWrite(LED_PIN, HIGH);
  RFID_init();
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  RFID_loop();
}
