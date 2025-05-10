#include "init.h"

#include <asw/asw.h>

#include "../tiles/structure_dictionary.h"
#include "../tiles/tile_dictionary.h"

void Init::init() {
  asw::display::setTitle("Setting up");

  TileDictionary::load("assets/tiles.json");
  StructureDictionary::load("assets/structures.json");
  StructureDictionary::load("assets/structures.json");

  asw::display::setIcon("assets/icon.ico");

  asw::display::setTitle("Tojam 2025");
}

void Init::update(float dt) {
  sceneManager.setNextScene(ProgramState::Intro);
}

void Init::draw() {
  asw::draw::clearColor(asw::util::makeColor(0, 0, 0));
}
