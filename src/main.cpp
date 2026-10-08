#ifndef AUDIO_TEST  // the speaker test (audio_test.cpp) has its own setup/loop

// M5Stack StopWatch as a Bluetooth Google TV remote.
// Two screen modes:
//   Browse:   the middle is a swipe pad (arrows / OK). Hold it to go to Playback.
//   Playback: play/pause, skip back/forward, captions, volume on the right edge.
//             The Browse pill, Back or Home return to Browse.
// Side buttons: yellow = Back, blue = Home (hold = Mute), red = Power.

#include <M5Unified.h>

#include "actions.h"
#include "ble_remote.h"
#include "ui.h"

namespace {

constexpr const char* kDeviceName = "StopWatch Remote";
constexpr int kSwipeMinPx = 40;          // shorter movement counts as a tap
constexpr uint32_t kHoldMs = 600;        // Browse: hold the pad this long for Playback
constexpr uint32_t kFlashMs = 260;       // how long a control stays lit
constexpr uint32_t kRepeatDelayMs = 400; // volume: hold this long before repeating
constexpr uint32_t kRepeatEveryMs = 150; // volume: then one step this often
constexpr uint32_t kBuzzMs = 30;
constexpr uint8_t kBuzzLevel = 170;
constexpr uint32_t kDimAfterMs = 30000;
constexpr uint32_t kSleepAfterMs = 90000;
constexpr uint8_t kBrightNormal = 160;
constexpr uint8_t kBrightDim = 30;

enum class Screen { Unknown, Pairing, Remote };

Screen screen = Screen::Unknown;
ui::Mode mode = ui::Mode::Browse;
Action flash = Action::None;
uint32_t flashUntil = 0;
uint32_t buzzUntil = 0;
uint32_t lastInputMs = 0;
bool dimmed = false;
bool asleep = false;

ui::Zone touchZone = ui::Zone::None;
int touchStartX = 0, touchStartY = 0;
uint32_t nextRepeatMs = 0;
bool touchConsumed = false;
uint32_t touchStartMs = 0;  // a long-press already acted; ignore its release

void buzz() {
  M5.Power.setVibration(kBuzzLevel);
  buzzUntil = millis() + kBuzzMs;
}

const char* actionName(Action a) {
  switch (a) {
    case Action::Up: return "up";       case Action::Down: return "down";
    case Action::Left: return "left";   case Action::Right: return "right";
    case Action::Ok: return "ok";       case Action::Back: return "back";
    case Action::Home: return "home";   case Action::Power: return "power";
    case Action::VolUp: return "vol+";  case Action::VolDown: return "vol-";
    case Action::Mute: return "mute";   case Action::PlayPause: return "play";
    case Action::SkipBack: return "skip-back";  case Action::SkipForward: return "skip-fwd";
    case Action::Captions: return "captions";   case Action::None: return "none";
  }
  return "?";
}

void perform(Action action) {
  if (action == Action::None) return;
  Serial.printf("[main] action %s\n", actionName(action));
  buzz();
  ble_remote::send(action);
  if (action == Action::Back || action == Action::Home) mode = ui::Mode::Browse;
  flash = action;
  flashUntil = millis() + kFlashMs;
  ui::drawRemote(mode, flash);
}

void switchMode(ui::Mode to) {
  Serial.printf("[main] mode %s\n", to == ui::Mode::Browse ? "browse" : "playback");
  mode = to;
  buzz();
  flash = Action::None;
  ui::drawRemote(mode, Action::None);
}

Action swipeAction(int dx, int dy) {
  if (abs(dx) < kSwipeMinPx && abs(dy) < kSwipeMinPx) {
    return mode == ui::Mode::Browse ? Action::Ok : Action::None;
  }
  if (abs(dx) > abs(dy)) {
    if (mode == ui::Mode::Playback) return dx > 0 ? Action::SkipForward : Action::SkipBack;
    return dx > 0 ? Action::Right : Action::Left;
  }
  return dy > 0 ? Action::Down : Action::Up;
}

Action volumeAction(ui::Zone zone) {
  return zone == ui::Zone::VolUp ? Action::VolUp : Action::VolDown;
}

Action touchAction(uint32_t now) {
  if (M5.Touch.getCount() == 0 && touchZone == ui::Zone::None) return Action::None;
  auto t = M5.Touch.getDetail();
  bool onVolume = touchZone == ui::Zone::VolUp || touchZone == ui::Zone::VolDown;
  if (t.wasPressed()) {
    touchZone = ui::zoneAt(mode, t.x, t.y);
    touchConsumed = false;
    Serial.printf("[touch] down x=%d y=%d zone=%d mode=%d\n", t.x, t.y, (int)touchZone, (int)mode);
    touchStartX = t.x;
    touchStartY = t.y;
    touchStartMs = now;
    // Volume steps on touch-down, then repeats while held.
    if (touchZone == ui::Zone::VolUp || touchZone == ui::Zone::VolDown) {
      nextRepeatMs = now + kRepeatDelayMs;
      return volumeAction(touchZone);
    }
    return Action::None;
  }
  if (onVolume && t.isPressed()) {
    if (now < nextRepeatMs) return Action::None;
    nextRepeatMs = now + kRepeatEveryMs;
    return volumeAction(touchZone);
  }
  // Browse: holding the pad (small drift allowed) switches to Playback. M5Unified's
  // own hold event never fires if the finger drifts a few pixels, so time it here.
  if (t.isPressed() && !touchConsumed && mode == ui::Mode::Browse &&
      touchZone == ui::Zone::Pad && now - touchStartMs >= kHoldMs &&
      abs(t.x - touchStartX) < kSwipeMinPx && abs(t.y - touchStartY) < kSwipeMinPx) {
    touchConsumed = true;
    switchMode(ui::Mode::Playback);
    return Action::None;
  }
  if (!t.wasReleased()) return Action::None;
  if (touchConsumed) {
    touchConsumed = false;
    touchZone = ui::Zone::None;
    return Action::None;
  }

  int dx = t.x - touchStartX, dy = t.y - touchStartY;
  ui::Zone zone = touchZone;
  Serial.printf("[touch] up x=%d y=%d dx=%d dy=%d zone=%d\n", t.x, t.y, dx, dy, (int)zone);
  touchZone = ui::Zone::None;
  bool tapped = abs(dx) < kSwipeMinPx && abs(dy) < kSwipeMinPx;
  switch (zone) {
    case ui::Zone::Pad:     return swipeAction(dx, dy);
    case ui::Zone::VolUp:   return Action::None;  // already sent on touch-down
    case ui::Zone::VolDown: return Action::None;
    case ui::Zone::Mute:    return tapped ? Action::Mute : Action::None;
    case ui::Zone::Play:    return tapped ? Action::PlayPause : Action::None;
    case ui::Zone::SkipBack:    return tapped ? Action::SkipBack : Action::None;
    case ui::Zone::SkipForward: return tapped ? Action::SkipForward : Action::None;
    case ui::Zone::Captions:    return tapped ? Action::Captions : Action::None;
    case ui::Zone::BrowsePill:
      if (tapped) switchMode(ui::Mode::Browse);
      return Action::None;
    case ui::Zone::None:    return Action::None;
  }
  return Action::None;
}

// The red button is wired to the power chip, not to the ESP32. M5Unified reads
// the chip's button-event register, but on the StopWatch the bit it reacts to
// is a "wake" event that also fires on its own every few seconds. So the
// library's power-button handling is off (see setup()), and this code reads
// the chip's live button state instead: a press shorter than kRedMaxClickMs
// counts as one click, sent when the button is released.
constexpr uint8_t kPm1BtnStatus = 0x48;  // bit 0 = button down now, bit 7 = was pressed
constexpr uint8_t kPm1BtnCfg1 = 0x49;    // bit 0 = disable "single click resets"
constexpr uint8_t kPm1BtnCfg2 = 0x4A;    // bit 0 = disable "double click powers off"
constexpr uint32_t kRedMaxClickMs = 1500;
bool redPressed = false;
uint32_t nextRedPollMs = 0;

void setupRedButton() {
  auto& pm1 = M5.Power.M5pm1;
  pm1.writeRegister8(kPm1BtnCfg1, pm1.readRegister8(kPm1BtnCfg1) | 0x01);
  pm1.writeRegister8(kPm1BtnCfg2, pm1.readRegister8(kPm1BtnCfg2) | 0x01);
  Serial.printf("[main] power chip button cfg1=0x%02X cfg2=0x%02X\n",
                pm1.readRegister8(kPm1BtnCfg1), pm1.readRegister8(kPm1BtnCfg2));
}

void pollRedButton() {
  redPressed = false;
  if (millis() < nextRedPollMs) return;
  nextRedPollMs = millis() + 20;
  static bool wasDown = false;
  static uint32_t downSinceMs = 0;
  uint8_t status = M5.Power.M5pm1.readRegister8(kPm1BtnStatus);
  bool down = status & 0x01;
  if (down && !wasDown) downSinceMs = millis();
  if (!down && wasDown) {
    uint32_t heldMs = millis() - downSinceMs;
    Serial.printf("[main] red button released after %lu ms\n", (unsigned long)heldMs);
    redPressed = heldMs < kRedMaxClickMs;
  }
  wasDown = down;
}

Action buttonAction() {
  if (M5.BtnA.wasClicked()) return Action::Back;    // yellow, 10:30
  if (M5.BtnB.wasHold()) return Action::Mute;       // blue, 1:30, held
  if (M5.BtnB.wasClicked()) return Action::Home;    // blue, 1:30
  if (redPressed) return Action::Power;             // red, 7:30
  return Action::None;
}

bool anyInputStarted() {
  return M5.Touch.getDetail().wasPressed() || M5.BtnA.wasPressed() ||
         M5.BtnB.wasPressed() || redPressed;
}

void wake() {
  if (asleep) M5.Display.wakeup();
  M5.Display.setBrightness(kBrightNormal);
  asleep = dimmed = false;
}

void updateIdle(uint32_t now) {
  uint32_t idle = now - lastInputMs;
  if (!asleep && idle > kSleepAfterMs) {
    M5.Display.sleep();
    asleep = true;
  } else if (!dimmed && !asleep && idle > kDimAfterMs) {
    M5.Display.setBrightness(kBrightDim);
    dimmed = true;
  }
}

void showScreenForConnection() {
  Screen want = ble_remote::isConnected() ? Screen::Remote : Screen::Pairing;
  if (want == screen) return;
  screen = want;
  if (asleep || dimmed) wake();
  lastInputMs = millis();
  if (screen == Screen::Remote) ui::drawRemote(mode, Action::None);
  else ui::drawPairing(ble_remote::hasPairedBefore());
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  cfg.pmic_button = false;  // see pollRedButton()
  M5.begin(cfg);
  Serial.begin(115200);
  Serial.printf("[main] board=%d display=%dx%d\n", (int)M5.getBoard(),
                M5.Display.width(), M5.Display.height());
  setupRedButton();
  ui::begin();
  ble_remote::begin(kDeviceName);
  lastInputMs = millis();
  showScreenForConnection();
}

void loop() {
  M5.update();
  pollRedButton();
  uint32_t now = millis();

  if (buzzUntil && now >= buzzUntil) {
    M5.Power.setVibration(0);
    buzzUntil = 0;
  }

  showScreenForConnection();

  // While the screen is dim or off, the first touch or press only wakes it.
  if (asleep || dimmed) {
    if (anyInputStarted()) {
      wake();
      lastInputMs = now;
      touchZone = ui::Zone::None;
      if (screen == Screen::Remote) ui::drawRemote(mode, Action::None);
      else ui::drawPairing(ble_remote::hasPairedBefore());
    }
    delay(10);
    return;
  }

  Action action = touchAction(now);
  if (action == Action::None) action = buttonAction();
  if (action != Action::None || M5.Touch.getCount() > 0) lastInputMs = now;

  if (screen == Screen::Remote) {
    perform(action);
    if (flash != Action::None && now >= flashUntil) {
      flash = Action::None;
      ui::drawRemote(mode, Action::None);
    }
  }

  updateIdle(now);
  delay(5);
}

#endif  // AUDIO_TEST
