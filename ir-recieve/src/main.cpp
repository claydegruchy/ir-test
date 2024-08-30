// test board mac address FC:E8:C0:7B:58:BC

// #define USE_ONKYO_PROTOCOL    // Like NEC, but take the 16 bit address and
// command each as one 16 bit value and not as 8 bit normal and 8 bit inverted
// value. #define USE_FAST_PROTOCOL // Use FAST protocol instead of NEC / ONKYO
#define IR_RECEIVE_PIN 35
// dont use pin 13 its fucked up
#include "TinyIRReceiver.hpp"

#include <WiFi.h>
#include <esp_now.h>

#define DEVICE_ID 20
// #define DEVICE_ID 30

uint8_t broadcastAddress[] = {
    // 0x08, 0xB6, 0x1F, 0xB8, 0x8E, 0x50}; // GUN3
    0x24, 0xDC, 0xC3, 0x45, 0x4A, 0x2C}; // GUN2

    
struct ESPNowData {
  int val;
  int id;
};

void esp_now_setup() {
  Serial.println("Running esp_now_setup");

  WiFi.mode(WIFI_MODE_APSTA);
  Serial.print("Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

  esp_now_init(); // Initialize ESP-NOW

  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  esp_now_add_peer(&peerInfo);
  Serial.println("Finished esp_now_setup");
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ; // Wait for Serial to become available. Is optimized away for some cores.

  esp_now_setup();

  initPCIInterruptForTinyReceiver(); // Enables the interrupt generation on
                                     // change of IR input signal

#if defined(USE_FAST_PROTOCOL)
  Serial.print("Using FAST: ");
#else
  Serial.print("Using NEC: ");

#endif
  Serial.println("Ready to receive");
}

void hit_recieved(uint8_t result) {
  Serial.println("hit_recieved, sending to ESP NOW");

  ESPNowData data;
  data.val = result;
  data.id = DEVICE_ID;

  esp_err_t result =
      esp_now_send(broadcastAddress, (uint8_t *)&data, sizeof(data));
  if (result == ESP_OK)
    Serial.println("SUCCESS:succeed ");
  if (result == ESP_ERR_ESPNOW_NOT_INIT)
    Serial.println("ERROR:ESPNOW is not initialized ");
  if (result == ESP_ERR_ESPNOW_ARG)
    Serial.println("ERROR:invalid argument ");
  if (result == ESP_ERR_ESPNOW_INTERNAL)
    Serial.println("ERROR:internal error ");
  if (result == ESP_ERR_ESPNOW_NO_MEM)
    Serial.println("ERROR:out of memory, when this happens, you can delay a "
                   "while before sending the next data ");
  if (result == ESP_ERR_ESPNOW_NOT_FOUND)
    Serial.println("ERROR:peer is not found ");
  if (result == ESP_ERR_ESPNOW_IF)
    Serial.println("ERROR:current WiFi interface doesn't match that of peer");
}

int i = 0;
void loop() {
  if (TinyIRReceiverData.justWritten) {
    Serial.println("Got something");
    TinyIRReceiverData.justWritten = false;
#if !defined(USE_FAST_PROTOCOL)
    // We have no address at FAST protocol
    Serial.print(F("Address=0x"));
    Serial.print(TinyIRReceiverData.Address, HEX);
    Serial.print(' ');
#endif
    Serial.print(F("Command=0x"));
    Serial.print(TinyIRReceiverData.Command, HEX);
    if (TinyIRReceiverData.Flags == IRDATA_FLAGS_IS_REPEAT) {
      Serial.println(F(" Repeat, skipping hit_recieved"));
    } else {
      hit_recieved(TinyIRReceiverData.Command);
    }

    if (TinyIRReceiverData.Flags == IRDATA_FLAGS_PARITY_FAILED) {
      Serial.print(F(" Parity failed"));
#if !defined(USE_EXTENDED_NEC_PROTOCOL) && !defined(USE_ONKYO_PROTOCOL)
      Serial.print(F(", try USE_EXTENDED_NEC_PROTOCOL or USE_ONKYO_PROTOCOL"));
#endif
    }
    Serial.println();
  }

  // i++;
  // if (i > 9999)
  // {
  //   i = 0;
  // }

  // if (i % 100000 == 0)
  // {

  //   Serial.print("loop:");
  //   Serial.println(i);
  //   numberToSend = i;
  // }
  // i++;
}
