#ifndef _SENDDATA_H
#define _SENDDATA_H

// Struct holding payload for data send queue
typedef struct {
  uint8_t MessageSize;
  uint8_t MessagePort;
  uint8_t Message[PAYLOAD_BUFFER_SIZE];
} MessageBuffer_t;

void SendData(uint8_t port);
void sendPayload(void);
void SendCycleIRQ(void);
void processSendBuffer(void);

#endif // _SENDDATA_H_