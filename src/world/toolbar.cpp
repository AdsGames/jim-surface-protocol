#include "toolbar.h"

#include <cmath>

void Toolbar::init() {
  font = asw::assets::loadFont("assets/fonts/syne-mono.ttf", 18);
  fontLarge = asw::assets::loadFont("assets/fonts/syne-mono.ttf", 30);

  inspect_button =
      asw::assets::loadTexture("assets/images/ui/inspect-button.png");
  purifier_button =
      asw::assets::loadTexture("assets/images/ui/purify-button.png");
  tree_button = asw::assets::loadTexture("assets/images/ui/tree-button.png");
  drill_button = asw::assets::loadTexture("assets/images/ui/drill-button.png");
  toolbar_ui = asw::assets::loadTexture("assets/images/ui/toolbar.png");

  upgrade = asw::assets::loadTexture("assets/images/ui/upgrade.png");

  overlay_1 = asw::assets::loadTexture("assets/images/ui/overlay_1.png");
};

void Toolbar::update(float dt, World& world) {
  // Enabled actions
  can_take_action = actionEnabled(world);

  // CHEATING ZONE (DOONT LOOK)

  if (asw::input::wasKeyPressed(asw::input::Key::F)) {
    world.getResourceManager().addResourceCount("scrap", 100);
    world.getResourceManager().addResourceCount("biomass", 100);
  }

  auto& camera = world.getCamera();
  auto& tile_map = world.getTileMap();
  auto mouse_pos = asw::Vec2(asw::input::mouse.x, asw::input::mouse.y);
  cursor_idx = tile_map.getIndexAt(camera.position + mouse_pos);
  tile_map.setSelectedIndex(cursor_idx);

  // Find distance to worker
  auto* selected_tile = tile_map.getTileAtIndex(cursor_idx);
  auto* worker = world.getPlayer();
  if (worker != nullptr && selected_tile != nullptr) {
    const auto tile_pos = selected_tile->getPosition();
    const auto distance = asw::Vec3<float>(tile_pos.x, tile_pos.y, tile_pos.z)
                              .distance(worker->getPosition());
    cursor_in_range = distance < 10.0F;
  }

  // Click buttons
  if (asw::input::isButtonDown(asw::input::MouseButton::LEFT) &&
      can_take_action && cursor_in_range) {
    action(world, dt);
  } else {
    actionProgress = 0.0F;
  }

  if (asw::input::wasButtonPressed(asw::input::MouseButton::LEFT)) {
    toolZoneAction(world);
  }

  if (asw::input::isButtonDown(asw::input::MouseButton::RIGHT)) {
    rightClickAction(world);
  }
}

void Toolbar::rightClickAction(World& world) {
  auto& tile_map = world.getTileMap();

  auto* tile = tile_map.getTileAtIndex(cursor_idx);
  if (tile != nullptr) {
    auto type = tile->getTypeId();
    if (type != 18) {
      world.setPlayerWaypoint(cursor_idx);
      world.setWaypointActive(true);
    }
  }
}

void Toolbar::toolZoneAction(World& world) {
  auto& camera = world.getCamera();
  auto& resource_manager = world.getResourceManager();
  auto mouse_pos = asw::Vec2(asw::input::mouse.x, asw::input::mouse.y);

  // Tool zone (I didn't know danny had a zone) HA GOTTEEEEEEEEEEEEEEM
  if (mouse_pos.y > camera.size.y - 160.0F) {
    if (purifier_button_trans.contains(mouse_pos)) {
      mode = ToolMode::PURIFIER;
    } else if (tree_button_trans.contains(mouse_pos)) {
      mode = ToolMode::TREE;
    } else if (drill_button_trans.contains(mouse_pos)) {
      mode = ToolMode::DRILL;
    } else if (upgrade_drill_trans.contains(mouse_pos)) {
      if (resource_manager.getResourceCount("scrap") >= drill_upgrade_cost) {
        world.setDrillSpeed(world.getDrillSpeed() + 1);
        resource_manager.addResourceCount("scrap", -drill_upgrade_cost);
        drill_upgrade_cost = drill_upgrade_cost * 1.5F;
      }
    } else if (upgrade_move_trans.contains(mouse_pos)) {
      if (resource_manager.getResourceCount("scrap") >= move_upgrade_cost) {
        world.setPlayerSpeed(world.getPlayerSpeed() + 1);
        resource_manager.addResourceCount("scrap", -move_upgrade_cost);
        move_upgrade_cost = move_upgrade_cost * 1.5F;
      }
    }
  }
}

