#include "ui.h"

#include "ui_draw.h"

namespace ui {

using namespace draw;

void begin() {
  M5.Display.setBrightness(160);
  canvas.setPsram(true);
  canvas.setColorDepth(16);
  canvas.createSprite(M5.Display.width(), M5.Display.height());
}

void drawRemote(Mode mode, Action flash) {
  canvas.fillScreen(kBlack);
  sideButtonMarkers();
  if (mode == Mode::Browse) drawBrowse(flash);
  else drawPlayback(flash);
  canvas.pushSprite(0, 0);
}

Zone zoneAt(Mode mode, int x, int y) {
  return mode == Mode::Browse ? Zone::Pad : playbackZoneAt(x, y);
}

void drawPairing(bool reconnecting) {
  canvas.fillScreen(kBlack);

  // Bluetooth rune in a ring.
  const int cx = 233, cy = 120;
  ringButton(cx, cy, 36, kAccent, kBlack);
  line(cx - 9, cy - 9, cx + 9, cy + 9, kAccent, 2.0f);
  line(cx + 9, cy + 9, cx, cy + 18, kAccent, 2.0f);
  line(cx, cy + 18, cx, cy - 18, kAccent, 2.0f);
  line(cx, cy - 18, cx + 9, cy - 9, kAccent, 2.0f);
  line(cx + 9, cy - 9, cx - 9, cy + 9, kAccent, 2.0f);

  const uint32_t body = 0xB0B0B0u;
  if (reconnecting) {
    centeredText("Reconnecting", 233, 200, &fonts::FreeSansBold18pt7b, kText);
    centeredText("Turn the TV on, or", 233, 250, &fonts::FreeSans12pt7b, body);
    centeredText("pair again from its", 233, 280, &fonts::FreeSans12pt7b, body);
    centeredText("Remotes settings", 233, 310, &fonts::FreeSans12pt7b, body);
  } else {
    centeredText("Pair with your TV", 233, 200, &fonts::FreeSansBold18pt7b, kText);
    centeredText("On the TV, open Settings >", 233, 250, &fonts::FreeSans12pt7b, body);
    centeredText("Remotes & Accessories >", 233, 280, &fonts::FreeSans12pt7b, body);
    centeredText("Pair, then pick", 233, 310, &fonts::FreeSans12pt7b, body);
  }
  centeredText("StopWatch Remote", 233, 350, &fonts::FreeSansBold12pt7b, kAccent);
  centeredText("waiting for TV...", 233, 390, &fonts::FreeSans9pt7b, 0x6E6E6Eu);

  canvas.pushSprite(0, 0);
}

}  // namespace ui
