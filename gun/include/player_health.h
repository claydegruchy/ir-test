#ifndef PLAYER_HEALTH_H
#define PLAYER_HEALTH_H

#include <WiFi.h>
#include <esp_now.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>

void player_health_setup(BLEService *pService);

#endif