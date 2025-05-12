#include <http.h>
#include <stdio.h>
#include <string.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define WIFI_MAX_RETRIES 20

enum HttpEventType {
  HTTP_EV_ITEM_EVENT
};
struct HttpEvent {
  enum HttpEventType type;
  char data[64 - sizeof(enum HttpEventType)];  // casted to Req structs
};

static const char* ssid = "ShelfAware";
static const char* password = "icecoldengineering";
static const char* itemEventApiUrl = "https://shelfaware-app.fly.dev/api/item-event";

QueueHandle_t httpQueue;

static void connectToWiFi(const char *ssid, const char *password);
static int sendHttpPost(char *url, char *data);
static void uidToString(char *str, const uint8_t *uid, size_t size);
static int buildItemEventJson(char *buf, size_t bufsize, struct DbItemEventReq *req);
static void Task_HTTP(void *pvParams);

void HTTP_init() {
  connectToWiFi(ssid, password);

  httpQueue = xQueueCreate(HTTP_Q_SIZE, sizeof(struct HttpEvent));

  xTaskCreatePinnedToCore(Task_HTTP, "Task_HTTP", 1024, NULL, 1, NULL, 1);
}

static void connectToWiFi(const char *ssid, const char *password) {
  Serial.println("Initializing WiFi...");

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA); // Force Station Mode

  // const int n_networks = WiFi.scanNetworks();
  // for (int i = 0; i < n_networks; i++) {
  //   Serial.printf("[%d/%d] SSID: %s, RSSI: %d\n", i, n_networks - 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
  // }

  WiFi.begin(ssid, password);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < WIFI_MAX_RETRIES) {
    Serial.print(".");
    delay(500);
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nWiFi connected!\n");
    Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.printf("\nFailed to connect to WiFi\n");
  }
}

static int sendHttpPost(char *url, char *data) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    int res_code = http.POST(data);

    if (res_code > 0) {
      // String res = http.getString();
      // Serial.printf("Response code: %s\n", String(res_code).c_str());
      // Serial.printf("Response body: %s\n", res.c_str());
    } else {
      Serial.printf("POST failed, error: %s\n", http.errorToString(res_code).c_str());
    }

    http.end();

    return res_code;
  } else {
    Serial.println("WiFi Disconnected");
    return -1;
  }
}

// Assuming size of str is more than 2 * size
static void uidToString(char *str, const uint8_t *uid, size_t size) {
  static const char hex[] = "0123456789abcdef";
  for (size_t i = 0; i < size; i++) {
    const uint8_t byte = uid[i];
    *str++ = hex[(byte >> 4) & 0x0F];
    *str++ = hex[byte & 0x0F];
  }
  *str = '\0';
}

// Return number of bytes written, or should have been written if buf_size is not enough
// Server expects:
// {
//   uid: ^\[0-9a-f]+\$,
//   weight: number,
//   eventType: "in" | "out" | "moved"
// }
static int buildItemEventJson(char *buf, size_t bufsize, struct DbItemEventReq *req) {
  static const char *type_str[] = {
    [DB_ITEM_EV_IN]    = "in",
    [DB_ITEM_EV_OUT]   = "out",
    [DB_ITEM_EV_MOVED] = "moved"
  };

  char uid_str[MAX_UID_SIZE * 2 + 1];
  uidToString(uid_str, req->uid, req->uid_size);

  const int n = snprintf(
    buf, bufsize,
    "{\"uid\":\"%s\",\"weight\":%.2f,\"eventType\":\"%s\"}",
    uid_str,
    req->weight,
    type_str[req->eventType]
  );

  if (n < 0)
    return -1;
  
  return n;
}

static void Task_HTTP(void *pvParams) {
  struct HttpEvent ev;
  char json_buf[256];
  for (;;) {
    if (xQueueReceive(httpQueue, &ev, portMAX_DELAY) == pdPASS) {
      switch (ev.type) {
        case HTTP_EV_ITEM_EVENT: {
          memset(json_buf, 0, sizeof(json_buf));
          buildItemEventJson(
            json_buf, 
            sizeof(json_buf), 
            (struct DbItemEventReq) ev.data
          );
          sendHttpPost(itemEventApiUrl, json_buf, sizeof(json_buf));
          break;
        }
      }
    }
  }
}

void HTTP_sendItemEvent(struct DbItemEventReq *req) {
  struct HttpEvent ev = {
    .type = HTTP_EV_ITEM_EVENT
  };
  memcpy(ev.data, req, sizeof(struct DbItemEventReq));
  xQueueSend(httpQueue, &ev, portMAX_DELAY);
}
