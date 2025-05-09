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
  auto world_pos = camera.position + mouse_pos;
  auto world_idx = tile_map.getIndexAt(world_pos);

  // Update transofmrs
  inspect_button_trans.position.y = camera.size.y - 70;
  trash_button_trans.position.y = camera.size.y - 70;
  worker_button_trans.position.y = camera.size.y - 70;

  // Find selected tile
  selected_tile = tile_map.getTileAt(world_pos);

  // Spawn worker
  if (asw::input::wasKeyPressed(asw::input::Key::W)) {
    world.addWorker(world_idx);
  }

  // Click buttons
  if (asw::input::wasButtonPressed(asw::input::MouseButton::LEFT)) {
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
        world.addWorker(world_idx);
      }
    }
  }
}

void Toolbar::draw(World& world) {
  auto& camera = world.getCamera();

  if (selected_tile != nullptr && selected_tile->getType() != nullptr) {
    selected_tile->drawWireframe(camera.position);
    asw::draw::text(font, "Info: " + selected_tile->getType()->getName(),
                    asw::Vec2(10.0F, 80.0F),
                    asw::util::makeColor(255, 255, 255));

    // if (mode == ToolMode::INSPECT) {
    //   selected_tile->getType()->draw(asw::Vec3(0, 0, 0), asw::Vec2(0, 0),
    //   true);
    // }
  }

  asw::draw::rectFill(
      asw::Quad(0.0F, camera.size.y - 80.0F, camera.size.x, camera.size.y),
      asw::util::makeColor(128, 128, 128));

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
}