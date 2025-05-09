#include "tile_map.h"

#include <cmath>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>

#include "../vendor/simplex_noise.h"

asw::Vec3<int> TileMap::getSize() const {
  return {MAP_WIDTH, MAP_HEIGHT, MAP_DEPTH};
}

void TileMap::update(float deltaTime) {
  // Z Layering
  z_focus += asw::input::mouse.z;

  if (asw::input::wasKeyPressed(asw::input::Key::KP_MINUS)) {
    z_focus -= 1;
  }

  if (asw::input::wasKeyPressed(asw::input::Key::KP_PLUS)) {
    z_focus += 1;
  }

  z_focus = std::min(std::max(z_focus, 0), MAP_HEIGHT);
}

void TileMap::generate() {
  // Init map
  for (int i = 0; i < MAP_WIDTH; ++i) {
    for (int j = 0; j < MAP_DEPTH; ++j) {
      for (int k = 0; k < MAP_HEIGHT; ++k) {
        mapTiles[i][j][k] = Tile();
        auto position = asw::Vec3<int>(i, j, k);
        mapTiles[i][j][k].setPosition(position);
      }
    }
  }

  // Height
  auto height_map = SimplexNoise(0.01f, 0.01f, 2.0f, 0.47f);
  auto seed = asw::random::between(0.0F, 10000.0F);

  // Dirt grass and rocks
  for (int i = 0; i < MAP_WIDTH; ++i) {
    for (int j = 0; j < MAP_DEPTH; ++j) {
      auto frac = height_map.fractal(10, seed + i, seed + j);
      auto height_val = static_cast<int>(MAP_HEIGHT * (frac + 1.0F) / 2.0F);

      for (int k = 0; k < MAP_HEIGHT; ++k) {
        if (k < height_val) {
          // Grass if at top of ground, unless water
          if (k + 1 >= height_val && k >= 3) {
            mapTiles[i][j][k].setType("ground_grass");
          } else if (k + 3 >= height_val) {
            mapTiles[i][j][k].setType("ground");
          } else {
            mapTiles[i][j][k].setType("rocks");
          }
        }

        else if (k < 4) {
          mapTiles[i][j][k].setType("water");
        }

        else if (k - 1 < height_val) {
          if (asw::random::chance(0.2F)) {
            mapTiles[i][j][k].setType("plant_1");
          }
        }
      }
    }
  }
}

Tile* TileMap::getTileAt(const asw::Vec2<float>& position) {
  return getTileAtIndex(getIndexAt(position));
}

Tile* TileMap::getTileAtIndex(const asw::Vec3<int>& index) {
  // Check bounds
  if (index.x < 0 || index.x >= MAP_WIDTH || index.y < 0 ||
      index.y >= MAP_DEPTH) {
    return nullptr;
  }

  return &mapTiles[index.x][index.y][index.z];
}

asw::Vec3<int> TileMap::getIndexAt(const asw::Vec2<float>& position) {
  // "Clamp" position coords to tile coords
  auto sx = position.x / (TILE_SIZE / 2.0F) - 0.5F;
  auto sy = position.y / (TILE_SIZE / 2.0F) - 0.5F;

  // Calculate screen coordinates to world coordinates using iso coords
  auto iso_y = static_cast<int>(std::round((2 * (sy + z_focus) - sx) / 2.0F));
  auto iso_x = static_cast<int>(std::round(iso_y + sx));

  // Return vec
  return {iso_x, iso_y, z_focus};
}

// Draw at position
void TileMap::draw(const asw::Quad<float>& camera) {
  for (int i = 0; i < MAP_HEIGHT; ++i) {
    draw_layer(camera, i);
  }
}

// Draw a layer
void TileMap::draw_layer(const asw::Quad<float>& camera, int layer) {
  if (layer > z_focus || layer < z_focus - 4) {
    return;
  }

  for (int i = 0; i < MAP_WIDTH; ++i) {
    for (int j = 0; j < MAP_DEPTH; ++j) {
      auto tile = mapTiles[i][j][layer];

      if (tile.getType() == nullptr) {
        continue;
      }

      if (layer == z_focus && layer < MAP_HEIGHT - 1 &&
          mapTiles[i][j][layer + 1].getType() != nullptr &&
          mapTiles[i][j][layer + 1].getType()->isOpaque()) {
        tile.drawHidden(camera.position);
      } else {
        tile.draw(camera.position);
      }
    }
  }

  // Draw depth layer
  // Draw semi-transparent buffer
  SDL_SetRenderDrawBlendMode(asw::display::renderer, SDL_BLENDMODE_BLEND);
  asw::draw::rectFill(asw::Quad<float>(0, 0, camera.size.x, camera.size.y),
                      asw::util::makeColor(0, 0, 0, (z_focus - layer) * 32));
  SDL_SetRenderDrawBlendMode(asw::display::renderer, SDL_BLENDMODE_BLEND);
}