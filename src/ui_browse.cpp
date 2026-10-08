// Browse mode: the whole middle is one swipe pad (concentric rings).
#include "ui.h"
#include "ui_draw.h"

namespace ui {

using namespace draw;

namespace {
constexpr int kPadCx = 233, kPadCy = 225;
}

void drawBrowse(Action flash) {
  bool isDir = flash == Action::Up || flash == Action::Down || flash == Action::Left ||
               flash == Action::Right;

  canvas.drawCircle(kPadCx, kPadCy, 135, kRing1);
  canvas.drawCircle(kPadCx, kPadCy, 100, kRing2);
  canvas.drawCircle(kPadCx, kPadCy, 65, kRing3);

  if (isDir) {
    // Light the outer ring on the side of the swipe.
    // Arc angles here count from 3 o'clock, clockwise.
    int mid = flash == Action::Right ? 360 : flash == Action::Down ? 90
            : flash == Action::Left ? 180 : 270;
    if (mid == 360) {
      canvas.fillArc(kPadCx, kPadCy, 137, 132, 320, 360, kAccent);
      canvas.fillArc(kPadCx, kPadCy, 137, 132, 0, 40, kAccent);
    } else {
      canvas.fillArc(kPadCx, kPadCy, 137, 132, mid - 40, mid + 40, kAccent);
    }
    chevron(kPadCx, kPadCy, flash, kAccent);
  } else if (flash == Action::Ok) {
    canvas.fillSmoothCircle(kPadCx, kPadCy, 64, 0x0F2A26u);
    centeredText("OK", kPadCx, kPadCy, &fonts::FreeSansBold24pt7b, kAccent);
  } else {
    centeredText("swipe", kPadCx, kPadCy - 12, &fonts::FreeSansBold18pt7b, kText);
    centeredText("tap to select", kPadCx, kPadCy + 22, &fonts::FreeSans12pt7b, kHint);
  }
  centeredText("hold for playback", kPadCx, kPadCy + 112, &fonts::FreeSans9pt7b, 0x5A5A5Au);
}

}  // namespace ui
