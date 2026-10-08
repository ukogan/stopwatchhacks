#include "ui_draw.h"

#include <math.h>

namespace ui::draw {

M5Canvas canvas(&M5.Display);

namespace {
// LovyanGFX arcs measure from 3 o'clock; clock angles measure from 12.
int toGfx(int clockDeg) { return ((clockDeg - 90) % 360 + 360) % 360; }
}  // namespace

void clockArc(int r0, int r1, int a0, int a1, uint32_t c) {
  int g0 = toGfx(a0), g1 = g0 + (a1 - a0);
  if (g1 <= 360) {
    canvas.fillArc(kCx, kCy, r0, r1, g0, g1, c);
  } else {
    canvas.fillArc(kCx, kCy, r0, r1, g0, 360, c);
    canvas.fillArc(kCx, kCy, r0, r1, 0, g1 - 360, c);
  }
}

void clockPoint(int deg, int r, int* x, int* y) {
  float rad = deg * (float)M_PI / 180.0f;
  *x = kCx + (int)lroundf(r * sinf(rad));
  *y = kCy - (int)lroundf(r * cosf(rad));
}

void line(int x0, int y0, int x1, int y1, uint32_t c, float w) {
  canvas.drawWideLine(x0, y0, x1, y1, w, c);
}

void ringButton(int cx, int cy, int r, uint32_t edge, uint32_t fill) {
  canvas.fillSmoothCircle(cx, cy, r, edge);
  canvas.fillSmoothCircle(cx, cy, r - 2, fill);
}

void iconBack(int cx, int cy, uint32_t c) {
  // Hook arrow: line left to an arrowhead, curving down on the right.
  canvas.fillArc(cx + 2, cy + 3, 10, 7, 270, 360, c);
  canvas.fillArc(cx + 2, cy + 3, 10, 7, 0, 90, c);
  line(cx - 10, cy - 5, cx + 2, cy - 5, c, 3.0f);
  canvas.fillTriangle(cx - 14, cy - 5, cx - 7, cy - 11, cx - 7, cy + 1, c);
  line(cx + 2, cy + 11, cx - 5, cy + 11, c, 3.0f);
}

void iconHome(int cx, int cy, uint32_t c) {
  line(cx - 13, cy - 1, cx, cy - 12, c, 3.0f);
  line(cx, cy - 12, cx + 13, cy - 1, c, 3.0f);
  line(cx - 9, cy - 3, cx - 9, cy + 12, c, 3.0f);
  line(cx + 9, cy - 3, cx + 9, cy + 12, c, 3.0f);
  line(cx - 9, cy + 12, cx + 9, cy + 12, c, 3.0f);
}

void iconPower(int cx, int cy, uint32_t c) {
  canvas.fillArc(cx, cy + 2, 12, 9, 300, 360, c);
  canvas.fillArc(cx, cy + 2, 12, 9, 0, 240, c);  // open at the top
  line(cx, cy - 13, cx, cy, c, 3.0f);
}

void iconSpeaker(int cx, int cy, uint32_t c) {
  canvas.fillRect(cx - 9, cy - 4, 5, 8, c);
  canvas.fillTriangle(cx - 5, cy, cx + 2, cy - 9, cx + 2, cy + 9, c);
}

void iconPlayPause(int cx, int cy, uint32_t c, int s) {
  canvas.fillTriangle(cx - s, cy - s * 3 / 4, cx - s, cy + s * 3 / 4, cx + s / 8, cy, c);
  int bw = s / 4 + 1;
  canvas.fillRect(cx + s / 3, cy - s * 3 / 4, bw, s * 3 / 2, c);
  canvas.fillRect(cx + s / 3 + bw * 2, cy - s * 3 / 4, bw, s * 3 / 2, c);
}

void chevron(int cx, int cy, Action dir, uint32_t c) {
  const int s = 22;
  int ax, ay, bx, by, tx, ty;  // two tails and the tip
  switch (dir) {
    case Action::Right: ax = cx - s / 2; ay = cy - s; bx = ax; by = cy + s; tx = cx + s / 2; ty = cy; break;
    case Action::Left:  ax = cx + s / 2; ay = cy - s; bx = ax; by = cy + s; tx = cx - s / 2; ty = cy; break;
    case Action::Up:    ax = cx - s; ay = cy + s / 2; bx = cx + s; by = ay; tx = cx; ty = cy - s / 2; break;
    default:            ax = cx - s; ay = cy - s / 2; bx = cx + s; by = ay; tx = cx; ty = cy + s / 2; break;
  }
  line(ax, ay, tx, ty, c, 4.5f);
  line(bx, by, tx, ty, c, 4.5f);
}

void sideButtonMarkers() {
  struct Marker { int clockDeg; uint32_t color; void (*icon)(int, int, uint32_t); };
  const Marker markers[] = {
      {315, kYellow, iconBack},   // 10:30
      {45, kBlue, iconHome},      // 1:30
      {225, kRed, iconPower},     // 7:30
  };
  for (const auto& m : markers) {
    clockArc(211, 227, m.clockDeg - 13, m.clockDeg + 13, m.color);
    int x, y;
    for (int end : {m.clockDeg - 13, m.clockDeg + 13}) {  // rounded ends
      clockPoint(end, 219, &x, &y);
      canvas.fillSmoothCircle(x, y, 8, m.color);
    }
    clockPoint(m.clockDeg, 178, &x, &y);
    m.icon(x, y, m.color);
  }
}

void centeredText(const char* s, int x, int y, const lgfx::IFont* font, uint32_t c) {
  canvas.setFont(font);
  canvas.setTextColor(c);
  canvas.setTextDatum(middle_center);
  canvas.drawString(s, x, y);
}

}  // namespace ui::draw
