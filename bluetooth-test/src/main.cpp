#include <Arduino.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "health_check.h"

// "BATTERY_SERVICE": "0000180f-0000-1000-8000-00805f9b34fb",
// "BATTERY_CHARACTERISTIC": "00002a19-0000-1000-8000-00805f9b34fb"

#define SERVICE_UUID "0000180f-0000-1000-8000-00805f9b34fb"

#define CHARACTERISTIC_UUID_SHOT_FIRED "ecc24e5d-0d95-4111-aa0a-6e40a76b8d74"
#define CHARACTERISTIC_UUID_RELOAD_PRESSED                                     \
  "2002a0b9-6aa4-40aa-a2bd-de02c8ec4e04"

#define CHARACTERISTIC_UUID_SHOT_RECIEVED "84c2e088-61e1-4df1-a1b4-12ec61a1144f"

BLEServer *pServer = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    Serial.println("Device connected");
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer) {
    Serial.println("Device disconnected");
    deviceConnected = false;
  }
};

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ; // Wait for Serial to become available. Is optimized away for some cores.
  Serial.println("BLUE test starting");
  BLEDevice::init("BLE test suite");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

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
  Serial.println("Waiting a client connection to notify...");
}

bool value_to_send = false;

/*
we send in a 5 slot array via some given charac

0 = the current configured type that is sending [1 = helmet, 2 = gun, 255 =
diganostic]. 2 = the id of this type. ie helmet 8 would be 8. 3 = the key that
this is targeting. [1 = shot fired, 2 = shot recieved]. 4 = the value of the
previous key [if i recieved a shot from gun with id 66, then the value would be
66]. 5 = always 0.

example
[1,3,2,23,0] - i am HELMET 3, i have been shot by gun 23
[2,23,1,0,0] - i am GUN 23, I have fired a shot


*/

void handle_connections() {
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
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
    Serial.println("device connected");
    Serial.println(l);
    health_tick();

    // delay(3); // bluetooth stack will go into congestion, if too many packets
    // are sent, in 6 hours test i was able to go as low as 3ms
    delay(1000);
  }
  handle_connections();
}
