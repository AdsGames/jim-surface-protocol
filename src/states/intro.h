#pragma once

#include <asw/asw.h>
#include <asw/util/Timer.h>

#include "../tiles/tile_map.h"
#include "state.h"

// Intro screen of game
class Intro : public asw::scene::Scene<ProgramState> {
 public:
  using asw::scene::Scene<ProgramState>::Scene;

  void init() override;
  void update(float dt) override;
  void draw() override;

 private:
  Timer timer;

  asw::Font font;

  float frame{0};

  TileMap tile_map;

  asw::Quad<float> camera{0, 0, 1280, 960};
};
