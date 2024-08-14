/*

this file handles the hp of the player and other vitals
it listens to the signals from the IR sensors and then relays that to the phone
the actuall HP levels are handled by the phone itself, as is disabling the gun in the case of death
*/

#include "player_health.h"

int receivedNumber = 0;

#define CHARACTERISTIC_UUID_HIT_REGO "d230d989-efbf-4658-967f-ed233b5c5c2b"
BLECharacteristic *HitRegistrationCharacteristic = NULL;

void onESPNowReceive(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  // we have been hit by something
  memcpy(&receivedNumber, incomingData, sizeof(receivedNumber));
  Serial.print("Received number: ");
  Serial.println(receivedNumber);

  HitRegistrationCharacteristic->setValue((uint8_t *)&receivedNumber, 4);
  HitRegistrationCharacteristic->notify(); // Notify connected devices (if desired)
  receivedNumber = 0;
}

void player_health_setup(BLEService *pService)
{

  Serial.println("[PLAYER_HEALTH]  Running player_health_setup");

  WiFi.mode(WIFI_STA);

  Serial.print("Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  esp_now_init(); // Initialize ESP-NOW

  esp_now_register_recv_cb(onESPNowReceive); // Register receive callback

  HitRegistrationCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_HIT_REGO, BLECharacteristic::PROPERTY_READ |
                                        BLECharacteristic::PROPERTY_WRITE |
                                        BLECharacteristic::PROPERTY_NOTIFY |
                                        BLECharacteristic::PROPERTY_INDICATE);

  HitRegistrationCharacteristic->addDescriptor(new BLE2902());
}
