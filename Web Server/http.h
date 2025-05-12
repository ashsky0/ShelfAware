#ifndef SHELFAWARE_HTTP_H_
#define SHELFAWARE_HTTP_H_

#define MAX_UID_SIZE 14
#define HTTP_Q_SIZE 2

enum DbItemEventReqType {
  DB_ITEM_EV_IN,
  DB_ITEM_EV_OUT,
  DB_ITEM_EV_MOVED
};
struct DbItemEventReq {
  enum DbItemEventReqType eventType;
  uint8_t uid_size;
  uint8_t uid[MAX_UID_SIZE];
  double weight;
};

void HTTP_init(void);

void HTTP_sendItemEvent(struct DbItemEventReq *req);

#endif  // SHELFAWARE_HTTP_H_
