#include "ble_remote.h"

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>

namespace ble_remote {
namespace {

constexpr uint8_t kKeyboardReportId = 1;
constexpr uint8_t kConsumerReportId = 2;

// Report 1: standard boot-style keyboard (arrows, Enter).
// Report 2: one 16-bit consumer-control usage (Back, Home, volume, media, power).
const uint8_t kReportMap[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, kKeyboardReportId,
    0x05, 0x07,        //   Usage Page (Keyboard)
    0x19, 0xE0, 0x29, 0xE7,  // Modifiers
    0x15, 0x00, 0x25, 0x01,
    0x75, 0x01, 0x95, 0x08,
    0x81, 0x02,        //   Input (Data, Var, Abs)
    0x95, 0x01, 0x75, 0x08,
    0x81, 0x01,        //   Input (Const) reserved byte
    0x95, 0x06, 0x75, 0x08,
    0x15, 0x00, 0x25, 0x65,
    0x05, 0x07, 0x19, 0x00, 0x29, 0x65,
    0x81, 0x00,        //   Input (Data, Array) 6 key slots
    0xC0,              // End Collection

    0x05, 0x0C,        // Usage Page (Consumer)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, kConsumerReportId,
    0x15, 0x00, 0x26, 0xFF, 0x03,  // Logical 0..1023
    0x19, 0x00, 0x2A, 0xFF, 0x03,  // Usage 0..1023
    0x75, 0x10, 0x95, 0x01,
    0x81, 0x00,        //   Input (Data, Array)
    0xC0,              // End Collection
};

// HID keyboard usage IDs.
constexpr uint8_t kKeyRight = 0x4F;
constexpr uint8_t kKeyLeft = 0x50;
constexpr uint8_t kKeyDown = 0x51;
constexpr uint8_t kKeyUp = 0x52;
constexpr uint8_t kKeyEnter = 0x28;

// HID consumer usage IDs. Android maps these to BACK, HOME, POWER, etc.
constexpr uint16_t kCcPower = 0x0030;
constexpr uint16_t kCcPlayPause = 0x00CD;
constexpr uint16_t kCcMute = 0x00E2;
constexpr uint16_t kCcVolUp = 0x00E9;
constexpr uint16_t kCcVolDown = 0x00EA;
constexpr uint16_t kCcHome = 0x0223;
constexpr uint16_t kCcBack = 0x0224;
constexpr uint16_t kCcCaptions = 0x0061;  // "Closed Caption"; Android support unverified

NimBLEHIDDevice* hid = nullptr;
NimBLECharacteristic* keyboardInput = nullptr;
NimBLECharacteristic* consumerInput = nullptr;
volatile bool connected = false;

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer*, ble_gap_conn_desc*) override {
    connected = true;
    Serial.println("[ble] connected");
  }
  void onDisconnect(NimBLEServer*) override {
    connected = false;
    Serial.println("[ble] disconnected, advertising again");
    NimBLEDevice::startAdvertising();
  }
  void onAuthenticationComplete(ble_gap_conn_desc* desc) override {
    Serial.printf("[ble] auth complete, bonded=%d encrypted=%d\n",
                  desc->sec_state.bonded, desc->sec_state.encrypted);
  }
};

void sendKey(uint8_t keycode) {
  uint8_t report[8] = {0, 0, keycode, 0, 0, 0, 0, 0};
  keyboardInput->setValue(report, sizeof(report));
  keyboardInput->notify();
  delay(12);
  uint8_t release[8] = {0};
  keyboardInput->setValue(release, sizeof(release));
  keyboardInput->notify();
}

void sendConsumer(uint16_t usage) {
  uint8_t report[2] = {uint8_t(usage & 0xFF), uint8_t(usage >> 8)};
  consumerInput->setValue(report, sizeof(report));
  consumerInput->notify();
  delay(12);
  uint8_t release[2] = {0, 0};
  consumerInput->setValue(release, sizeof(release));
  consumerInput->notify();
}

}  // namespace

void begin(const char* deviceName) {
  NimBLEDevice::init(deviceName);
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  NimBLEDevice::setSecurityAuth(true, false, true);  // bond, no PIN, secure connections
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);

  NimBLEServer* server = NimBLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  hid = new NimBLEHIDDevice(server);
  keyboardInput = hid->inputReport(kKeyboardReportId);
  consumerInput = hid->inputReport(kConsumerReportId);
  hid->manufacturer()->setValue("M5Stack");
  hid->pnp(0x02, 0x303A, 0x4002, 0x0100);  // USB vendor source, Espressif VID
  hid->hidInfo(0x00, 0x01);
  hid->reportMap((uint8_t*)kReportMap, sizeof(kReportMap));
  hid->setBatteryLevel(100);
  hid->startServices();

  NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
  adv->setAppearance(HID_KEYBOARD);
  adv->addServiceUUID(hid->hidService()->getUUID());
  adv->setScanResponse(true);
  adv->start();
  Serial.printf("[ble] advertising as \"%s\", bonds=%d\n", deviceName,
                NimBLEDevice::getNumBonds());
}

bool isConnected() { return connected; }

bool hasPairedBefore() { return NimBLEDevice::getNumBonds() > 0; }

void send(Action action) {
  if (!connected) return;
  switch (action) {
    case Action::Up:        sendKey(kKeyUp); break;
    case Action::Down:      sendKey(kKeyDown); break;
    case Action::Left:      sendKey(kKeyLeft); break;
    case Action::Right:     sendKey(kKeyRight); break;
    case Action::Ok:        sendKey(kKeyEnter); break;
    case Action::Back:      sendConsumer(kCcBack); break;
    case Action::Home:      sendConsumer(kCcHome); break;
    case Action::Power:     sendConsumer(kCcPower); break;
    case Action::VolUp:     sendConsumer(kCcVolUp); break;
    case Action::VolDown:   sendConsumer(kCcVolDown); break;
    case Action::Mute:      sendConsumer(kCcMute); break;
    case Action::PlayPause: sendConsumer(kCcPlayPause); break;
    case Action::SkipBack:    sendKey(kKeyLeft); break;
    case Action::SkipForward: sendKey(kKeyRight); break;
    case Action::Captions:    sendConsumer(kCcCaptions); break;
    case Action::None:      break;
  }
}

}  // namespace ble_remote
