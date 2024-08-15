#ifndef PLAYER_HEALTH_H
#define PLAYER_HEALTH_H

#include <WiFi.h>
#include <esp_now.h>

#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>

void player_health_setup(BLEService *pService);
void player_health_tick(int loopNumber);

#endif