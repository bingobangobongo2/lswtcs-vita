<div align=center>

<img src="extras/screenshots/banner.png" alt="Banner" width="35%">

</div>

<h1 align=center>Lego Star Wars: The Complete Saga · PSVita Port</h1>

<p align=center>
  <a href="#setup-instructions-for-players">How to install</a> •
  <a href="#controls">Controls</a> •
  <a href="#known-issues">Known Issues</a> •
  <a href="#build-instructions-for-developers">How to compile</a> •
  <a href="#credits">Credits</a> •
  <a href="#license">License</a>
</p>

Lego Star Wars: The Complete Saga is a Lego-themed action-adventure video game based on the Lego Star Wars line of
construction toys. It is a combination of the game Lego Star Wars: The Video Game and its sequel, Lego Star Wars
II: The Original Trilogy, which span the first six episodes of the Skywalker Saga.

This repository contains a loader for **the Android release of Lego Star Wars: The Complete Saga v2.0.2.02**, based on
the [Android SO Loader by TheFloW][gtasa]. The loader provides a tailored, minimalistic Android-like environment to run
the official ARMv7 game executable on the PS Vita.

Disclaimer
----------

**LEGO® STAR WARS™: THE COMPLETE SAGA** software © 2022 TT Games Publishing Ltd. LEGO, the LEGO logo, the Brick and the
Knob configurations and the Minifigure are trademarks of the LEGO Group. © 2022 The LEGO Group. LucasArts and the
LucasArts logo are registered trademarks of Lucasfilm Ltd. © 2022 Lucasfilm Entertainment Company Ltd. or Lucasfilm Ltd.
® or TM as indicated, which are owned by Disney Enterprises, Inc. All rights reserved.

The work presented in this repository is not "official" or produced or sanctioned by the owner(s) of the aforementioned
trademark(s) or any other registered trademark mentioned in this repository.

This software does not contain the original code, executables, or other non-redistributable parts of the original game
product. The authors of this work do not promote or condone piracy in any way. To launch and play the game on their PS
Vita device, users must possess their own legally obtained copy of the game in the form of an .apk file.
# LEGO Star Wars: The Complete Saga — v26b (Audio Fix, Clean Controls)

**Version**: v26b (2026-03-24)
**Type**: Source build
**For**: PS Vita with HENkaku/taiHEN
**Bounty**: https://zealouschuck.com/ps-vita-bounty

## What's New in v26b (vs v26)

### Removed right stick byte overwrite
v26 had a NuPadRead hook that wrote right stick values into left stick byte positions (nupad_s offsets 0xA2-0xA3), causing the right joystick to control character movement. v26b removes all right stick byte modifications — NuPadRead hook is passthrough only.

### Audio unchanged from v26
- Int32 accumulator mixer (no per-track clipping distortion)
- SFX gain at -26dB (0.05f)
- Distance attenuation gain timing fix
- softfp ABI for OpenSLES

## Current Status

| Feature | Status |
|---------|--------|
| Graphics rendering | **Working** |
| Music playback | **Working** |
| Sound effects | **Working** (int32 mixer, -26dB SFX) |
| SFX with many NPCs | **Fixed** (no per-track clipping distortion) |
| SFX distance attenuation | **Working** (gain copy timing fix) |
| Right stick camera | Not working (upstream .so limitation) |
| Left stick movement | **Working** |
| Gamepad controls | **Working** |
| Touch input | **Working** |

## Installation

### Requirements
- PS Vita on 3.60 or 3.65 enso firmware
- kubridge.skprx v0.3.1+ (bythos14 fork) in `ur0:tai/`
- fd_fix.skprx in `ur0:tai/` (unless using rePatch)
- libshacccg.suprx in `ur0:data/`
- LEGO Star Wars TCS Android APK v2.0.2.02 (build 20202) data files

### Game Data Setup
1. Extract `lib/armeabi-v7a/libTTapp.so` from the APK -> `ux0:data/lswtcs/libTTapp.so`
2. Copy `Audio.dat`, `Levels.dat`, `Other.dat`, `Textures.dat` -> `ux0:data/lswtcs/`
3. Verify libTTapp.so SHA-1: `291321330a3789414cab7d411dae64debc990ad6`

### Install
1. Transfer `lswtcs_v26b.vpk` to Vita
2. Install via VitaShell (overwrites previous installation if present)
3. Launch from LiveArea

## Source Changes vs v26

1. **patch.c**: NuPadRead hook changed to passthrough (no byte overwrites). Removes right stick leaking into movement controls.

## Files

| File | Description |
|------|-------------|
| `lswtcs_v26b.vpk` | Ready-to-install VPK |
| `eboot_v26b.bin` | Compiled eboot binary (for FTP swap) |
| `README.md` | This file |

## Checksums

```
SHA-256:
7d6773b7697069016589a7fb9d0db01928bf56869b0cd304d8a1c9d40234638d  eboot_v26b.bin
9e3599ef6bc5da558c3270e258ef96ec40e7c20661e92ca5d8d7ac20e58d0868  lswtcs_v26b.vpk
```

## Credits

- **gm666q** — Original LSWTCS Vita port
- **Andy "The FloW" Nguyen** — Original .so loader
- **Rinnegatamante** — vitaGL, vitashark
- **Volodymyr Atamanenko** — soloader-boilerplate template
- **v24-v26b fixes** — vitaGL update, DSA fixes, GXP loading, int32 mixer, SFX volume rebalancing, gain timing fix
