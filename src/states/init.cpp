#include "init.h"

#include <asw/asw.h>

#include "../tiles/structure_dictionary.h"
#include "../tiles/tile_dictionary.h"

void Init::init() {
  asw::display::set_title("Setting up");

  TileDictionary::load("assets/tiles.json");
  StructureDictionary::load("assets/structures.json");

  asw::display::set_icon("assets/icon.ico");

  asw::display::set_title("J1M Surface Protocol");
}

void Init::update(float dt) {
  manager.set_next_scene(ProgramState::Intro);
}

void Init::draw() {
  asw::draw::clear_color(asw::color::black);
}
