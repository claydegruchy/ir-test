// #include <Arduino.h>
// #include <BLEDevice.h>
// #include <BLEUtils.h>
// #include <BLEServer.h>

// // "BATTERY_SERVICE": "0000180f-0000-1000-8000-00805f9b34fb",
// // "BATTERY_CHARACTERISTIC": "00002a19-0000-1000-8000-00805f9b34fb"

// #define SERVICE_UUID "0000180f-0000-1000-8000-00805f9b34fb"
// #define CHARACTERISTIC_UUID "00002a19-0000-1000-8000-00805f9b34fb"

// uint32_t value = 69;

// void setup()
// {
//   Serial.begin(115200);
//   while (!Serial)
//     ; // Wait for Serial to become available. Is optimized away for some cores.
//   Serial.println("BLUE test starting");

//   BLEDevice::init("Long name works now");
//   BLEServer *pServer = BLEDevice::createServer();
//   BLEService *pService = pServer->createService(SERVICE_UUID);
//   BLECharacteristic *pCharacteristic = pService->createCharacteristic(
//       CHARACTERISTIC_UUID,
//       BLECharacteristic::PROPERTY_READ |
//           BLECharacteristic::PROPERTY_WRITE);

//   pCharacteristic->setValue((uint8_t *)&value, 4);
//   pService->start();
//   // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
//   BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
//   pAdvertising->addServiceUUID(SERVICE_UUID);
//   pAdvertising->setScanResponse(true);
//   pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
//   pAdvertising->setMinPreferred(0x12);
//   BLEDevice::startAdvertising();
//   Serial.println("Characteristic defined! Now you can read it in your phone!");

// }

// void loop()
// {
//   Serial.println("loop");

//   delay(1000);
// }
