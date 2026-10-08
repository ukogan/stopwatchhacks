#pragma once

#include "actions.h"

// Draws the remote's screens (from the design canvas) on the 466x466 round AMOLED.
namespace ui {

// Browse: swipe/tap pad for moving around menus.
// Playback: play/pause, skip, captions, volume on the right edge.
enum class Mode { Browse, Playback };

enum class Zone { None, Pad, VolUp, VolDown, Mute, Play, SkipBack, SkipForward, Captions, BrowsePill };

void begin();
// `flash` lights up the control that was just used (Action::None = idle).
void drawRemote(Mode mode, Action flash);
// Shown whenever no TV is connected.
void drawPairing(bool reconnecting);
// Which on-screen control a touch at (x, y) starts on.
Zone zoneAt(Mode mode, int x, int y);

// Per-mode screens (ui_browse.cpp, ui_playback.cpp).
void drawBrowse(Action flash);
void drawPlayback(Action flash);
Zone playbackZoneAt(int x, int y);

}  // namespace ui
