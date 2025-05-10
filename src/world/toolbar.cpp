#include "toolbar.h"

#include <cmath>

void Toolbar::init() {
  font = asw::assets::loadFont("assets/fonts/ariblk.ttf", 16);
  inspect_button =
      asw::assets::loadTexture("assets/images/ui/inspect-button.png");
  worker_button =
      asw::assets::loadTexture("assets/images/ui/worker-button.png");
  purifier_button =
      asw::assets::loadTexture("assets/images/ui/purify-button.png");
  tree_button = asw::assets::loadTexture("assets/images/ui/tree-button.png");
  drill_button = asw::assets::loadTexture("assets/images/ui/drill-button.png");
};

void Toolbar::update(float dt, World& world) {
  auto& camera = world.getCamera();
  auto& tile_map = world.getTileMap();
  auto mouse_pos = asw::Vec2(asw::input::mouse.x, asw::input::mouse.y);
  cursor_idx = tile_map.getIndexAt(camera.position + mouse_pos);

  // Update transofmrs

  // T R A N S O F M R S

  // M O O G E T R O N

  inspect_button_trans.position.y = camera.size.y - 70;
  worker_button_trans.position.y = camera.size.y - 70;
  purifier_button_trans.position.y = camera.size.y - 70;
  tree_button_trans.position.y = camera.size.y - 70;
  drill_button_trans.position.y = camera.size.y - 70;

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
  if (asw::input::isButtonDown(asw::input::MouseButton::LEFT)) {
    toolZoneAction(world);
    actionProgress += dt * 0.5F;
    if (actionProgress > 100.0F) {
      action(world);
      actionProgress = 0;
    }

  } else {
    actionProgress = 0;
  }

  if (asw::input::isButtonDown(asw::input::MouseButton::RIGHT)) {
    rightClickAction(world);
  }

  // Hacker mode
  if (asw::input::wasKeyPressed(asw::input::Key::M)) {
    auto tile = tile_map.getTileAtIndex(cursor_idx);
    if (tile != nullptr) {
      tile->setType("machine");
    }
  }

  if (asw::input::wasKeyPressed(asw::input::Key::N)) {
    auto tile = tile_map.getTileAtIndex(cursor_idx);
    if (tile != nullptr) {
      tile->setType("rocks_rough");
    }
  }
}

void Toolbar::rightClickAction(World& world) {
  world.setPlayerWaypoint(cursor_idx);
  world.setWaypointActive(true);
}

void Toolbar::toolZoneAction(World& world) {
  auto& camera = world.getCamera();
  auto& tile_map = world.getTileMap();
  auto& resource_manager = world.getResourceManager();
  auto mouse_pos = asw::Vec2(asw::input::mouse.x, asw::input::mouse.y);
  // Tool zone (I didn't know danny had a zone) HA GOTTEEEEEEEEEEEEEEM
  if (mouse_pos.y > camera.size.y - 80.0F) {
    if (inspect_button_trans.contains(mouse_pos)) {
      mode = ToolMode::INSPECT;
    } else if (worker_button_trans.contains(mouse_pos)) {
      mode = ToolMode::WORKER;
    } else if (purifier_button_trans.contains(mouse_pos)) {
      mode = ToolMode::PURIFIER;
    } else if (tree_button_trans.contains(mouse_pos)) {
      mode = ToolMode::TREE;
    } else if (drill_button_trans.contains(mouse_pos)) {
      mode = ToolMode::DRILL;
    }
  }
}

