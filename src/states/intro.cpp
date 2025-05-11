#include "intro.h"

#include <asw/asw.h>

#include "../tiles/structure_dictionary.h"
#include "../tiles/tile_dictionary.h"

void Intro::init() {
  intro_1 = asw::assets::loadTexture("assets/images/ui/intro-1.png");
  font = asw::assets::loadFont("assets/fonts/syne-mono.ttf", 128);
  font_small = asw::assets::loadFont("assets/fonts/syne-mono.ttf", 96);

  music = asw::assets::loadMusic("assets/music/intro.ogg");
  asw::sound::playMusic(music, 64.0F);
}

void Intro::update(float dt) {
  timer += dt;
  if (timer > 2000.0f || asw::input::keyboard.anyPressed) {
    sceneManager.setNextScene(ProgramState::Menu);
  }
}

void Intro::draw() {
  asw::draw::sprite(intro_1, asw::Vec2(0.0F, 0.0F));
  asw::draw::textCenter(font, "J1M:", asw::Vec2(640.0F, 320.0F),
                        asw::util::makeColor(255, 255, 255));
  asw::draw::textCenter(font_small, "Surface Protocol",
                        asw::Vec2(640.0F, 440.0F),
                        asw::util::makeColor(255, 255, 255));
}
