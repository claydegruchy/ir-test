/*
this file contains the code for a BLE transaction attribute
it has a CHARACTERISTIC which it broadcasts on
and it defines some callbacks that are activated when it gets a write

this code:
sends a health signal every big loop


 */

#include "health_check.h"

#include <Arduino.h> // Make sure Arduino.h is included
#include <BLE2902.h>

#define CHARACTERISTIC_UUID_SHOT_FIRED "ecc24e5d-0d95-4111-aa0a-6e40a76b8d74"
#define CHARACTERISTIC_UUID_RELOAD "2002a0b9-6aa4-40aa-a2bd-de02c8ec4e04"
#define CHARACTERISTIC_UUID_CONFIGURATION "2002a0b9-6aa4-40aa-a2bd-de02c8ec4e04"

uint32_t shots_fired = 0;

uint32_t max_clip_size = 5;
uint32_t current_clip_size = 5;

bool gun_enabled = true;

BLECharacteristic *shotFiredCharacteristic = NULL;
BLECharacteristic *reloadPressedCharacteristic = NULL;

class shotFiredCharacteristicCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *shotFiredCharacteristic)
  {
    Serial.println("[GUN]  [shotFiredCharacteristicCallbacks] write: ");

    Serial.println("[GUN]  [shotFiredCharacteristicCallbacks] Running onWrite callback");

    std::string recievedValue = shotFiredCharacteristic->getValue();
    if (recievedValue.length() != 1)
    {
      Serial.println("[GUN]  [shotFiredCharacteristicCallbacks] This is a wrong value");
      return;
    }
    // Print the received bytes
    Serial.print("Received Value: ");
    for (int i = 0; i < recievedValue.length(); i++)
    {

      Serial.print((uint32_t)recievedValue[i]);
      // value=
      Serial.print(" ");
    }
    Serial.println();
  }
};
class reloadPressedCharacteristicCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *reloadPressedCharacteristic)
  {
    Serial.println("[GUN]  [reloadPressedCharacteristicCallbacks] write: ");
    Serial.println("[GUN]  [reloadPressedCharacteristicCallbacks] Running onWrite callback");

    std::string recievedValue = reloadPressedCharacteristic->getValue();
    if (recievedValue.length() != 1)
    {
      Serial.println("[GUN]  [reloadPressedCharacteristicCallbacks] This is a wrong value");
      return;
    }
    // Print the received bytes
    Serial.print("Received Value: ");
    for (int i = 0; i < recievedValue.length(); i++)
    {

      Serial.print((uint32_t)recievedValue[i]);
      Serial.print("(reloading clip with this many bullets)");

      current_clip_size = (uint32_t)recievedValue[i];
      // value=
      Serial.print(" ");
    }
    Serial.println();
  }
};

void gun_setup(BLEService *pService)
{
  Serial.println("[GUN]  Running GUN_SETUP");

  Serial.println("[GUN]  establishing shotFiredCharacteristic");
  // Create a BLE Characteristic FOR THE GUNSHOT
  shotFiredCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_SHOT_FIRED, BLECharacteristic::PROPERTY_READ |
                                          BLECharacteristic::PROPERTY_WRITE |
                                          BLECharacteristic::PROPERTY_NOTIFY |
                                          BLECharacteristic::PROPERTY_INDICATE);

  shotFiredCharacteristic->setCallbacks(new shotFiredCharacteristicCallbacks());

  // Create a BLE Descriptor for reload pressed
  shotFiredCharacteristic->addDescriptor(new BLE2902());

  Serial.println("[GUN]  establishing reloadPressedCharacteristic");
  // Create a BLE Characteristic FOR THE GUNSHOT
  reloadPressedCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_RELOAD, BLECharacteristic::PROPERTY_READ |
                                      BLECharacteristic::PROPERTY_NOTIFY |
                                      BLECharacteristic::PROPERTY_INDICATE);

  reloadPressedCharacteristic->setCallbacks(new reloadPressedCharacteristicCallbacks());

  // Create a BLE Descriptor
  reloadPressedCharacteristic->addDescriptor(new BLE2902());

  Serial.println("[GUN]  Running GUN_SETUP complete");
}

void send_ir_signal()
{
  Serial.println("[GUN]  [send_ir_signal]  sending ir signal ");
}

void fire_shot()
{
  Serial.println("[GUN]  [fire_shot]  starting ");

  if (current_clip_size <= 0)
  {
    Serial.println("[GUN]  [fire_shot]  failed: no ammo ");
    return;
  }

  // pay for the shot
  shots_fired += 1;
  current_clip_size -= 1;
  Serial.print("[GUN]  [fire_shot]  success: deducting ammo ");
  Serial.print(current_clip_size);
  Serial.println(" remaining");

  // actually send the IR
  send_ir_signal();

  // tell the phone about it
  Serial.println("[GUN]  [fire_shot]  sending ble communication ");
  shotFiredCharacteristic->setValue((uint8_t *)&current_clip_size, 4);
  shotFiredCharacteristic->notify(); // Notify connected devices (if desired)
  delay(5);                          // this prevent flooding of the BLE connection
}

void reload_gun()
{
  Serial.println("[GUN]  [reload_gun] reloading");
  Serial.println("[GUN]  [reload_gun]  sending ble communication ");
  // reloadPressedCharacteristic->setValue((uint8_t *)&current_clip_size, 4);
  String val = "val.1";
  reloadPressedCharacteristic->setValue(val.c_str());
  reloadPressedCharacteristic->notify(); // Notify connected devices (if desired)
  delay(5);                              // this prevent flooding of the BLE connection
  current_clip_size = max_clip_size;
}

void gun_tick()
{
  Serial.println("[GUN]  [gun_tick] Running gun_tick");
  if (!gun_enabled)
  {
    Serial.println("[GUN]  [gun_tick] gun not enabled. skipping");
    return;
  }

  fire_shot();

  if (current_clip_size <= 0)
  {
    Serial.println("[GUN]  [gun_tick] out of ammo, reloading...");
    reload_gun();
  }

  Serial.println("[GUN]  [gun_tick] Running gun_tick complete");
}