void Toolbar::action(World& world) {
  auto& camera = world.getCamera();
  auto& tile_map = world.getTileMap();
  auto& resource_manager = world.getResourceManager();
  auto mouse_pos = asw::Vec2(asw::input::mouse.x, asw::input::mouse.y);

  // Find selected tile
  auto selected_tile = tile_map.getTileAtIndex(cursor_idx);
  std::shared_ptr<TileType> select_type = nullptr;
  if (selected_tile != nullptr) {
    select_type = selected_tile->getType();
  }

  // Tile zone
  if (mouse_pos.y > camera.size.y - 80.0F) {
    // take a huge steaming stinky poo on the floor
  } else if (cursor_in_range) {
    if (mode == ToolMode::DRILL && selected_tile != nullptr &&
        select_type != nullptr) {
      auto actions = select_type->getActionsOfType(ActionType::DESTROY);
      for (const auto& action : actions) {
        if (action.drop_resource_id != "") {
          resource_manager.addResourceCount(action.drop_resource_id, 1);
        }
      }
      selected_tile->setType(0);
    }

    if (mode == ToolMode::WORKER) {
      world.addWorker(cursor_idx);
    }

    if (mode == ToolMode::PURIFIER && selected_tile != nullptr &&
        select_type != nullptr) {
      auto actions = select_type->getActionsOfType(ActionType::PURIFY);
      for (const auto& action : actions) {
        if (action.transition_tile_id != "") {
          selected_tile->setType(action.transition_tile_id);
        }
      }
    }
  }
}

void Toolbar::draw(World& world) {
  auto& camera = world.getCamera();
  auto& tile_map = world.getTileMap();
  auto mouse_pos = asw::Vec2(asw::input::mouse.x, asw::input::mouse.y);
  auto world_pos = camera.position + mouse_pos;
  auto selected_tile = tile_map.getTileAt(world_pos);
  auto text_colour = asw::util::makeColor(255, 255, 255, 255);  // White

  // Outline
  if (cursor_in_range) {
    drawWireframe(cursor_idx, camera.position, asw::util::makeColor(0, 255, 0));
  } else {
    drawWireframe(cursor_idx, camera.position, asw::util::makeColor(255, 0, 0));
  }

  asw::draw::rectFill(
      asw::Quad(0.0F, camera.size.y - 80.0F, camera.size.x, camera.size.y),
      asw::util::makeColor(128, 128, 128));

  // Buttons
  asw::draw::stretchSprite(inspect_button, inspect_button_trans);
  asw::draw::stretchSprite(worker_button, worker_button_trans);
  asw::draw::stretchSprite(purifier_button, purifier_button_trans);
  asw::draw::stretchSprite(tree_button, tree_button_trans);
  asw::draw::stretchSprite(drill_button, drill_button_trans);

  if (mode == ToolMode::INSPECT) {
    asw::draw::rect(inspect_button_trans, asw::util::makeColor(255, 255, 0));
  } else if (mode == ToolMode::WORKER) {
    asw::draw::rect(worker_button_trans, asw::util::makeColor(255, 255, 0));

  } else if (mode == ToolMode::PURIFIER) {
    asw::draw::rect(purifier_button_trans, asw::util::makeColor(255, 255, 0));
  } else if (mode == ToolMode::TREE) {
    asw::draw ::rect(tree_button_trans, asw::util::makeColor(255, 255, 0));
  } else if (mode == ToolMode::DRILL) {
    asw::draw::rect(drill_button_trans, asw::util::makeColor(255, 255, 0));
  }

  // Inspect View
  if (mode == ToolMode::INSPECT) {
    asw::draw::rectFill(asw::Quad(0.0F, 0.0F, 200.0F, 200.0F),
                        asw::util::makeColor(64, 64, 0));

    // X + Y
    asw::draw::text(font, "X: " + std::to_string(cursor_idx.x),
                    asw::Vec2(10.0F, 100.0F), text_colour);
    asw::draw::text(font, "Y: " + std::to_string(cursor_idx.y),
                    asw::Vec2(10.0F, 120.0F), text_colour);
    asw::draw::text(font, "Z: " + std::to_string(cursor_idx.z),
                    asw::Vec2(10.0F, 140.0F), text_colour);
  }

  // Inspect mode
  if (selected_tile != nullptr) {
    auto tile_type = selected_tile->getType();
    auto tile_structure = selected_tile->getStructure();

    if (tile_type != nullptr) {
      asw::draw::text(font, "Info: " + tile_type->getName(),
                      asw::Vec2(10.0F, 80.0F), text_colour);
    }

    if (tile_structure != nullptr) {
      asw::draw::text(font, "Structure: " + tile_structure->getType()->name,
                      asw::Vec2(10.0F, 140.0F), text_colour);

      asw::draw::text(font,
                      "Structure: " + tile_structure->getType()->description,
                      asw::Vec2(10.0F, 160.0F), text_colour);
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