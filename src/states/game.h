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
  void cleanup() override;

 private:
  // Toast when the world passes a purity milestone
  void checkMilestones();

  void openPauseMenu();
  void openWinMenu();

  // A controller moves focus into the toolbar, or back out to the world
  void setToolbarFocus(bool focus);

  bool saveGame();

  asw::Font font;
  asw::Font font_ui;

  World world;
  Toolbar toolbar;

  asw::ui::Root ui;

  // UI navigation for menus and the toolbar, and none for play
  asw::ui::Navigation menu_navigation;
  asw::ui::Navigation play_navigation;

  float last_progression{0.0F};
  bool won{false};
  bool milestones_primed{false};
  bool toolbar_focus{false};

  // False after a menu, the toolbar or a new scene had input, until every
  // world button is let go. Stops the click or A that closed a menu from
  // also drilling or moving the worker
  bool world_input_ready{false};
};
