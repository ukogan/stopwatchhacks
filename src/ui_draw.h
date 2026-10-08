#pragma once

// Shared drawing helpers and colors for the screens in ui_*.cpp.
#include <M5Unified.h>

#include "actions.h"

namespace ui::draw {

// Colors from the design canvas (RGB888).
constexpr uint32_t kBlack = 0x000000u;
constexpr uint32_t kText = 0xE8E8E8u;
constexpr uint32_t kHint = 0x7C7C7Cu;
constexpr uint32_t kRing1 = 0x1C1C1Cu, kRing2 = 0x262626u, kRing3 = 0x333333u;
constexpr uint32_t kButtonFill = 0x0E0E0Eu;
constexpr uint32_t kButtonEdge = 0x333333u;
constexpr uint32_t kAccent = 0x5ED6C4u;      // teal: volume edge and lit controls
constexpr uint32_t kAccentDark = 0x16302Cu;  // volume edge track
constexpr uint32_t kYellow = 0xF5C542u;
constexpr uint32_t kBlue = 0x4A90E2u;
constexpr uint32_t kRed = 0xE5484Du;

// Screen center (the panel is 466-468 px square).
constexpr int kCx = 233, kCy = 233;

extern M5Canvas canvas;

// Ring segment between clock angles a0..a1 (degrees from 12 o'clock, clockwise).
void clockArc(int r0, int r1, int a0, int a1, uint32_t c);
// Point at clock angle `deg`, radius r from the screen center.
void clockPoint(int deg, int r, int* x, int* y);

void line(int x0, int y0, int x1, int y1, uint32_t c, float w = 2.2f);
void ringButton(int cx, int cy, int r, uint32_t edge, uint32_t fill);

void iconBack(int cx, int cy, uint32_t c);
void iconHome(int cx, int cy, uint32_t c);
void iconPower(int cx, int cy, uint32_t c);
void iconSpeaker(int cx, int cy, uint32_t c);
void iconPlayPause(int cx, int cy, uint32_t c, int size);
void chevron(int cx, int cy, Action dir, uint32_t c);

// Thick colored arcs at the rim next to each physical button, with its icon.
void sideButtonMarkers();

void centeredText(const char* s, int x, int y, const lgfx::IFont* font, uint32_t c);

}  // namespace ui::draw
