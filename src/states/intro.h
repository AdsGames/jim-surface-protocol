#pragma once

#include <asw/asw.h>
#include <asw/util/Timer.h>

#include "../world/toolbar.h"
#include "../world/world.h"
#include "state.h"

// Intro screen of game
class Intro : public asw::scene::Scene<ProgramState> {
 public:
  using asw::scene::Scene<ProgramState>::Scene;

  void init() override;
  void update(float dt) override;
  void draw() override;

 private:
  asw::Font font;

  World world;
  Toolbar toolbar;
};