void Toolbar::action(World& world, float dt) {
  auto& camera = world.getCamera();
  auto& tile_map = world.getTileMap();
  auto& resource_manager = world.getResourceManager();
  auto mouse_pos = asw::Vec2(asw::input::mouse.x, asw::input::mouse.y);

  // Toolbar zone
  if (mouse_pos.y > camera.size.y - TOOLBAR_HEIGHT) {
    return;
  }

  // Find selected tile
  auto* selected_tile = tile_map.getTileAtIndex(cursor_idx);
  std::shared_ptr<TileType> select_type = nullptr;
  if (selected_tile != nullptr) {
    select_type = selected_tile->getType();
  }

  if (mode == ToolMode::DRILL && selected_tile != nullptr &&
      select_type != nullptr) {
    auto density = select_type->getDensity();

    if (density > 0) {
      // this is where the drilling begins

      actionProgress += dt * (0.03F / density) * (world.getDrillSpeed() * 3);
      if (actionProgress > 100.0F) {
        auto actions = select_type->getActionsOfType(ActionType::DESTROY);
        for (const auto& action : actions) {
          if (!action.drop_resource_id.empty()) {
            resource_manager.addResourceCount(action.drop_resource_id, 1);
          }
        }
        selected_tile->setType(0);
        actionProgress = 0.0F;
      }
    }
  }

  if (mode == ToolMode::PURIFIER && selected_tile != nullptr &&
      select_type != nullptr) {
    auto idx = cursor_idx;
    idx.z = idx.z + 1;

    auto* tile = tile_map.getTileAtIndex(idx);
    if (tile != nullptr && tile->getType() == nullptr) {
      tile->setType("purifier");
      resource_manager.addResourceCount("scrap", -10);
    }
  }

  if (mode == ToolMode::TREE) {
    auto idx = cursor_idx;
    idx.z = idx.z + 1;

    auto* tile = tile_map.getTileAtIndex(idx);
    if (tile != nullptr && tile->getType() == nullptr) {
      tile->setType("sapling");
      resource_manager.addResourceCount("biomass", -10);
    }
  }
}

bool Toolbar::actionEnabled(World& world) {
  auto& camera = world.getCamera();
  auto& tile_map = world.getTileMap();
  auto& resource_manager = world.getResourceManager();
  auto mouse_pos = asw::Vec2(asw::input::mouse.x, asw::input::mouse.y);
  cursor_idx = tile_map.getIndexAt(camera.position + mouse_pos);

  // Can place purifier
  const auto can_buy_purifier =
      resource_manager.getResourceCount("scrap") >= 10;
  const auto can_buy_tree = resource_manager.getResourceCount("biomass") >= 10;

  if (!can_buy_purifier) {
    SDL_SetTextureColorMod(purifier_button.get(), 255, 150, 150);
  } else {
    SDL_SetTextureColorMod(purifier_button.get(), 255, 255, 255);
  }

  if (!can_buy_tree) {
    SDL_SetTextureColorMod(tree_button.get(), 255, 150, 150);
  } else {
    SDL_SetTextureColorMod(tree_button.get(), 255, 255, 255);
  }

  // Find selected tile
  auto* selected_tile = tile_map.getTileAtIndex(cursor_idx);
  std::shared_ptr<TileType> select_type = nullptr;
  if (selected_tile != nullptr) {
    select_type = selected_tile->getType();
  }

  // Density check
  if (mode == ToolMode::DRILL && selected_tile != nullptr &&
      select_type != nullptr) {
    auto density = select_type->getDensity();
    if (density > 0) {
      return true;
    }
  }

  // Purifier check
  if (mode == ToolMode::PURIFIER && selected_tile != nullptr &&
      can_buy_purifier && select_type != nullptr) {
    if (select_type->getIdString() == "toxic_water" ||
        select_type->getIdString() == "water") {
      return true;
    }
  }

  // Tree check
  if (mode == ToolMode::TREE && selected_tile != nullptr && can_buy_tree &&
      select_type != nullptr) {
    if (select_type->getIdString() == "toxic_grass" ||
        select_type->getIdString() == "ground_grass") {
      return true;
    }
  }

  return false;
}

