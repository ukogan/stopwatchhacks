#pragma once

#include "actions.h"

// Bluetooth keyboard + media-key device that Google TV pairs with as a remote.
namespace ble_remote {

void begin(const char* deviceName);
bool isConnected();
bool hasPairedBefore();  // true once any TV has bonded with the watch
void send(Action action);

}  // namespace ble_remote
