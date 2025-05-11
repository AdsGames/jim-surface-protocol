#pragma once

#include <asw/asw.h>
#include <array>

#include "state.h"

class Menu : public asw::scene::Scene<ProgramState> {
 public:
  using asw::scene::Scene<ProgramState>::Scene;

  void init() override;
  void update(float dt) override;
  void draw() override;
  void cleanup() override;

 private:
  asw::Texture background;
  asw::Texture jim;
  std::array<asw::Texture, 6> jims;

  asw::Font font;
  asw::Font font_small;
  asw::Font font_button;

  asw::Quad<float> start_easy_transform{40.0F, 740.0F, 300.0F, 50.0F};
  asw::Quad<float> start_hard_transform{40.0F, 780.0F, 300.0F, 50.0F};
  asw::Quad<float> exit_transform{40.0F, 820.0F, 300.0F, 50.0F};

  float timer{0.0F};
  int frame{0};
};