void Toolbar::draw(World& world) {
  auto& camera = world.getCamera();
  auto& tile_map = world.getTileMap();
  auto& resource_manager = world.getResourceManager();
  auto mouse_pos = asw::Vec2(asw::input::mouse.x, asw::input::mouse.y);
  auto world_pos = camera.position + mouse_pos;
  auto* selected_tile = tile_map.getTileAt(world_pos);
  auto white = asw::util::makeColor(255, 255, 255);
  auto green = asw::util::makeColor(128, 255, 128);
  auto black = asw::util::makeColor(0, 0, 0);

  if (cursor_in_range && can_take_action) {
    drawWireframe(cursor_idx, camera.position, asw::util::makeColor(0, 255, 0));
  } else if (cursor_in_range) {
    drawWireframe(cursor_idx, camera.position, white);
  } else {
    drawWireframe(cursor_idx, camera.position, asw::util::makeColor(255, 0, 0));
  }

  // Overlay
  asw::draw::sprite(toolbar_ui, asw::Vec2<float>(0, 0));

  // Resource window
  asw::draw::text(fontLarge, "Resources", asw::Vec2(1112.0F, 822.0F), black);
  asw::draw::text(fontLarge, "Resources", asw::Vec2(1110.0F, 820.0F), white);

  asw::draw::text(font, "Scrap", asw::Vec2(1110.0F, 868.0F), white);
  asw::draw::textRight(
      font, std::to_string(resource_manager.getResourceCount("scrap")),
      asw::Vec2(1260.0F, 868.0F), green);

  asw::draw::text(font, "Biomass", asw::Vec2(1110.0F, 915.0F), white);
  asw::draw::textRight(
      font, std::to_string(resource_manager.getResourceCount("biomass")),
      asw::Vec2(1260.0F, 915.0F), green);

  // Upgrade window
  auto drill_speed = std::to_string(world.getDrillSpeed());
  auto player_speed = std::to_string(world.getPlayerSpeed());

  auto canBuyDrill = world.getResourceManager().getResourceCount("scrap") >=
                     drill_upgrade_cost;
  auto canBuyMove =
      world.getResourceManager().getResourceCount("scrap") >= move_upgrade_cost;
  auto canBuyColour = asw::util::makeColor(150, 255, 150, 255);
  auto cantBuyColour = asw::util::makeColor(255, 150, 150, 255);

  asw::draw::text(fontLarge, "Upgrades", asw::Vec2(882.0F, 822.0F), black);
  asw::draw::text(fontLarge, "Upgrades", asw::Vec2(880.0F, 820.0F), white);

  asw::draw::text(font, "Drill Speed: " + drill_speed,
                  asw::Vec2(882.0F, 858.0F),
                  asw::util::makeColor(255, 255, 255, 255));

  asw::draw::text(font, "Cost " + std::to_string(drill_upgrade_cost) + " scrap",
                  asw::Vec2(882.0F, 878.0F),
                  canBuyDrill ? canBuyColour : cantBuyColour);

  asw::draw::text(font, "Move Speed: " + player_speed,
                  asw::Vec2(882.0F, 905.0F),
                  asw::util::makeColor(255, 255, 255, 255));

  asw::draw::text(font, "Cost " + std::to_string(move_upgrade_cost) + " scrap",
                  asw::Vec2(882.0F, 925.0F),
                  canBuyMove ? canBuyColour : cantBuyColour);

  asw::draw::sprite(upgrade, asw::Vec2<float>(upgrade_drill_trans.position.x,
                                              upgrade_drill_trans.position.y));
  asw::draw::sprite(upgrade, asw::Vec2<float>(upgrade_move_trans.position.x,
                                              upgrade_move_trans.position.y));

  // Debuggo
  if (asw::input::isKeyDown(asw::input::Key::Q)) {
    // X + Y
    asw::draw::text(font,
                    "Pos: " + std::to_string(cursor_idx.x) + ", " +
                        std::to_string(cursor_idx.y) + ", " +
                        std::to_string(cursor_idx.z),
                    asw::Vec2(183.0F, 845.0F), green);

    // Camera pos
    asw::draw::text(font,
                    "Cam: " + std::to_string(camera.position.x) + ", " +
                        std::to_string(camera.position.y),
                    asw::Vec2(183.0F, 865.0F), green);

    // Mouse pos
    asw::draw::text(font,
                    "Mouse: " + std::to_string(mouse_pos.x) + ", " +
                        std::to_string(mouse_pos.y),
                    asw::Vec2(183.0F, 885.0F), green);
  }

  // Inspect window
  else if (mouse_pos.y < camera.size.y - TOOLBAR_HEIGHT &&
           selected_tile != nullptr) {
    auto tile_type = selected_tile->getType();
    auto tile_structure = selected_tile->getStructure();

    if (tile_type != nullptr) {
      asw::draw::text(font, "Tile: " + tile_type->getName(),
                      asw::Vec2(183.0F, 825.0F), green);
    }

    if (tile_structure != nullptr) {
      asw::draw::text(font, tile_structure->getType()->name,
                      asw::Vec2(183.0F, 845.0F), green);

      asw::draw::text(font, tile_structure->getType()->description,
                      asw::Vec2(183.0F, 865.0F), green);
    }

    // Get resources from actions
    std::map<std::string, int> resources;
    int i = 0;
    for (const auto& [res, count] : tile_type->getResources()) {
      if (res.empty()) {
        continue;
      }

      asw::draw::text(font, "Drop: " + res + " x " + std::to_string(count),
                      asw::Vec2(183.0F, 885.0F + (i * 20)), green);
      i++;
    }
  }

  // Monkey head
  else if (mouse_pos.x < 170.0F) {
    asw::draw::text(font, "J1M Says:", asw::Vec2(183.0F, 825.0F), green);

    if (world.getProgression() < 0.01F) {
      asw::draw::text(font, "I am J1M, your guide.", asw::Vec2(183.0F, 845.0F),
                      green);
      asw::draw::text(font, "I will help you survive.",
                      asw::Vec2(183.0F, 865.0F), green);
      asw::draw::text(font, "Right click to set a waypoint.",
                      asw::Vec2(183.0F, 885.0F), green);
      asw::draw::text(font, "Left click to interact.",
                      asw::Vec2(183.0F, 905.0F), green);
      asw::draw::text(font, "Your goal is to purify the planet.",
                      asw::Vec2(183.0F, 925.0F), green);
    } else if (world.getProgression() < 0.5F) {
      asw::draw::text(font, "You are doing great!", asw::Vec2(183.0F, 845.0F),
                      green);
      asw::draw::text(font, "Keep up the good work!", asw::Vec2(183.0F, 865.0F),
                      green);
    } else {
      asw::draw::text(font, "You are almost there!", asw::Vec2(183.0F, 845.0F),
                      green);
      asw::draw::text(font, "Keep going!", asw::Vec2(183.0F, 865.0F), green);
    }

  } else if (drill_button_trans.contains(mouse_pos)) {
    asw::draw::text(font, "Drill: ", asw::Vec2(183.0F, 825.0F), green);

    asw::draw::text(font, "This tool is used to destroy scrap.",
                    asw::Vec2(183.0F, 845.0F), green);
    asw::draw::text(font, "It is very useful for clearing the area ",
                    asw::Vec2(183.0F, 865.0F), green);
    asw::draw::text(font, "and collecting resources.",
                    asw::Vec2(183.0F, 885.0F), green);
  } else if (purifier_button_trans.contains(mouse_pos)) {
    asw::draw::text(font, "Purifier: ", asw::Vec2(183.0F, 825.0F), green);

    asw::draw::text(font, "This tool is used to place a water ",
                    asw::Vec2(183.0F, 845.0F), green);
    asw::draw::text(font, "purifier. Water purifiers slowly ",
                    asw::Vec2(183.0F, 865.0F), green);
    asw::draw::text(font, "purify water and must be placed on water.",
                    asw::Vec2(183.0F, 885.0F), green);

    asw::draw::text(font, "Cost: 10 scrap", asw::Vec2(183.0F, 905.0F), green);
  } else if (tree_button_trans.contains(mouse_pos)) {
    asw::draw::text(font, "Tree: ", asw::Vec2(183.0F, 825.0F), green);

    asw::draw::text(font, "This tool is used to plant trees.",
                    asw::Vec2(183.0F, 845.0F), green);
    asw::draw::text(font, "Trees will slowly purify soil.",
                    asw::Vec2(183.0F, 865.0F), green);

    asw::draw::text(font, "Cost: 10 biomass", asw::Vec2(183.0F, 905.0F), green);
  }

  // Buttons
  asw::draw::stretchSprite(purifier_button, purifier_button_trans);
  asw::draw::textCenter(
      font, "Purifier",
      purifier_button_trans.position + asw::Vec2(32.0F, 74.0F), white);

  asw::draw::stretchSprite(tree_button, tree_button_trans);
  asw::draw::textCenter(font, "Tree",
                        tree_button_trans.position + asw::Vec2(32.0F, 74.0F),
                        white);

  asw::draw::stretchSprite(drill_button, drill_button_trans);
  asw::draw::textCenter(font, "Drill",
                        drill_button_trans.position + asw::Vec2(32.0F, 74.0F),
                        white);

  // Percent purified
  auto percent_purified = std::to_string(world.getProgression() * 100.0F);
  asw::draw::text(font, "Purification: " + percent_purified + "%",
                  asw::Vec2(620.0F, 920.0F), white);

  asw::draw::rectFill(asw::Quad(620.0F, 944.0F, 200.0F, 2.0F),
                      asw::util::makeColor(32, 32, 32));
  asw::draw::rectFill(
      asw::Quad(620.0F, 944.0F, 200.0F * world.getProgression(), 2.0F), green);

  if (mode == ToolMode::PURIFIER) {
    asw::draw::rect(purifier_button_trans, asw::util::makeColor(255, 255, 0));
  } else if (mode == ToolMode::TREE) {
    asw::draw ::rect(tree_button_trans, asw::util::makeColor(255, 255, 0));
  } else if (mode == ToolMode::DRILL) {
    asw::draw::rect(drill_button_trans, asw::util::makeColor(255, 255, 0));
  }

  if (actionProgress > 0) {
    asw::draw::rectFill(asw::Quad(mouse_pos.x, mouse_pos.y - 60, 208.0F, 38.0F),
                        asw::util::makeColor(0, 0, 0));

    asw::draw::rectFill(
        asw::Quad(mouse_pos.x + 4.0F, mouse_pos.y + 4 - 60, actionProgress * 2,
                  30.0F),
        asw::util::makeColor(255 - (255 / 100) * actionProgress,
                             255 + (255 / 100) * actionProgress, 0));

    asw::draw::text(font, "Drilling...",
                    asw::Vec2(mouse_pos.x + 4.0F, mouse_pos.y - 60 + 4.0F),
                    asw::util::makeColor(255, 255, 255));
  }

  if (world.getProgression() < 0.33F) {
    asw::draw::setAlpha(overlay_1, 1);
    asw::draw::sprite(overlay_1, asw::Vec2<float>(0, 0));
  }

  if (world.getProgression() >= 0.33F && world.getProgression() < 0.66F) {
    asw::draw::setAlpha(overlay_1, 0.5F);
    asw::draw::sprite(overlay_1, asw::Vec2<float>(0, 0));
  }
}

