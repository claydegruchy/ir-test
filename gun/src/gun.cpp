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

#define IR_SEND_PIN 12
#define TRIGGER_PIN 14
#define RELOAD_PIN 27

#include "TinyIRSender.hpp"

#define CHARACTERISTIC_UUID_SHOT_FIRED "ecc24e5d-0d95-4111-aa0a-6e40a76b8d74"
#define CHARACTERISTIC_UUID_RELOAD "2002a0b9-6aa4-40aa-a2bd-de02c8ec4e04"
#define CHARACTERISTIC_UUID_CONFIGURATION "0fa0e6c8-f586-4c84-a431-2862275808e2"

// CONFIGS
bool gun_enabled = true;
bool debug_auto_shoot = true;
uint32_t max_clip_size = 12;
uint32_t fire_shot_cooldown_max = 1000 / 10;
uint32_t reload_cooldown_max = 1000;

// ephemerals/HANDLERS
uint32_t current_clip_size = 0;
uint32_t shots_fired = 0;
uint32_t fire_shot_cooldown_remaining = 0;
uint32_t reload_cooldown_remaining = 0;

BLECharacteristic *shotFiredCharacteristic = NULL;
BLECharacteristic *reloadPressedCharacteristic = NULL;
BLECharacteristic *configurationCharacteristic = NULL;

class shotFiredCharacteristicCallbacks : public BLECharacteristicCallbacks {};
class reloadPressedCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *reloadPressedCharacteristic) {
    Serial.println("[GUN]  [reloadPressedCharacteristicCallbacks] write: ");
    Serial.println("[GUN]  [reloadPressedCharacteristicCallbacks] Running "
                   "onWrite callback");

    std::string recievedValue = reloadPressedCharacteristic->getValue();
    if (recievedValue.length() != 1) {
      Serial.println("[GUN]  [reloadPressedCharacteristicCallbacks] This is a "
                     "wrong value");
      return;
    }
    // Print the received bytes
    Serial.print("Received Value: ");
    for (int i = 0; i < recievedValue.length(); i++) {

      Serial.print((uint32_t)recievedValue[i]);
      Serial.print("(reloading clip with this many bullets)");

      current_clip_size = (uint32_t)recievedValue[i];
      // value=
      Serial.print(" ");
    }
    Serial.println();
  }
};

class configurationCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic *configurationCharacteristicCallbacks) {
    Serial.println("[GUN]  [configurationCharacteristicCallbacks] read: ");
    String val = "val for config callback";
    configurationCharacteristicCallbacks->setValue(val.c_str());
  }
  void onWrite(BLECharacteristic *configurationCharacteristicCallbacks) {
    Serial.println("[GUN]  [configurationCharacteristicCallbacks] write: ");

    std::string value = configurationCharacteristicCallbacks->getValue();

    // Step 2: Convert string to byte array
    std::vector<uint8_t> byteData(value.begin(), value.end());

    // Step 3: Interpret byte array as Uint16Array
    size_t numValues = byteData.size() / 2; // Number of Uint16 values
    std::vector<uint16_t> values(numValues);

    for (size_t i = 0; i < numValues; ++i) {
      // Combine two bytes into a Uint16 value (assuming little-endian)
      values[i] = byteData[i * 2] | (byteData[i * 2 + 1] << 8);
    }

    Serial.print("Total size of values:");
    Serial.println(values.size());

    Serial.print("[config] [gun_enabled]");
    if (gun_enabled != values[0]) {
      Serial.print("updating ");
      Serial.print(gun_enabled);
      Serial.print(" to ");
      Serial.println(values[0]);
      gun_enabled = values[0];
    } else {
      Serial.print("skipping. currently:");
      Serial.println(gun_enabled);
    }

    Serial.print("[config] [debug_auto_shoot]");
    if (debug_auto_shoot != values[1]) {
      Serial.print("updating ");
      Serial.print(debug_auto_shoot);
      Serial.print(" to ");
      Serial.println(values[1]);
      debug_auto_shoot = values[1];
    } else {
      Serial.print("skipping. currently:");
      Serial.println(debug_auto_shoot);
    }

    Serial.print("[config] [max_clip_size]");
    if (max_clip_size != values[2]) {
      Serial.print("updating ");
      Serial.print(max_clip_size);
      Serial.print(" to ");
      Serial.println(values[2]);
      max_clip_size = values[2];
    } else {
      Serial.print("skipping. currently:");
      Serial.println(max_clip_size);
    }

    Serial.print("[config] [fire_shot_cooldown_max]");
    if (fire_shot_cooldown_max != values[3]) {
      Serial.print("updating ");
      Serial.print(fire_shot_cooldown_max);
      Serial.print(" to ");
      Serial.println(values[3]);
      fire_shot_cooldown_max = values[3];
    } else {
      Serial.print("skipping. currently:");
      Serial.println(fire_shot_cooldown_max);
    }

    Serial.print("[config] [reload_cooldown_max]");
    if (reload_cooldown_max != values[4]) {
      Serial.print("updating ");
      Serial.print(reload_cooldown_max);
      Serial.print(" to ");
      Serial.println(values[4]);
      reload_cooldown_max = values[4];
    } else {
      Serial.print("skipping. currently:");
      Serial.println(reload_cooldown_max);
    }
  }
};

