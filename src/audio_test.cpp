// Speaker test firmware (build with `pio run -e audiotest`).
// Tap Voice / Music / Sweep to play a clip; - and + change the volume.
#ifdef AUDIO_TEST

#include <M5Unified.h>

#include "audio_samples.h"

namespace {

struct Pad {
  const char* label;
  int x, y, w, h;
};

constexpr Pad kVoice = {"Voice", 93, 120, 136, 100};
constexpr Pad kBtnMusic = {"Song", 237, 120, 136, 100};
constexpr Pad kBtnSweep = {"Sweep", 93, 228, 136, 100};
constexpr Pad kStop = {"Stop", 237, 228, 136, 100};
constexpr Pad kVolDown = {"-", 133, 344, 90, 64};
constexpr Pad kVolUp = {"+", 243, 344, 90, 64};

uint8_t volume = 160;

bool hit(const Pad& p, int x, int y) {
  return x >= p.x && x < p.x + p.w && y >= p.y && y < p.y + p.h;
}

void drawPad(const Pad& p, uint32_t fill) {
  M5.Display.fillRoundRect(p.x, p.y, p.w, p.h, 18, fill);
  M5.Display.setTextColor(0xFFFFFFu);
  M5.Display.setTextDatum(middle_center);
  M5.Display.setFont(&fonts::FreeSansBold18pt7b);
  M5.Display.drawString(p.label, p.x + p.w / 2, p.y + p.h / 2);
}

void drawScreen() {
  M5.Display.fillScreen(0x000000u);
  M5.Display.setTextColor(0xBBBBBBu);
  M5.Display.setTextDatum(middle_center);
  M5.Display.setFont(&fonts::FreeSansBold12pt7b);
  M5.Display.drawString("Speaker test", 233, 70);
  M5.Display.setFont(&fonts::FreeSans9pt7b);
  M5.Display.drawString(String("volume ") + volume + " / 255", 233, 98);
  drawPad(kVoice, 0x2F6FC2u);
  drawPad(kBtnMusic, 0x2E8B57u);
  drawPad(kBtnSweep, 0x8A5A00u);
  drawPad(kStop, 0x7A2226u);
  drawPad(kVolDown, 0x333333u);
  drawPad(kVolUp, 0x333333u);
}

void play(const int16_t* data, size_t len, uint32_t rate = kSampleRate) {
  M5.Speaker.stop();
  M5.Speaker.playRaw(data, len, rate, false, 1, 0, true);
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  cfg.pmic_button = false;
  M5.begin(cfg);
  Serial.begin(115200);
  M5.Display.setBrightness(160);
  M5.Speaker.begin();
  M5.Speaker.setVolume(volume);
  Serial.printf("[audio] speaker enabled=%d\n", M5.Speaker.isEnabled());
  drawScreen();
}

void loop() {
  M5.update();
  auto t = M5.Touch.getDetail();
  if (t.wasPressed()) {
    int x = t.x, y = t.y;
    if (hit(kVoice, x, y)) play(kSpeech, sizeof(kSpeech) / sizeof(kSpeech[0]));
    else if (hit(kBtnMusic, x, y)) play(kMusic, sizeof(kMusic) / sizeof(kMusic[0]), kMusicRate);
    else if (hit(kBtnSweep, x, y)) play(kSweep, sizeof(kSweep) / sizeof(kSweep[0]));
    else if (hit(kStop, x, y)) M5.Speaker.stop();
    else if (hit(kVolDown, x, y) || hit(kVolUp, x, y)) {
      int v = volume + (hit(kVolUp, x, y) ? 32 : -32);
      volume = v < 0 ? 0 : v > 255 ? 255 : v;
      M5.Speaker.setVolume(volume);
      drawScreen();
    }
  }
  delay(5);
}

#endif  // AUDIO_TEST
