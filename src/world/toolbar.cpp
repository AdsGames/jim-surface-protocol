#include "toolbar.h"

void Toolbar::init() {
  font = asw::assets::loadFont("assets/fonts/ariblk.ttf", 16);
  inspect_button =
      asw::assets::loadTexture("assets/images/ui/inspect-button.png");
  trash_button = asw::assets::loadTexture("assets/images/ui/trash-button.png");
  worker_button =
      asw::assets::loadTexture("assets/images/ui/worker-button.png");
};

void Toolbar::update(float dt, World& world) {
  auto& camera = world.getCamera();
  auto& tile_map = world.getTileMap();
  auto mouse_pos = asw::Vec2(asw::input::mouse.x, asw::input::mouse.y);
  cursor_idx = tile_map.getIndexAt(camera.position + mouse_pos);

  // Update transofmrs
  inspect_button_trans.position.y = camera.size.y - 70;
  trash_button_trans.position.y = camera.size.y - 70;
  worker_button_trans.position.y = camera.size.y - 70;

  // Click buttons
  if (asw::input::isButtonDown(asw::input::MouseButton::LEFT)) {
    action(world);
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

void Toolbar::action(World& world) {
  auto& camera = world.getCamera();
  auto& tile_map = world.getTileMap();
  auto mouse_pos = asw::Vec2(asw::input::mouse.x, asw::input::mouse.y);

  // Find selected tile
  auto selected_tile = tile_map.getTileAtIndex(cursor_idx);

  // Tool zone (I didn't know danny had a zone)
  if (mouse_pos.y > camera.size.y - 80.0F) {
    if (inspect_button_trans.contains(mouse_pos)) {
      mode = ToolMode::INSPECT;
    } else if (trash_button_trans.contains(mouse_pos)) {
      mode = ToolMode::TRASH;
    } else if (worker_button_trans.contains(mouse_pos)) {
      mode = ToolMode::WORKER;
    }
  }

  // Tile zone
  else {
    if (mode == ToolMode::TRASH && selected_tile != nullptr) {
      selected_tile->setType(0);
    }

    if (mode == ToolMode::WORKER) {
      world.addWorker(cursor_idx);
    }
  }
}

void Toolbar::draw(World& world) {
  auto& camera = world.getCamera();
  auto& tile_map = world.getTileMap();
  auto mouse_pos = asw::Vec2(asw::input::mouse.x, asw::input::mouse.y);
  auto world_pos = camera.position + mouse_pos;
  auto selected_tile = tile_map.getTileAt(world_pos);

  drawWireframe(cursor_idx, camera.position);

  asw::draw::rectFill(
      asw::Quad(0.0F, camera.size.y - 80.0F, camera.size.x, camera.size.y),
      asw::util::makeColor(128, 128, 128));

  // Buttons
  asw::draw::stretchSprite(inspect_button, inspect_button_trans);
  asw::draw::stretchSprite(trash_button, trash_button_trans);
  asw::draw::stretchSprite(worker_button, worker_button_trans);

  if (mode == ToolMode::INSPECT) {
    asw::draw::rect(inspect_button_trans, asw::util::makeColor(255, 255, 0));

  } else if (mode == ToolMode::TRASH) {
    asw::draw::rect(trash_button_trans, asw::util::makeColor(255, 255, 0));
  } else if (mode == ToolMode::WORKER) {
    asw::draw::rect(worker_button_trans, asw::util::makeColor(255, 255, 0));
  }

  // Inspect View
  if (mode == ToolMode::INSPECT) {
    asw::draw::rectFill(asw::Quad(0.0F, 0.0F, 200.0F, 200.0F),
                        asw::util::makeColor(128, 128, 0));

    // X + Y
    asw::draw::text(font, "X: " + std::to_string(cursor_idx.x),
                    asw::Vec2(10.0F, 100.0F),
                    asw::util::makeColor(255, 255, 255));
    asw::draw::text(font, "Y: " + std::to_string(cursor_idx.y),
                    asw::Vec2(10.0F, 120.0F),
                    asw::util::makeColor(255, 255, 255));
  }

  if (selected_tile != nullptr && selected_tile->getType() != nullptr) {
    asw::draw::text(font, "Info: " + selected_tile->getType()->getName(),
                    asw::Vec2(10.0F, 80.0F),
                    asw::util::makeColor(255, 255, 255));

    if (selected_tile->getStructure() != nullptr) {
      asw::draw::text(
          font, "Structure: " + selected_tile->getStructure()->getType()->name,
          asw::Vec2(10.0F, 140.0F), asw::util::makeColor(255, 255, 255));

      asw::draw::text(
          font,
          "Structure: " + selected_tile->getStructure()->getType()->description,
          asw::Vec2(10.0F, 160.0F), asw::util::makeColor(255, 255, 255));
    }

    // if (mode == ToolMode::INSPECT) {
    //   selected_tile->getType()->draw(asw::Vec3(0, 0, 0), asw::Vec2(0, 0),
    //   true);
    // }
  }
}

void Toolbar::drawWireframe(const asw::Vec3<int>& position,
                            const asw::Vec2<float>& offset) {
  // Calc screen position
  auto iso_x = isoX(position);
  auto iso_y = isoY(position);
  auto screen_pos = asw::Vec2(iso_x, iso_y) * TILE_HEIGHT - offset;

  // Draw of top of tile
  asw::draw::line(screen_pos + asw::Vec2(0.0F, TILE_HEIGHT_F / 2),
                  screen_pos + asw::Vec2(TILE_WIDTH_F / 2, 0.0F),
                  asw::util::makeColor(255, 255, 0));

  asw::draw::line(screen_pos + asw::Vec2(TILE_WIDTH_F / 2, 0.0F),
                  screen_pos + asw::Vec2(TILE_WIDTH_F, TILE_HEIGHT_F / 2),
                  asw::util::makeColor(255, 0, 255));

  asw::draw::line(screen_pos + asw::Vec2(TILE_WIDTH_F, TILE_HEIGHT_F / 2),
                  screen_pos + asw::Vec2(TILE_WIDTH_F / 2, TILE_HEIGHT_F),
                  asw::util::makeColor(0, 255, 0));

  asw::draw::line(screen_pos + asw::Vec2(TILE_WIDTH_F / 2, TILE_HEIGHT_F),
                  screen_pos + asw::Vec2(0.0F, TILE_HEIGHT_F / 2),
                  asw::util::makeColor(0, 255, 255));
}