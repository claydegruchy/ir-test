#ifndef HEALTH_CHECK_H
#define HEALTH_CHECK_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>

void health_setup(BLEService *pService);
void health_tick();

#endif