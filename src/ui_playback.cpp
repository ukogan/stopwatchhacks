// Playback mode: play/pause with skip buttons, captions, volume on the right edge.
#include <math.h>

#include "ui.h"
#include "ui_draw.h"

namespace ui {

using namespace draw;

namespace {

// Volume edge: a band along the rim from 2:36 to 4:56 (clock degrees 78..148).
constexpr int kVolFromDeg = 78, kVolToDeg = 148;
constexpr int kVolInnerR = 183, kVolOuterR = 227, kVolMidR = 205;
constexpr int kVolUpDeg = 86, kMuteDeg = 113, kVolDownDeg = 140;
constexpr int kMuteR = 22;

constexpr int kCcX = 201, kCcY = 34, kCcW = 64, kCcH = 44;
constexpr int kSkipBackX = 116, kSkipFwdX = 328, kRowY = 232, kSkipR = 36;
constexpr int kPlayX = 222, kPlayR = 54;
constexpr int kPillX = 163, kPillY = 388, kPillW = 140, kPillH = 46;

uint32_t edgeFor(Action flash, Action mine) { return flash == mine ? kAccent : kButtonEdge; }

void drawSkip(int cx, bool forward, bool lit) {
  ringButton(cx, kRowY, kSkipR, lit ? kAccent : kButtonEdge, kButtonFill);
  // Circular arrow: 3/4 ring plus an arrowhead (gfx angles from 3 o'clock).
  int ax = cx, ay = kRowY - 6;
  if (forward) {
    canvas.fillArc(ax, ay, 13, 10, 300, 360, kText);
    canvas.fillArc(ax, ay, 13, 10, 0, 210, kText);
    canvas.fillTriangle(ax + 7, ay - 15, ax + 15, ay - 6, ax + 4, ay - 4, kText);
  } else {
    canvas.fillArc(ax, ay, 13, 10, 330, 360, kText);
    canvas.fillArc(ax, ay, 13, 10, 0, 240, kText);
    canvas.fillTriangle(ax - 7, ay - 15, ax - 15, ay - 6, ax - 4, ay - 4, kText);
  }
  centeredText("10", cx, kRowY + 20, &fonts::FreeSansBold9pt7b, kText);
}

void drawVolumeEdge(Action flash) {
  clockArc(kVolInnerR, kVolOuterR, kVolFromDeg, kVolToDeg, kAccentDark);
  int x, y;
  for (int end : {kVolFromDeg, kVolToDeg}) {
    clockPoint(end, kVolMidR, &x, &y);
    canvas.fillSmoothCircle(x, y, (kVolOuterR - kVolInnerR) / 2, kAccentDark);
  }
  if (flash == Action::VolUp) clockArc(kVolInnerR, kVolOuterR, kVolFromDeg, kVolUpDeg + 12, kAccent);
  if (flash == Action::VolDown) clockArc(kVolInnerR, kVolOuterR, kVolDownDeg - 12, kVolToDeg, kAccent);

  clockPoint(kVolUpDeg, kVolMidR, &x, &y);  // +
  line(x - 9, y, x + 9, y, flash == Action::VolUp ? kBlack : kAccent, 3.4f);
  line(x, y - 9, x, y + 9, flash == Action::VolUp ? kBlack : kAccent, 3.4f);

  clockPoint(kMuteDeg, kVolMidR, &x, &y);  // mute
  ringButton(x, y, kMuteR, kAccent, flash == Action::Mute ? kAccent : kBlack);
  uint32_t ink = flash == Action::Mute ? kBlack : kAccent;
  iconSpeaker(x - 2, y, ink);
  line(x + 4, y - 5, x + 11, y + 5, ink, 2.0f);
  line(x + 4, y + 5, x + 11, y - 5, ink, 2.0f);

  clockPoint(kVolDownDeg, kVolMidR, &x, &y);  // -
  line(x - 9, y, x + 9, y, flash == Action::VolDown ? kBlack : kAccent, 3.4f);
}

bool inCircle(int x, int y, int cx, int cy, int r) {
  int dx = x - cx, dy = y - cy;
  return dx * dx + dy * dy <= r * r;
}

bool inRect(int x, int y, int rx, int ry, int rw, int rh, int pad) {
  return x >= rx - pad && x <= rx + rw + pad && y >= ry - pad && y <= ry + rh + pad;
}

}  // namespace

void drawPlayback(Action flash) {
  drawVolumeEdge(flash);

  canvas.fillSmoothRoundRect(kCcX, kCcY, kCcW, kCcH, 12, edgeFor(flash, Action::Captions));
  canvas.fillSmoothRoundRect(kCcX + 2, kCcY + 2, kCcW - 4, kCcH - 4, 10, kButtonFill);
  centeredText("CC", kCcX + kCcW / 2, kCcY + kCcH / 2, &fonts::FreeSansBold12pt7b, kText);

  drawSkip(kSkipBackX, false, flash == Action::SkipBack);
  drawSkip(kSkipFwdX, true, flash == Action::SkipForward);
  canvas.fillSmoothCircle(kPlayX, kRowY, kPlayR, flash == Action::PlayPause ? kAccent : 0xEDEDEDu);
  iconPlayPause(kPlayX, kRowY, kBlack, 22);

  centeredText("or swipe to skip", 233, 310, &fonts::FreeSans9pt7b, kHint);

  canvas.fillSmoothRoundRect(kPillX, kPillY, kPillW, kPillH, kPillH / 2, 0x444444u);
  canvas.fillSmoothRoundRect(kPillX + 1, kPillY + 1, kPillW - 2, kPillH - 2, kPillH / 2 - 1, kButtonFill);
  centeredText("Browse", kPillX + kPillW / 2, kPillY + kPillH / 2, &fonts::FreeSansBold12pt7b, kText);
}

Zone playbackZoneAt(int x, int y) {
  int dx = x - kCx, dy = y - kCy;
  int dist = (int)sqrtf((float)(dx * dx + dy * dy));
  int deg = ((int)lroundf(atan2f((float)dx, (float)-dy) * 180.0f / (float)M_PI) + 360) % 360;
  if (dist >= kVolInnerR - 15 && deg >= kVolFromDeg - 8 && deg <= kVolToDeg + 8) {
    if (deg < (kVolUpDeg + kMuteDeg) / 2) return Zone::VolUp;
    if (deg < (kMuteDeg + kVolDownDeg) / 2) return Zone::Mute;
    return Zone::VolDown;
  }
  if (inRect(x, y, kCcX, kCcY, kCcW, kCcH, 10)) return Zone::Captions;
  // Fingers land below the drawn pill (logs show taps at y 440-455), so the
  // pill's touch area runs to the bottom edge of the screen.
  if (x >= kPillX - 20 && x <= kPillX + kPillW + 20 && y >= kPillY - 10) return Zone::BrowsePill;
  if (inCircle(x, y, kPlayX, kRowY, kPlayR + 6)) return Zone::Play;
  if (inCircle(x, y, kSkipBackX, kRowY, kSkipR + 10)) return Zone::SkipBack;
  if (inCircle(x, y, kSkipFwdX, kRowY, kSkipR + 10)) return Zone::SkipForward;
  return Zone::Pad;
}

}  // namespace ui
