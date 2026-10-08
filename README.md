# stopwatchhacks

Turns the M5Stack StopWatch (a round touchscreen watch) into a Bluetooth remote for Google TV. No hub, app or account is needed. The TV pairs with the watch the same way it pairs with any Bluetooth remote.

## What it does

- **Browse mode.** The middle of the screen is a swipe pad. Swipe to move through menus, and tap to select.
- **Playback mode.** Hold the swipe pad for about half a second to switch. This mode has play/pause, jump back and forward, subtitles, and volume along the right edge of the screen.
- **Side buttons.** The colored arcs on screen point at the physical buttons.
  - Yellow (10:30): Back.
  - Blue (1:30): Home. Hold it to mute.
  - Red (7:30): TV power. Holding red for several seconds turns the watch off.
- Every action makes the watch buzz.
- The screen dims after 30 seconds and turns off after 90 seconds. The first touch only wakes it.

## Setting it up

1. Install PlatformIO. On a Mac: `brew install platformio`.
2. Connect the watch by USB, then run `pio run -e stopwatch -t upload`.
3. On the TV, open Settings, then Remotes & Accessories, then Pair remote or accessory. Choose "StopWatch Remote".

Loading this code replaces the watch's factory software. To keep a copy of the factory software, back it up before step 2. The command is under Technical details.

## Known limits

- The watch can only send commands to the TV. Over Bluetooth, the TV sends nothing back, so the watch can't tell what is playing.
- Some apps may ignore the subtitles button.
- The speaker is good enough for alert sounds, not for music.

## Technical details

**Hardware:** M5Stack StopWatch (C152). It uses an ESP32-S3 chip with 16 MB of storage and 8 MB of extra memory. The screen is a 466x466 round AMOLED touchscreen. A power chip handles the red button, and the yellow and blue buttons connect to the chip's pins 2 and 1.

**Build:** PlatformIO, `espressif32 @ 6.12.0`, Arduino framework, board `esp32s3box`. The libraries are M5Unified and M5GFX (from GitHub) and NimBLE-Arduino 1.4.x.

**Code layout:**

| File | Role |
|---|---|
| `src/main.cpp` | Input handling, browse/playback mode switching, dimming, red-button workaround |
| `src/ble_remote.*` | Bluetooth keyboard with a media-key report; sends the remote's commands to the TV |
| `src/ui*.cpp` | Screen drawing: shared helpers, Browse mode, Playback mode, pairing screen |
| `src/actions.h` | The list of remote commands |
| `src/audio_test.cpp` | Optional speaker test (`pio run -e audiotest -t upload`) |
| `tools/make_audio_samples.py` | Builds `src/audio_samples.h` for the speaker test |

**Bluetooth:** One report sends keyboard keys: the arrow keys for moving and skipping, and Enter for OK. A second report sends media keys: Back, Home, Power, Volume up and down, Mute, Play/Pause, and Closed Caption. Pairing uses bonding with secure connections and no PIN.

**Red button:** The red button is wired to the power chip at I2C address 0x6E. M5Unified reads the chip's button events, but the bit it reacts to is a wake event that also fires by itself every few seconds. That made the TV switch off and on in a loop. The library's power-button handling is turned off (`cfg.pmic_button = false`). Instead, the code reads the button's live state (register 0x48, bit 0) and counts a press shorter than 1.5 s as one click. Startup also turns off the chip's "single click resets" and "double click powers off" settings (registers 0x49 and 0x4A, bit 0).

**Touch:** M5Unified's touch-hold event never fires if the finger drifts a few pixels, so the code times the hold itself (600 ms, up to 40 px of drift). Taps on the bottom button tend to land below the drawn shape, so its touch area reaches the screen edge.

**Speaker test:** `src/audio_samples.h` is not in the repo. Create it with:

```
say -o speech.wav --file-format=WAVE --data-format=LEI16@16000 "Hello from your StopWatch."
python3 tools/make_audio_samples.py speech.wav src/audio_samples.h [song-24k-mono.wav]
```

**Back up the factory software** (16 MB, about 3.5 minutes):

```
python3 ~/.platformio/packages/tool-esptoolpy/esptool.py --chip esp32s3 \
  --port /dev/cu.usbmodem1101 -b 921600 read_flash 0 0x1000000 factory-16MB.bin
```
