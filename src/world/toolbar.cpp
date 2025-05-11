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

  upgrade = asw::assets::loadTexture("assets/images/ui/upgrade.png");
};

void Toolbar::update(float dt, World& world) {
  // Enabled actions
  can_take_action = actionEnabled(world);

  // CHEATING ZONE (DOONT LOOK)

  if (asw::input::wasKeyPressed(asw::input::Key::F)) {
    world.getResourceManager().addResourceCount("scrap", 100);
  }

  auto& camera = world.getCamera();
  auto& tile_map = world.getTileMap();
  auto mouse_pos = asw::Vec2(asw::input::mouse.x, asw::input::mouse.y);
  cursor_idx = tile_map.getIndexAt(camera.position + mouse_pos);

  // Update transofmrs

  // T R A N S O F M R S

  // M O O G E T R O N

  inspect_button_trans.position.y = camera.size.y - 70;
  purifier_button_trans.position.y = camera.size.y - 70;
  tree_button_trans.position.y = camera.size.y - 70;
  drill_button_trans.position.y = camera.size.y - 70;
  upgrade_drill_trans.position.y = camera.size.y - 70;
  upgrade_move_trans.position.y = camera.size.y - 40;

  // Find distance to worker
  auto selected_tile = tile_map.getTileAtIndex(cursor_idx);
  auto worker = world.getPlayer();
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
  if (mouse_pos.y > camera.size.y - 80.0F) {
    if (inspect_button_trans.contains(mouse_pos)) {
      mode = ToolMode::INSPECT;
    } else if (purifier_button_trans.contains(mouse_pos)) {
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
        move_upgrade_cost = move_upgrade_cost * 1.5F;
        world.setPlayerSpeed(world.getPlayerSpeed() + 1);
        resource_manager.addResourceCount("scrap", -move_upgrade_cost);
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
  if (mouse_pos.y > camera.size.y - 80.0F) {
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

      actionProgress +=
          dt * (0.05F / density) * (1 + world.getDrillSpeed() * 1.5F);
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
    if (tile != nullptr) {
      tile->setType("machine");
    }
  }

  if (mode == ToolMode::TREE) {
    auto idx = cursor_idx;
    idx.z = idx.z + 1;

    auto* tile = tile_map.getTileAtIndex(idx);
    if (tile != nullptr) {
      tile->setType("sapling");
    }
  }
}

bool Toolbar::actionEnabled(World& world) {
  auto& camera = world.getCamera();
  auto& tile_map = world.getTileMap();
  auto mouse_pos = asw::Vec2(asw::input::mouse.x, asw::input::mouse.y);
  cursor_idx = tile_map.getIndexAt(camera.position + mouse_pos);

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

  // Worker check
  if (mode == ToolMode::INSPECT) {
    return true;
  }

  // Purifier check
  if (mode == ToolMode::PURIFIER && selected_tile != nullptr &&
      select_type != nullptr) {
    if (select_type->getIdString() == "toxic_water" ||
        select_type->getIdString() == "water") {
      return true;
    }
  }

  // Tree check
  if (mode == ToolMode::TREE && selected_tile != nullptr &&
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
  auto mouse_pos = asw::Vec2(asw::input::mouse.x, asw::input::mouse.y);
  auto world_pos = camera.position + mouse_pos;
  auto* selected_tile = tile_map.getTileAt(world_pos);
  auto text_colour = asw::util::makeColor(255, 255, 255, 255);  // White

  if (cursor_in_range && can_take_action) {
    drawWireframe(cursor_idx, camera.position, asw::util::makeColor(0, 255, 0));
  } else if (cursor_in_range) {
    drawWireframe(cursor_idx, camera.position,
                  asw::util::makeColor(255, 255, 255));
  } else {
    drawWireframe(cursor_idx, camera.position, asw::util::makeColor(255, 0, 0));
  }

  // Border
  asw::draw::rectFill(
      asw::Quad(0.0F, camera.size.y - 80.0F, camera.size.x, camera.size.y),
      asw::util::makeColor(128, 128, 128));

  // X + Y
  asw::draw::text(font,
                  "Pos: " + std::to_string(cursor_idx.x) + ", " +
                      std::to_string(cursor_idx.y) + ", " +
                      std::to_string(cursor_idx.z),
                  asw::Vec2(10.0F, 100.0F), text_colour);

  // Camera pos
  asw::draw::text(font,
                  "Cam: " + std::to_string(camera.position.x) + ", " +
                      std::to_string(camera.position.y),
                  asw::Vec2(10.0F, 115.0F), text_colour);

  // Percent purified
  auto percent_purified = std::to_string(world.getProgression() * 100.0F);
  asw::draw::text(font, "Purity: " + percent_purified + "%",
                  asw::Vec2(10.0F, 130.0F), text_colour);

  // Buttons
  asw::draw::stretchSprite(inspect_button, inspect_button_trans);
  asw::draw::stretchSprite(purifier_button, purifier_button_trans);
  asw::draw::stretchSprite(tree_button, tree_button_trans);
  asw::draw::stretchSprite(drill_button, drill_button_trans);

  auto drill_speed = std::to_string(world.getDrillSpeed());
  auto player_speed = std::to_string(world.getPlayerSpeed());

  auto canBuyDrill = world.getResourceManager().getResourceCount("scrap") >=
                     drill_upgrade_cost;
  auto canBuyMove =
      world.getResourceManager().getResourceCount("scrap") >= move_upgrade_cost;
  auto canBuyColour = asw::util::makeColor(150, 255, 150, 255);
  auto cantBuyColour = asw::util::makeColor(255, 150, 150, 255);

  asw::draw::text(fontLarge, "Drill Speed: " + drill_speed,
                  asw::Vec2(500.0F, camera.size.y - 70),
                  asw::util::makeColor(255, 255, 255, 255));

  asw::draw::text(fontLarge,
                  "Upgrade:   " + std::to_string(drill_upgrade_cost) + " scrap",
                  asw::Vec2(800.0F, camera.size.y - 70),
                  canBuyDrill ? canBuyColour : cantBuyColour);

  asw::draw::sprite(upgrade, asw::Vec2<float>(upgrade_drill_trans.position.x,
                                              upgrade_drill_trans.position.y));

  asw::draw::sprite(upgrade, asw::Vec2<float>(upgrade_move_trans.position.x,
                                              upgrade_move_trans.position.y));

  asw::draw::sprite(upgrade, asw::Vec2<float>(935.0F, camera.size.y - 40));

  asw::draw::text(fontLarge,
                  "Upgrade:   " + std::to_string(move_upgrade_cost) + " scrap",
                  asw::Vec2(800.0F, camera.size.y - 40),
                  canBuyMove ? canBuyColour : cantBuyColour);

  asw::draw::text(fontLarge, "Move Speed: " + player_speed,
                  asw::Vec2(500.0F, camera.size.y - 40),
                  asw::util::makeColor(255, 255, 255, 255));

  if (mode == ToolMode::INSPECT) {
    asw::draw::rect(inspect_button_trans, asw::util::makeColor(255, 255, 0));
  } else if (mode == ToolMode::PURIFIER) {
    asw::draw::rect(purifier_button_trans, asw::util::makeColor(255, 255, 0));
  } else if (mode == ToolMode::TREE) {
    asw::draw ::rect(tree_button_trans, asw::util::makeColor(255, 255, 0));
  } else if (mode == ToolMode::DRILL) {
    asw::draw::rect(drill_button_trans, asw::util::makeColor(255, 255, 0));
  }

  // Inspect View
  if (mode == ToolMode::INSPECT) {
    asw::draw::rectFill(asw::Quad(mouse_pos.x, mouse_pos.y, 500.0F, 150.0F),
                        asw::util::makeColor(0, 0, 0));

    // Inspect mode
    if (selected_tile != nullptr) {
      auto tile_type = selected_tile->getType();
      auto tile_structure = selected_tile->getStructure();

      if (tile_type != nullptr) {
        asw::draw::text(font, "Info: " + tile_type->getName(),
                        mouse_pos + asw::Vec2(10.0F, 10.0F), text_colour);
      }

      if (tile_structure != nullptr) {
        asw::draw::text(font, tile_structure->getType()->name,
                        mouse_pos + asw::Vec2(10.0F, 30.0F), text_colour);

        asw::draw::text(font, tile_structure->getType()->description,
                        mouse_pos + asw::Vec2(10.0F, 60.0F), text_colour);
      }
    }
  }

  drawResourceWindow(world);

  if (actionProgress > 0) {
    asw::draw::rectFill(asw::Quad(mouse_pos.x, mouse_pos.y, 204.0F, 34.0F),
                        asw::util::makeColor(128, 128, 128));

    asw::draw::rectFill(asw::Quad(mouse_pos.x + 2.0F, mouse_pos.y + 2,
                                  actionProgress * 2, 30.0F),
                        asw::util::makeColor(0, 255, 0));
  }
}

void Toolbar::drawResourceWindow(World& world) {
  auto& resource_manager = world.getResourceManager();
  auto& camera = world.getCamera();

  const int offset_x = -170.0F;
  auto& resources = resource_manager.getResources();

  asw::draw::rectFill(
      asw::Quad(camera.size.x + offset_x - 20.0F, 0.0F, 200.0F, 200.0F),
      asw::util::makeColor(64, 64, 0));

  asw::draw::text(font, "Resources", asw::Vec2(camera.size.x + offset_x, 10.0F),
                  asw::util::makeColor(255, 255, 255, 255));

  int index = 0;
  for (const auto& [key, resource] : resources) {
    const auto y_pos = 50.0F + index * 20;

    asw::draw::stretchSprite(
        resource->icon,
        asw::Quad(camera.size.x + offset_x, y_pos + 10.0F, 10.0F, 10.0F));

    asw::draw::text(font,
                    resource->name + ": " + std::to_string(resource->amount),
                    asw::Vec2(camera.size.x + offset_x + 20.0F, y_pos),
                    asw::util::makeColor(255, 255, 255, 255));

    index++;
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