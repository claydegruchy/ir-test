#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// "BATTERY_SERVICE": "0000180f-0000-1000-8000-00805f9b34fb",
// "BATTERY_CHARACTERISTIC": "00002a19-0000-1000-8000-00805f9b34fb"

#define SERVICE_UUID "0000180f-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_UUID "00002a19-0000-1000-8000-00805f9b34fb"

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
  }
};

uint32_t health_tick_value = 69;

class pCharacteristicCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string recievedValue = pCharacteristic->getValue();
    if (recievedValue.length() != 5)
    {
      Serial.println("This is a wrong value");
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

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ; // Wait for Serial to become available. Is optimized away for some cores.
  Serial.println("BLUE test starting");
  BLEDevice::init("BLE test suite");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_NOTIFY |
          BLECharacteristic::PROPERTY_INDICATE);

  pCharacteristic->setCallbacks(new pCharacteristicCallbacks());

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0); // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

bool value_to_send = false;

/*
we send in a 5 slot array via some given charac

0 = the current configured type that is sending [1 = helmet, 2 = gun, 255 = diganostic].
2 = the id of this type. ie helmet 8 would be 8.
3 = the key that this is targeting. [1 = shot fired, 2 = shot recieved].
4 = the value of the previous key [if i recieved a shot from gun with id 66, then the value would be 66].
5 = always 0.

example
[1,3,2,23,0] - i am HELMET 3, i have been shot by gun 23
[2,23,1,0,0] - i am GUN 23, I have fired a shot


*/

uint32_t next_package[5] = {0};

void send_value()
{
  next_package[0] = 12;
  value_to_send = true;
}

void sendArrayViaBLE(uint32_t *dataArray)
{
  // Calculate the total number of bytes required
  size_t arraySize = sizeof(dataArray) / sizeof(dataArray[0]);
  size_t dataSize = arraySize * sizeof(uint32_t);

  // Allocate a buffer to hold the data
  uint8_t *dataBuffer = new uint8_t[dataSize];

  // Copy the array into the buffer
  memcpy(dataBuffer, dataArray, dataSize);

  // Call BLECharacteristic::setValue with the buffer and size
  pCharacteristic->setValue(dataBuffer, dataSize);
  pCharacteristic->notify(); // Notify connected devices (if desired)

  // Clean up the allocated buffer
  delete[] dataBuffer;
}

void handle_connections()
{
  // disconnecting
  if (!deviceConnected && oldDeviceConnected)
  {
    delay(500);                  // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected)
  {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}

void health_tick()
{
  Serial.print("Sending health tick value:");
  Serial.println(health_tick_value);

  // Example array of uint32_t values
  uint32_t dataArray[5] = {
      health_tick_value,
      0,
      0,
      0,
      health_tick_value,
  };

  // Send the array via BLE
  sendArrayViaBLE(dataArray);

  // pCharacteristic->setValue((uint8_t *)&health_tick_value, 4);
  // pCharacteristic->notify();

  health_tick_value++;
  if (health_tick_value > 99)
  {
    health_tick_value = 1;
  }
}

int l = 0;
void loop()
{
  l++;

  // notify changed value
  if (deviceConnected)
  {
    Serial.println("device connected");
    Serial.println(l);
    health_tick();

    // delay(3); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    delay(1000);
  }
  handle_connections();
  if (value_to_send)
  {

    next_package[5] = {0};
    value_to_send = false;
  }
}
