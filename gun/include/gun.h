#ifndef GUN_H
#define GUN_H

#include <BLEDevice.h>
#include <BLEServer.h>

void gun_setup(BLEService *pService);
void gun_tick();

#endif