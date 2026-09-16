# Combat Assist (Linux)

[![Platform](https://img.shields.io/static/v1?label=platform&message=Linux%20(Proton)%20%7C%20Windows&color=dimgray&style=flat)](#)
[![License](https://img.shields.io/static/v1?label=license&message=GPLv3&color=blue&style=flat)](LICENSE)

A [KCSE](https://github.com/ThisIsAkill/KCSE-kcd1_linux) plugin for **Kingdom Come: Deliverance 1** that automates perfect blocks and master strikes, configurable via in-game console variables. This build runs on **Linux via Wine/Proton** (and still works on native Windows).

Requires [KCSE](https://github.com/ThisIsAkill/KCSE-kcd1_linux) — install that first.

## Installation

1. Grab `CombatAssist.dll` from the [latest release](../../releases/latest).
2. Copy it to `<game>/mods/CombatAssist/KCSE/Plugins/` (create the folders if they don't exist).
3. Launch the game. Check `<game>/KCSE/KCSE.log` for `Loaded Combat Assist` to confirm it's active.

This currently supports **Steam, game version 1.9.7.0** (same as KCSE).

## Console Variables

Set from the in-game console (`~`):

| CVar | Default | Description |
|------|---------|-------------|
| `kcse_ca_alwaysPB` | 0 | Always auto perfect block |
| `kcse_ca_alwaysMS` | 0 | Always auto master strike |
| `kcse_ca_chancePB` | 0 | 0–100: % chance to auto perfect block |
| `kcse_ca_chanceMS` | 0 | 0–100: % chance to auto master strike |
| `kcse_ca_alwaysHuntPB` | 0 | Auto PB during hunt/gap-close attacks |
| `kcse_ca_alwaysHuntMS` | 0 | Auto MS during hunt attacks |
| `kcse_ca_autoCounter` | 0 | Auto-react N times then stop (decrements each use) |
| `kcse_ca_alwaysTackle` | 0 | Player can always tackle during a hunt attack |

Master strikes are preferred over perfect blocks when both are available.

`kcse_ca_alwaysTackle` requires **v1.0.1+**: earlier Linux builds shipped this hook broken (it failed to link), so the cvar existed but never actually worked.

### Setting these automatically

These cvars aren't registered until after Combat Assist loads, so a `+exec user.cfg` **launch option** (processed at engine boot, before the plugin loads) won't set them — the console just doesn't know about them yet.

To avoid retyping them every session, put them in `<game>/user.cfg` as usual:

```
kcse_ca_alwaysPB = 1
kcse_ca_alwaysMS = 1
```

Combat Assist automatically re-runs `exec user.cfg` itself right after registering its cvars, so this file gets applied for real once per game launch — no launch options or manual console entry needed.

## Building from source

```sh
git clone --recursive https://github.com/ThisIsAkill/CombatAssist-KCSE_linux.git
cd CombatAssist-KCSE_linux
./build.sh
```

This initializes the `libKCD1` submodule, cross-compiles with MinGW-w64 (`sudo apt install mingw-w64`), and produces `build-mingw/CombatAssist.dll`.

`libKCD1`'s reverse-engineered struct layouts assume MSVC's `std::map` (16 bytes); GCC/MinGW's `std::map` is 48 bytes. `build.sh` automatically applies `patches/libKCD1-mingw-struct-layout.patch` to fix the resulting size mismatches before building — you don't need to do anything manually.

Building against MSVC/vcpkg on Windows works the same way as upstream; see `.buildenv/CMakeLists.txt`.

## Acknowledgments

This is a Linux port of [JerryYOJ](https://github.com/JerryYOJ)'s **[CombatAssist](https://github.com/JerryYOJ/CombatAssist-KCSE)**, built on their **[KCSE](https://github.com/JerryYOJ/KCSE-for-kcd1)** framework and **[libKCD1](https://github.com/JerryYOJ/libKCD1)** reverse-engineering work. All credit for the plugin's design and the reverse-engineered game internals it depends on goes to them — this fork's contribution is getting it building and running on Linux via Wine/Proton.

Parts of this port (build fixes, debugging, and documentation) were done with AI assistance.

## License

[GPLv3](LICENSE)
