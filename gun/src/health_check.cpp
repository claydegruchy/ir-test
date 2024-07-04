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

#define CHARACTERISTIC_UUID_HEALTH "00002a19-0000-1000-8000-00805f9b34fb"

uint32_t health_tick_value = 69;

BLECharacteristic *pCharacteristic = NULL;

class pCharacteristicCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    Serial.println("[HEALTH_CHECK]  Running onWrite callback");

    std::string recievedValue = pCharacteristic->getValue();
    if (recievedValue.length() != 1)
    {
      Serial.println("[HEALTH_CHECK]  This is a wrong value");
      return;
    }
    // Print the received bytes
    Serial.print("Received Value: ");
    for (int i = 0; i < recievedValue.length(); i++)
    {
      if (i == 0)
      {
        Serial.print("setting value to:");
        health_tick_value = (uint32_t)recievedValue[i];
      }

      Serial.print((uint32_t)recievedValue[i]);
      // value=
      Serial.print(" ");
    }
    Serial.println();
  }
};

void health_setup(BLEService *pService)
{
  Serial.println("[HEALTH_CHECK]  Running health_setup");

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_HEALTH, BLECharacteristic::PROPERTY_READ |
                                      BLECharacteristic::PROPERTY_WRITE |
                                      BLECharacteristic::PROPERTY_NOTIFY |
                                      BLECharacteristic::PROPERTY_INDICATE);

  pCharacteristic->setCallbacks(new pCharacteristicCallbacks());

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());
  Serial.println("[HEALTH_CHECK]  Running health_setup complete");
}

void health_tick()
{
  Serial.println("[HEALTH_CHECK]  Running health_tick");

  Serial.print("Sending health tick value:");
  Serial.println(health_tick_value);

  pCharacteristic->setValue((uint8_t *)&health_tick_value, 4);
  pCharacteristic->notify(); // Notify connected devices (if desired)
  delay(5);                  // this prevent flooding of the BLE connection

  health_tick_value++;
  if (health_tick_value > 99)
  {
    health_tick_value = 1;
  }
  Serial.println("[HEALTH_CHECK]  Running health_tick complete");
}