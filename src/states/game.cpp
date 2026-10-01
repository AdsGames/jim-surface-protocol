#include "game.h"

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <vector>

#include "../lib/controls.h"
#include "../lib/save_game.h"
#include "../tiles/tile_dictionary.h"

namespace {
struct Milestone {
  float progression;
  const char* message;
  asw::Color color;
};

const std::array<Milestone, 3> MILESTONES = {{
    {0.33F, "The world is getting cleaner.", asw::color::yellow},
    {0.66F, "Almost there! Keep going.", asw::color::cyan},
    {0.99F, "You win! The world is clean.", asw::color::green},
}};
}  // namespace

void Game::init() {
  font = asw::assets::load_font("assets/fonts/syne-mono.ttf", 32);
  font_ui = asw::assets::load_font("assets/fonts/syne-mono.ttf", 22);

  // Map size comes from the save, so read it before the world is made
  std::optional<nlohmann::json> save;
  if (save_game::load_requested) {
    save_game::load_requested = false;
    save = save_game::read();
    if (save) {
      TileMap::MAP_WIDTH =
          std::clamp(save->at("map").at("width").get<int>(), 1, MAX_MAP_WIDTH);
      TileMap::MAP_DEPTH =
          std::clamp(save->at("map").at("depth").get<int>(), 1, MAX_MAP_DEPTH);
      TileMap::SEED = save->at("map").at("seed").get<float>();
    }
  }

  world.init();

  // The scene is reused, so rebuild the UI from nothing
  ui.close_modals();
  ui.clear_toasts();
  ui.root.clear_children();
  ui.ctx.theme.font = font_ui;
  ui.ctx.theme.toast.margin = 60.0F;
  ui.ctx.theme.sound_activate = asw::assets::load_sample("assets/sfx/ui.ogg");

  // White ring, so focus does not look like the yellow chosen tool frame
  ui.ctx.theme.focus_ring = {
      .color = asw::color::white, .width = 2.0F, .offset = 3.0F};
  ui.on_back = [this]() { setToolbarFocus(false); };
  menu_navigation = asw::ui::bind_default_navigation();
  play_navigation = controls::off_navigation();

  auto& title = ui.root.add_child<asw::ui::Label>();
  title.font = font;
  title.color = asw::color::black;
  title.text = "J1M Surface Protocol";
  title.anchor = asw::ui::Anchor::TopLeft;
  title.anchor_margin = {10.0F, 10.0F};

  toolbar.init(ui, world);
  setToolbarFocus(false);
  world_input_ready = false;

  won = false;
  milestones_primed = false;
  last_progression = 0.0F;

  if (save) {
    world.load(*save);
    toolbar.load(*save);
    ui.toast("Save loaded");
  } else {
    ui.toast({.text = "The world is toxic. Clean it up.",
              .seconds = 5.0F,
              .color = asw::color::red});
  }
}

void Game::update(float dt) {
  // Menus and the toolbar take UI navigation. Play leaves it off, so the
  // D-pad and sticks move the camera and cursor instead of UI focus
  const bool was_busy = ui.has_modal() || toolbar_focus;
  ui.ctx.navigation = was_busy ? menu_navigation : play_navigation;

  // The left stick moves UI focus while busy, so the cursor stays put
  if (!was_busy) {
    controls::update_pointer(dt);
  }

  // Back closes a modal, or leaves the toolbar, inside ui.update. Checked
  // against was_busy, so the same press does not reach the world
  const bool pointer_used_by_ui = ui.update();
  if (ui.has_modal()) {
    // The toolbar does not update under a menu, so stop the drill here
    world.getSounds().setDrilling(false);
    return;
  }

  // Focus only shows while the toolbar has it. A click on a toolbar button
  // focuses it, which would show a ring there after the next menu
  if (!toolbar_focus && ui.ctx.focus.focused() != nullptr) {
    ui.clear_focus();
  }

  if (was_busy || toolbar_focus) {
    world_input_ready = false;
  } else if (!controls::any_world_action_held()) {
    world_input_ready = true;
  }

  const bool world_input = world_input_ready && !toolbar_focus;
  world.update(dt, world_input);
  toolbar.update(dt, world, ui, pointer_used_by_ui, world_input);

  checkMilestones();

  // Start pauses from the toolbar too. Escape there is back, handled above
  if (toolbar_focus && asw::input::get_action_down(controls::PAUSE)) {
    setToolbarFocus(false);
    openPauseMenu();
    return;
  }

  if (was_busy) {
    return;
  }

  if (asw::input::get_action_down(controls::PAUSE)) {
    openPauseMenu();
  } else if (asw::input::get_action_down(controls::TOOLBAR)) {
    setToolbarFocus(true);
  }
}

void Game::setToolbarFocus(bool focus) {
  toolbar_focus = focus;
  if (focus) {
    ui.focus(toolbar.firstFocus(), true);
  } else {
    ui.clear_focus();
  }
}

void Game::draw() {
  world.draw();
  toolbar.drawWorld(world);
  ui.draw();

  // Focus is the cursor while the toolbar or a menu has it
  if (!toolbar_focus && !ui.has_modal()) {
    controls::draw_pointer();
  }
}

void Game::cleanup() {
  // The stage music and ambience belong to this playthrough
  asw::sound::stop_music();
  world.getSounds().stop();
}

void Game::checkMilestones() {
  const float progression = world.getProgression();

  // A loaded world starts part way, so take its progression without toasts
  if (!milestones_primed) {
    milestones_primed = true;
    last_progression = progression;
    won = progression >= MILESTONES.back().progression;
    return;
  }

  for (const auto& milestone : MILESTONES) {
    if (progression >= milestone.progression &&
        last_progression < milestone.progression) {
      ui.toast({.text = milestone.message,
                .seconds = 5.0F,
                .color = milestone.color});
      controls::rumble(0.5F, 0.5F, 400);
    }
  }

  if (!won && progression >= MILESTONES.back().progression) {
    won = true;
    openWinMenu();
  }

  last_progression = progression;
}

void Game::openPauseMenu() {
  auto& modal = ui.open_modal();
  modal.add_text("Paused", font);

  modal.add_button("Resume", [&modal]() { modal.close(); });

  modal.add_button("Save", [this, &modal]() {
    modal.close();
    if (saveGame()) {
      ui.toast("Game saved");
      world.getSounds().play("three_tone_2");
    } else {
      ui.toast({.text = "Could not save", .color = asw::color::red});
      world.getSounds().play("blocked");
    }
  });

  modal.add_button("Save and quit to menu", [this, &modal]() {
    modal.close();
    saveGame();
    manager.set_next_scene(ProgramState::Menu);
  });
}

void Game::openWinMenu() {
  auto& modal = ui.open_modal();
  modal.add_text("The world is clean!", font);
  modal.add_text("J1M thanks you for your service.");

  modal.add_button("Keep playing", [&modal]() { modal.close(); });

  modal.add_button("Back to menu", [this, &modal]() {
    modal.close();
    saveGame();
    manager.set_next_scene(ProgramState::Menu);
  });
}

bool Game::saveGame() {
  nlohmann::json data;
  world.save(data);
  toolbar.save(data);
  return save_game::write(data);
}
