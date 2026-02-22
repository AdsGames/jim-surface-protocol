#include "intro.h"

#include <asw/asw.h>

#include "../tiles/structure_dictionary.h"
#include "../tiles/tile_dictionary.h"

void Intro::init() {
  intro_1 = asw::assets::load_texture("assets/images/ui/intro-1.png");
  font = asw::assets::load_font("assets/fonts/syne-mono.ttf", 128);
  font_small = asw::assets::load_font("assets/fonts/syne-mono.ttf", 96);

  music = asw::assets::load_music("assets/music/intro.ogg");
  asw::sound::play_music(music, 64.0F);
}

void Intro::update(float dt) {
  timer += dt;
  if (timer > 2.0f || asw::input::keyboard.any_pressed) {
    manager.set_next_scene(ProgramState::Menu);
  }
}

void Intro::draw() {
  asw::draw::sprite(intro_1, asw::Vec2(0.0F, 0.0F));
  asw::draw::text(font, "J1M:", asw::Vec2(640.0F, 320.0F), asw::color::white,
                  asw::TextJustify::Center);
  asw::draw::text(font_small, "Surface Protocol", asw::Vec2(640.0F, 440.0F),
                  asw::color::white, asw::TextJustify::Center);
}
