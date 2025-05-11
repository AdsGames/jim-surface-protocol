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

Right Click: Set waypoint
Left Click: Perform action (drill/ build)
WASD/Arrow Keys/Mouse: Scroll Map
1: Select Drill
2: Purifier Build Tool
3: Tree Build Tool
Q: Debug Mode
F11: Full Screen (Menu Only)

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
