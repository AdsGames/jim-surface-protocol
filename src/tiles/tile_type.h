#pragma once

#include <asw/asw.h>
#include <bitset>
#include <string>
#include <vector>

#include "../lib/project.h"

constexpr int ATTRIBUTE_MAX = 8;

constexpr int TILE_SIZE = 64;
constexpr int TILE_WIDTH = TILE_SIZE;
constexpr int TILE_HEIGHT = TILE_SIZE / 2;
constexpr float TILE_WIDTH_F = static_cast<float>(TILE_WIDTH);
constexpr float TILE_HEIGHT_F = static_cast<float>(TILE_HEIGHT);

enum class TileRenderMode {
  NONE,
  CUBE,
  CUBE_UNIQUE_TOP,
  FLAT,
};

class TileType {
 public:
  TileType(short id, const std::string& name, const std::string& id_str);

  void draw(const asw::Vec3<int>& position,
            const asw::Vec2<float>& offset,
            bool hidden);

  short getId() const;
  std::string getName() const;
  std::string getIdString() const;

  const asw::Quad<float>& getBoundingBox() const;

  bool hasAttribute(int attribute);

  void addImage(asw::Texture image);
  void addAttribute(int attribute);

  void bakeTexture(TileRenderMode mode, float alpha);
  bool isOpaque() const {
    return render_mode == TileRenderMode::CUBE ||
           render_mode == TileRenderMode::CUBE_UNIQUE_TOP;
  }

 private:
  void renderCube(bool unique_top);
  void renderFlat();

  /// Project a 3D vector to 2D screen coordinates
  /// @param v The 3D vector to project.
  /// @param out The output 2D point.
  /// @note The output point is in screen coordinates. The offset is so that
  /// this can be rendered on a texture of TILE_SIZE.
  SDL_FPoint project(const asw::Vec3<int>& v) {
    SDL_FPoint out;
    out.x = isoX(v) * (TILE_SIZE / 2.0F) + TILE_SIZE / 2.0F;
    out.y = isoY(v) * (TILE_SIZE / 2.0F) + TILE_SIZE / 2.0F;
    return out;
  }

  short id;
  std::string name;
  std::string id_str;
  asw::Quad<float> bounds;

  std::bitset<ATTRIBUTE_MAX> attributes;
  std::vector<asw::Texture> images;
  asw::Texture image;
  TileRenderMode render_mode{TileRenderMode::NONE};
};
