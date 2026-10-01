#pragma once

#include <asw/asw.h>
#include <array>
#include <functional>
#include <string>

#include "state.h"

class Menu : public asw::scene::Scene<ProgramState> {
 public:
  using asw::scene::Scene<ProgramState>::Scene;

  void init() override;
  void update(float dt) override;
  void draw() override;
  void cleanup() override;

 private:
  // Start a new map, with sizes and seed from the config file if it has them
  void startNewGame(int size, float seed);

  void toggleFullscreen();

  asw::ui::Button& addButton(asw::ui::Stack& list,
                             const std::string& text,
                             std::function<void()> on_click);

  asw::Texture background;
  asw::Texture jim;
  std::array<asw::Texture, 6> jims;

  asw::Font font;
  asw::Font font_small;
  asw::Font font_button;

  asw::ui::Root ui;

  asw::Music music;

  float timer{0.0F};
  int frame{0};
  bool fullscreen{false};
};
