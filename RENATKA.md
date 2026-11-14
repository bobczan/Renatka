# Renatka 2.2.1

**Renatka** is a ClioPlay build for the Renault Carminat TomTom, made for one
particular car: a 2010 Renault Scénic III whose head unit is driven from the
armrest console instead of a Bluetooth remote. It keeps everything ClioPlay 2.0
does — offline OpenStreetMap navigation, routing and rerouting, Bluetooth audio,
phone destination transfer, 18 interface languages — and adds the work needed to
make that car usable: the physical buttons, a deeper zoom, and a speed display
laid out the way the driver wants it.

The important consequence for anyone used to upstream: **the armrest console
works out of the box.** No helper daemon, no `/dev/uinput`, no extra binary. The
reader lives in `clio040` and is enabled by the empty `csw` file on the card.

> **Version note.** `main` carries 2.1.1; this tree is **2.2.1**, built on the
> latest upstream. The older 1.x Renatka tree is preserved on the `legacy-1.x`
> branch and tag; it has no Scenic console support and no HUD rework.

### Based on

| | |
| --- | --- |
| Upstream | [`pxwel-dev/ClioPlay`](https://github.com/pxwel-dev/ClioPlay), commit `7a962e2` (2.0.0 plus the rendering and popup work) |
| Vehicle | Renault Scénic III (2010), Carminat TomTom non-LIVE, `hw_remote=0`, `hw_handsfree=0`, `hw_bluetooth=1`, Linux 2.6.13 / S3C2443 |
| Firmware | 8.842 on the unit; the installer supplies 8.843 `ttsystem` + `vehicledaemon` from TomTom's own CAB |

### What it does

- Offline OSM navigation with CPM5 map tiles, CPR2 routing, turn-by-turn
  guidance, roundabouts, trips, stops, recents and favourites.
- **Armrest console**: nine buttons, joystick, rotary knob (zoom/list movement)
  and knob press, read straight from `vehicledaemon`'s `/dev/input/CswEvent0`.
- **Speed in the bottom-right** with a round UK speed-limit sign to its left; no
  `--` placeholder when the limit is unknown.
- **Manual zoom to Z10**, two steps deeper than upstream, persisted across
  reboots.
- Bluetooth audio and phone destination transfer through the car's own factory
  Bluetooth module.
- Everything is `debug=0` by default: no logs, no SD-card writes while driving.

### Summary of changes from upstream

| Area | Change |
| --- | --- |
| Branding | splash title `Renatka` (original play-triangle kept), `from Renatka Send` in 18 locales, `RENATKA` debug banner |
| Boot | `ttn` finds the card itself — this firmware mounts it at `/media/sdcard`, not upstream's `/mnt/sdcard`, which was why the first card booted stock TomTom |
| Console | new CSW reader in `clio040.c`: creates the FIFO, decodes the real 28-byte record shapes, built-in Scenic key map, optional `cswmap.ini` override |
| HUD | speed moved bottom-right, manoeuvre card bottom-left, round limit sign, measured pill width, 49 px bar |
| Zoom | `DRIVE_MAX_ZOOM` and `MAX_EXPLORE_ZOOM` raised to 10 |
| Logging | `csw.log` and all other diagnostics are Debug-only |

### Where to find things

| File | Contents |
| --- | --- |
| [`RENATKA.md`](RENATKA.md) | this file: what it does, what is branded, console protocol, build and install |
| [`CHANGELOG-RENATKA.md`](CHANGELOG-RENATKA.md) | full change history, hardware facts, verification, and a step-by-step recipe for porting to a new ClioPlay release |
| [`cswmap.example.ini`](cswmap.example.ini) | documented console map; copy to the card as `cswmap.ini` only to override a control |
| `splash-preview.png` | rendered 480×272 proof of the splash |

Everything in ClioPlay 2.0 is included: the frosted-glass HUD pills, shared
status notifications, Bluetooth CTRL with playback progress, coordinate
destination entry, 18 UI languages, the 19-country camera policy and the
adaptive 15/10/5 FPS HUD refresh.

---

## 1. What is branded

| Item | Change |
| --- | --- |
| Boot splash mark | **Unchanged** — the original ClioPlay play-triangle is kept |
| Boot splash title | `ClioPlay` → **`Renatka`** (`src/clio843/src/cliospl.c`) |
| Phone-send wording | `from ClioPlay Send` → **`from Renatka Send`** in all 18 languages (edited in `src/locales/*.json`; `src/clio843/src/ui_translations.h` is generated from them) |
| Debug banners | `CLIOPLAY GLOBAL 1.1.0-rc.1 …` → `RENATKA 2.1.1 …` |

The splash is still drawn once, from precomputed 8×8 coverage masks, so the
branding costs nothing at boot. The mark generator is unchanged from upstream;
only the title string differs. Regenerate it with:

```sh
cd src
python3 tools/build_splash_icon.py
```

> The phone-send *protocol* string in `clioa2dp.c` is deliberately **unchanged**
> (`ClioPlay Send`). It is what the sender app puts in AVRCP metadata; renaming
> it would break destination transfer. Only the on-screen label is Renatka.

The rendered result (`splash-preview.png`, 480×272) is committed next to this
file so the branding can be checked without booting the car.

## 2. Scenic III compatibility carried over

These are the `clio040` changes proposed in issue #1 §5 and §6:

| Change | Where | Why it matters on a Scenic |
| --- | --- | --- |
| Rotary knob keys `191`/`192` (`KEY_F21`/`KEY_F22`) mapped to the current screen | `clio040.c` `K_ROTN`/`K_ROTP` | The armrest knob can zoom the map, walk menus and move through dialogs without a Bluetooth remote |
| Outside temperature read from `/tmp/cltemp` before the radio telemetry file | `clio040.c` `poll_cltemp()` | The Scenic does not deliver temperature over the Focus path |
| Implausible temperatures rejected (`-60…85 °C`) | `clio040.c` `poll_cltemp()` | Stops the stock `999 °C` failure value from being shown |
| Empty source / volume / temperature fields hidden instead of `--` | upstream `hud.h` | Radio source and volume are on the dashboard TFT, not in the ClioPlay data path |
| Manual driving zoom may use Z7…Z10 | `clio040.c` `DRIVE_MAX_ZOOM` | Deeper map zoom; Z7…Z10 reuse the Z6 tiles |
| Manual zoom level persisted | upstream `prefs.ini` `manualzoom` | Survives a reboot |
| Browsed map point → navigate / **add to favourites** / cancel | `clio040.c` `MOD_ROUTE_POINT` | Saves a point without leaving the map first |

The 28 px top bar, reordered bottom bar, round speed-limit sign and larger clock
from §6 are **not** ported: ClioPlay 2.0 replaced that whole header/footer with
the HUD pill system, so those patches no longer apply.

### Renatka HUD changes on top of 2.0

| Change | Where |
| --- | --- |
| Speed card moved to the **bottom-right**; the manoeuvre card takes the bottom-left | `hud.h`, `hud_driver_prepare()` |
| Speed-limit display is a **round UK sign** (red ring, white face, black digits) to the left of the speed, instead of a small glass box | `hud.h`, `hud_speed_sign()` |
| Unknown, infinite or variable limits draw **nothing** rather than `--` | `hud.h`, `hud_sign_text()` |
| Estimated limits use a grey ring instead of red, as in the earlier Renatka build | `hud.h`, `hud_speed_sign()` |

## 3. Armrest console and outside temperature

### Controls: read directly, no daemon

`clio040` only consumes plain Linux key events from `/dev/input/event0..7`, and
on this Scenic the armrest controls are not exposed that way. They arrive over
the `UiInput::RCswPipe` FIFO at `/dev/input/CswEvent0` as 28-byte records.

Two details matter, both taken from the working `cswd` capture on this car:

1. **Something has to create the FIFO.** `vehicledaemon` publishes into
   `/dev/input/CswEvent0`, but on this firmware the node is created by the
   consumer (`cswd` did `unlink` + `mknod(S_IFIFO|0666)`). Renatka does the
   same in `csw_init()`, before `vehicledaemon` is restarted, and treats
   `EEXIST` as "already there".
2. **The record shapes differ per control**, and only some fields are real:

| Control | `src` | `dir` | `rot` | Notes |
| --- | --- | --- | --- | --- |
| Console button (9 of them) | 1…9 | varies | garbage | only `src` is meaningful; the rest is uninitialised memory |
| Joystick | 0 | 1 up, 2 down, 3 right, 4 left | 3 | |
| Knob rotation | 0 | 5 | 1 anticlockwise, 2 clockwise | |
| Knob press | 0 | 5 | 3 | repeats while held; one key per burst |

The FIFO is opened `O_NONBLOCK` so boot never waits for `vehicledaemon`, and it
is deliberately never closed: dropping the read end would deliver `SIGPIPE` to
the writer and could take `vehicledaemon` down with it.

| Piece | Where |
| --- | --- |
| FIFO node creation, framing, debounce | `clio040.c`, `csw_init()` / `poll_csw()` |
| Enable switch | empty file `csw` in the SD-card root |
| Control map | `cswmap.ini` in the SD-card root (`cswmap.example.ini` in the repo) |
| Diagnostics | `clio843/csw.log` |

The built-in mapping matches this console, so no `cswmap.ini` is required:
`btn1` zoom out, `btn2` back, `btn3` zoom in, `btn4` ETA, `btn5`/`btn8` menu,
`btn9` map, joystick arrows, knob rotate for zoom/list movement, knob press for
OK. `cswmap.ini` only overrides.

### First run: capture, then map

1. Create an empty file `csw` in the SD-card root. No `cswmap.ini` is needed
   unless you want to change a control.
2. Boot, then press each armrest control once, slowly, one at a time.
3. Read `clio843/csw.log`. Every record is logged as:

   ```
   rec src=12 dir=3 rot=0 pr=1 st=0 0c 00 00 00 03 00 ...
   ```

4. Copy `cswmap.example.ini` to the SD-card root as `cswmap.ini` and add one
   line per control that needs changing:

   ```
   btn9=map
   dir1=up
   rot2=knob+
   push=enter
   ```

   The map is re-read within 5 seconds — no reboot needed.
5. Anything still unmapped is logged as `UNMAPPED src=… dir=…`.

Deleting the `csw` file and rebooting restores stock behaviour completely: with
no marker file the FIFO is not created or opened.

### Outside temperature

`poll_cltemp()` already reads a plain integer from `/tmp/cltemp` and rejects
implausible values (`-60…85 °C`), which is what stops the stock `999 °C`. What is
still missing is a producer: the value lives on the PAC reflection socket (port
3700, `VehicleStatus` interface 37, item 4) and nothing writes `/tmp/cltemp` yet.

At boot `csw.log` records which candidate paths exist (`/tmp/cltemp`,
`/tmp/cfstate`, `/tmp/cfbus`, `/tmp/cftemp`, …), so the first boot shows whether
an existing daemon already publishes the value somewhere. If none does, a PAC
client is the next piece of work.

### What works today

- Boot, splash and branding
- Map rendering, GPS, pan/zoom, Explore mode
- Turn-by-turn routing, trips, stops, ETA
- Recents and favourites
- Coordinate destination entry
- Bluetooth audio through the Scenic's **separate factory Bluetooth/audio
  module** (independent of the navigation unit)
- Phone connection and destination transfer from the phone
- Armrest controls (built-in Scenic mapping, overridable with `cswmap.ini`)

### What still does not work

- Outside temperature, until something writes `/tmp/cltemp`

Radio source, station and volume are not available in ClioPlay on this vehicle
at all — they are shown on the dashboard TFT. ClioPlay 2.0 hides those fields
rather than printing `--`.

## 4. Build

Requires Python 3.12+, Clang with the ARM backend, LLD and `llvm-ar`
(WSL 2 / Ubuntu 24.04 is what this was built with).

```sh
cd src
python3 tools/build_ui.py --install     # builds and installs clio040, cliospl, clioa2dp
```

`build_ui.py` also regenerates `splash_icon.h` from `tools/build_splash_icon.py`,
so the splash masks are always in sync with the source.

### About the trailing error from `--install`

`--install` copies the three programs into `clio843/` first, then tries to
refresh `latest.json` / `SHA256SUMS`. That refresh validates the README preview
GIFs and their source fingerprints, so after any change to a previewed source
file (for example `cliospl.c`) it stops with:

```
ValueError: Preview input changed; run tools/render_previews.py: clio843/src/cliospl.c
```

The binaries are already installed at that point, so the error is cosmetic.
Regenerate the previews and manifests only if you want them current:

```sh
python3 tools/render_previews.py
python3 tools/refresh_latest.py
```

The card installer (`src/install.py`) does **not** read `latest.json` or
`SHA256SUMS`, so leaving them stale does not affect installation.

### Build the navigation bundle on ext4

`navigation-builder/build_navigation.py` must run with `--work` and `--output`
on a Linux filesystem. On a `/mnt/c` DrvFs mount, `os.replace()` of a SQLite
file that is still open (as `build_index()` in `build_ocean.py` leaves it)
silently leaves the target missing, and the build then fails with:

```
sqlite3.OperationalError: unable to open database file
```

Point `--cache` at a normal directory; the ~900 MB coastline archive is only
read after the first extraction.

```sh
python3 navigation-builder/build_navigation.py region.osm.pbf --yes \
  --output /home/you/clioplay-nav/output \
  --work   /home/you/clioplay-nav/work \
  --cache  navigation-builder/.cache
```

## 5. Install on the card

Same as upstream: run `Install.cmd` on Windows, or `Install.command` /
`python3 install.py` on macOS/Linux, and point it at an `.osm.pbf` extract.
See `src/BUILDING.md` and `WINDOWS.md`.

The resulting card layout is:

```
ttn  ttsystem  vehicledaemon        boot + vendor runtime
clio843/                            clio040, cliospl, clioa2dp, font.bin, btstart.sh,
                                    focusd, hidd, prefs.ini, card-runtime.json
clio843/maps/                       map.cpm, route.cpr, route.qadj, fonts.cpf,
                                    regions.cpg, timezones.cpt, cameras.cpc, build.json
csw                                 empty marker file: enable the armrest reader
cswmap.example.ini                  template, inactive until renamed to cswmap.ini
```

A Northamptonshire test bundle occupies about 90 MB, so a 4 GB card is enough
for a regional test; a full country needs the 8 GB card from the upstream
README.

## 6. Verification

```sh
cd src
python3 tools/verify.py
python3 tools/verify_install.py
```
