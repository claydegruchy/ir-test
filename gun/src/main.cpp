#include <Arduino.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "gun.h"
#include "health_check.h"

#define SERVICE_UUID "0000180f-0000-1000-8000-00805f9b34fb"

#define DEVICE_ID "2"
#define DEVICE_TYPE "GUN"

String device_name = String(DEVICE_TYPE) + String(DEVICE_ID);

BLEServer *pServer = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    Serial.println("[MAIN]  Device connected");
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer) {
    Serial.println("[MAIN]  Device disconnected");
    deviceConnected = false;
  }
};

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ; // Wait for Serial to become available. Is optimized away for some cores.
  Serial.print("[MAIN]  BLE test starting with name:");
  Serial.println(device_name);

  BLEDevice::init(std::string(device_name.c_str()));

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  gun_setup(pService);
  health_setup(pService);

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(
      0x0); // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("[MAIN]  Waiting a client connection to notify...");
}

void handle_connections() {
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("[MAIN]  start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}

int l = 0;
void loop() {
  l++;

  // notify changed value
  if (deviceConnected) {
    if (l % 1000 == 0) {
      Serial.println("[MAIN]  device connected");
    }
    if (l % 1000 == 0) {
      Serial.println(l);
      health_tick();
    }
    if (l % 300 == 0) {
      Serial.println("[MAIN]  running firing test");
      Serial.println(l);
      gun_tick();
    }

    // delay(3); // bluetooth stack will go into congestion, if too many
    // packets are sent, in 6 hours test i was able to go as low as 3ms
  }
  handle_connections();
  delay(1);
}
