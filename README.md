# j1m-surface-protocol

The year is 2099, after decades of mistreatment, the planet has decayed to an uninhabitable sludge. It is your mission to reclaim the planet.

## Demo

[Web Demo](https://adsgames.github.io/jim-surface-protocol/)

## Gameplay

Your goal is to purify the map using trees and water purifiers. As you progress, so will the environment around you.

## Walkthrough

Start by drilling a few crates or dead trees to gather biomass.

Once you have collected 10 units of biomass, you can build a sapling. After some time the sapling will grow into a tree which purifies nearby soil and grass.

Junk drops scrap which can be used to upgrade move/drill speed.

Once you have accumulated enough biomass, you can place purifiers on the toxic water.

Enjoy and try to purify the whole world!

## Controls

| Action                                          | Mouse and keyboard                           | Controller                                       |
| ----------------------------------------------- | -------------------------------------------- | ------------------------------------------------ |
| Choose tool                                     | 1 Drill, 2 Purifier, 3 Tree, or click a tool | LB / RB                                          |
| Use chosen tool (drill or build)                | Hold left click                              | Hold LT                                          |
| Drill, whatever tool is chosen                  |                                              | Hold RT                                          |
| Set waypoint                                    | Right click                                  | A                                                |
| Centre camera on J1M                            | Space                                        | X                                                |
| Move cursor                                     | Mouse                                        | Left stick                                       |
| Scroll map                                      | WASD, arrow keys, or mouse at screen edge    | Right stick, D-pad, or cursor at screen edge     |
| Use toolbar buttons (tools, upgrades, J1M tips) | Click                                        | A on a button, or Y then D-pad and A. B to leave |
| Pause menu (resume, save, save and quit)        | Escape                                       | Start or Back                                    |
| Menus                                           | Mouse, or arrow keys and Enter               | D-pad or left stick, and A                       |
| Debug info                                      | Q                                            |                                                  |
| Full screen (menu only)                         | F11                                          |                                                  |

The game saves from the pause menu. Choose **Continue** on the main menu to load the save. Web builds keep the save in browser storage.

## Config

Set `ASW_CONFIG` to the path of a config file to change settings, for example for an arcade launcher:

```ini
# arcade.cfg
display.fullscreen = true
game.map_size = 60     # Map width and depth for new games, 10 to 100
game.seed = 1234       # Seed for new maps
game.day_length = 240  # Seconds in one day and night cycle
```

```bash
ASW_CONFIG=arcade.cfg ./J1MSurfaceProtocol
```

## Setup

### CMake

```bash
cmake --preset debug
cmake --build --preset debug
```

### Build Emscripten

```bash
emcmake cmake --preset debug
cmake --build --preset debug
```
