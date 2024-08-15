/*

this file handles the hp of the player and other vitals
it listens to the signals from the IR sensors and then relays that to the phone
the actuall HP levels are handled by the phone itself, as is disabling the gun
in the case of death
*/

#include "player_health.h"

int receivedNumber = 0;

#define CHARACTERISTIC_UUID_SHOT_RECIEVED "d230d989-efbf-4658-967f-ed233b5c5c2b"
BLECharacteristic *HitRegistrationCharacteristic = NULL;

struct ESPNowData {
  int val;
  int id;
};

void onESPNowReceive(const uint8_t *mac, const uint8_t *incomingData, int len) {
  // we have been hit by something
  // memcpy(&receivedNumber, incomingData, sizeof(receivedNumber));

  ESPNowData *data = (ESPNowData *)incomingData;
  int id = data->id;
  int val = data->val;
  // val = 200;

  Serial.println("Received signal via ESP NOW: ");
  Serial.printf("From recieving device: %d, Value: %d\n", id, val);

  int hitData[] = {val, id};

  HitRegistrationCharacteristic->setValue((uint8_t *)&hitData, sizeof(hitData));

  HitRegistrationCharacteristic
      ->notify(); // Notify connected devices (if desired)
  delay(5);
  // receivedNumber = 0;
}

void player_health_setup(BLEService *pService) {

  Serial.println("[PLAYER_HEALTH]  Running player_health_setup");

  // set up bluetooth signals
  HitRegistrationCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_SHOT_RECIEVED,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY |
          BLECharacteristic::PROPERTY_INDICATE);

  HitRegistrationCharacteristic->addDescriptor(new BLE2902());

  // set up esp now hit rego signals
  WiFi.mode(WIFI_STA);

  Serial.print("Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  esp_now_init(); // Initialize ESP-NOW

  esp_now_register_recv_cb(onESPNowReceive); // Register IR receive callback
}

void player_health_tick(int loopNumber) {
  // if (loopNumber % 1500 != 0)
  //   return;
  // receivedNumber++;
  // Serial.println("[PLAYER_HEALTH]  tick");
  // send_to_ble();
}