void gun_setup(BLEService *pService) {
  Serial.println("[GUN]  Running GUN_SETUP");

  Serial.println("[GUN]  establishing shotFiredCharacteristic");
  // Create a BLE Characteristic FOR THE GUNSHOT
  shotFiredCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_SHOT_FIRED, BLECharacteristic::PROPERTY_READ |
                                          BLECharacteristic::PROPERTY_NOTIFY |
                                          BLECharacteristic::PROPERTY_INDICATE);

  shotFiredCharacteristic->setCallbacks(new shotFiredCharacteristicCallbacks());

  // Create a BLE Descriptor for reload pressed
  shotFiredCharacteristic->addDescriptor(new BLE2902());

  Serial.println("[GUN]  establishing reloadPressedCharacteristic");
  // Create a BLE Characteristic FOR THE RELOAD
  reloadPressedCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_RELOAD, BLECharacteristic::PROPERTY_READ |
                                      BLECharacteristic::PROPERTY_NOTIFY |
                                      BLECharacteristic::PROPERTY_INDICATE);

  reloadPressedCharacteristic->setCallbacks(
      new reloadPressedCharacteristicCallbacks());

  // Create a BLE Descriptor
  reloadPressedCharacteristic->addDescriptor(new BLE2902());

  Serial.println("[GUN]  establishing configurationCharacteristic");
  // Create a BLE Characteristic FOR CONFIGURATION
  configurationCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_CONFIGURATION,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  configurationCharacteristic->setCallbacks(
      new configurationCharacteristicCallbacks());

  // Create a BLE Descriptor
  configurationCharacteristic->addDescriptor(new BLE2902());

  Serial.println("[GUN]  setting pinmode for IR system");

  pinMode(TRIGGER_PIN, INPUT_PULLUP); // config GPIO21 as input pin and enable
                                      // the internal pull-down resistor
  pinMode(RELOAD_PIN, INPUT_PULLUP);  // config GPIO21 as input pin and enable
  // the internal pull-down resistor

  Serial.println("[GUN]  Running GUN_SETUP complete");
}

void send_ir_signal() {
  // Serial.println("[GUN]  [send_ir_signal]  sending ir signal ");
  // Serial.flush();
  sendNEC(IR_SEND_PIN, 0, 11,
          2); // Send address 0 and command 11 on pin 3 with 2 repeats.
}

void fire_gun() {
  // Serial.println("[GUN]  [fire_gun]  starting ");

  if (fire_shot_cooldown_remaining > 0) {
    // Serial.print("[GUN]  [fire_gun]  failed: on cooldown:");
    Serial.println(fire_shot_cooldown_remaining);

    return;
  }

  if (current_clip_size <= 0) {
    Serial.println("[GUN]  [fire_gun]  failed: no ammo ");
    return;
  }

  // pay for the shot
  shots_fired += 1;
  current_clip_size -= 1;
  // Serial.print("[GUN]  [fire_gun]  firing shot: deducting ammo ");
  // Serial.print(current_clip_size);
  // Serial.println(" remaining");

  // Serial.print("[GUN]  [fire_gun]  firing shot: starting cooldown ");
  fire_shot_cooldown_remaining = fire_shot_cooldown_max;

  // actually send the IR

  unsigned long start = micros();

  send_ir_signal();

  // Compute the time it took
  unsigned long end = micros();
  unsigned long delta = end - start;
  Serial.println(delta);

  // tell the phone about it
  // Serial.println("[GUN]  [fire_gun]  sending ble communication ");
  shotFiredCharacteristic->setValue((uint8_t *)&current_clip_size, 4);
  shotFiredCharacteristic->notify(); // Notify connected devices (if desired)
  delay(5); // this prevent flooding of the BLE connection
  fire_shot_cooldown_remaining -= 5; // dont make the user pay a tax for bt lag
}

void reload_gun() {
  Serial.println("[GUN]  [reload_gun] reloading");
  if (reload_cooldown_remaining > 0) {
    Serial.println("[GUN]  [reload_gun] failed: reload cooldown");
    return;
  }

  Serial.println("[GUN]  [reload_gun]  sending ble communication ");
  reloadPressedCharacteristic->setValue((uint8_t *)&current_clip_size, 4);
  // String val = "val.1";
  // reloadPressedCharacteristic->setValue(val.c_str());
  reloadPressedCharacteristic
      ->notify(); // Notify connected devices (if desired)
  delay(5);       // this prevent flooding of the BLE connection
  current_clip_size = max_clip_size;
  fire_shot_cooldown_remaining = 0;
  reload_cooldown_remaining = reload_cooldown_max;
}

bool previous_trigger_pin_state = 1;
bool previous_reload_pin_state = 1;

void gun_tick(int tick = -1) {
  // Serial.println("[GUN]  [gun_tick] Running gun_tick");
  if (!gun_enabled) {
    Serial.println("[GUN]  [gun_tick] gun not enabled. skipping");
    return;
  }

  if (debug_auto_shoot && tick % fire_shot_cooldown_max == 0) {

    Serial.println("[GUN]  [gun_tick] debug auto fire enabled");
    fire_gun();
    if (current_clip_size <= 0) {
      Serial.println("[GUN]  [gun_tick] debug auto reload enabled");
      reload_gun();
    }
  }

  int trigger_pin_state = digitalRead(TRIGGER_PIN);

  if (!trigger_pin_state) {
    // Serial.print("[GUN]  [gun_tick] checking if trigger pressed:");
    // Serial.println(trigger_pin_state);
    fire_gun();
  }

  int reload_pin_state = digitalRead(RELOAD_PIN);
  if (!reload_pin_state) {
    // Serial.print("[GUN]  [gun_tick] checking if reload pressed:");
    // Serial.println(reload_pin_state);
    reload_gun();
  }

  if (fire_shot_cooldown_remaining > 0)
    fire_shot_cooldown_remaining -= 1;

  if (reload_cooldown_remaining > 0)
    reload_cooldown_remaining -= 1;

  // Serial.println("[GUN]  [gun_tick] Running gun_tick complete");
}