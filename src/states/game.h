#pragma once

#include <asw/asw.h>

#include "../world/toolbar.h"
#include "../world/world.h"
#include "state.h"

// Game screen of game
class Game : public asw::scene::Scene<ProgramState> {
 public:
  using asw::scene::Scene<ProgramState>::Scene;

  void init() override;
  void update(float dt) override;
  void draw() override;

 private:
  asw::Font font;
  asw::Font font_small;

  World world;
  Toolbar toolbar;
};
