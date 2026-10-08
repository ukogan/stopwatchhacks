#pragma once

// Everything the remote can ask the TV to do.
enum class Action {
  None,
  Up,
  Down,
  Left,
  Right,
  Ok,
  Back,
  Home,
  Power,
  VolUp,
  VolDown,
  Mute,
  PlayPause,
  SkipBack,     // playback: jump back (sent as Left)
  SkipForward,  // playback: jump forward (sent as Right)
  Captions,
};
