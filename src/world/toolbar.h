#pragma once

#include <asw/asw.h>

#include "world.h"

class Toolbar {
 public:
  Toolbar() = default;

  void init();
  void update(float dt, World& world);
  void draw(World& world);

 private:
  asw::Font font;
  Tile* selected_tile{nullptr};
};