void Toolbar::drawWireframe(const asw::Vec3<int>& position,
                            const asw::Vec2<float>& offset,
                            asw::Color colour) {
  // Calc screen position
  auto iso_x = isoX(position);
  auto iso_y = isoY(position);
  auto screen_pos = asw::Vec2(iso_x, iso_y) * TILE_HEIGHT - offset;

  // Draw of top of tile
  asw::draw::line(screen_pos + asw::Vec2(0.0F, TILE_HEIGHT_F / 2),
                  screen_pos + asw::Vec2(TILE_WIDTH_F / 2, 0.0F), colour);

  asw::draw::line(screen_pos + asw::Vec2(TILE_WIDTH_F / 2, 0.0F),
                  screen_pos + asw::Vec2(TILE_WIDTH_F, TILE_HEIGHT_F / 2),
                  colour);

  asw::draw::line(screen_pos + asw::Vec2(TILE_WIDTH_F, TILE_HEIGHT_F / 2),
                  screen_pos + asw::Vec2(TILE_WIDTH_F / 2, TILE_HEIGHT_F),
                  colour);

  asw::draw::line(screen_pos + asw::Vec2(TILE_WIDTH_F / 2, TILE_HEIGHT_F),
                  screen_pos + asw::Vec2(0.0F, TILE_HEIGHT_F / 2), colour);
}