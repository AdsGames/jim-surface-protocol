#pragma once

#include <asw/asw.h>
#include <bitset>
#include <string>
#include <vector>

constexpr int ATTRIBUTE_MAX = 8;
constexpr int TILE_SIZE = 64;
constexpr float TILE_WIDTH = TILE_SIZE;
constexpr float TILE_HEIGHT = TILE_SIZE / 2.0f;

enum class TileRenderMode {
  CUBE,
  CUBE_UNIQUE_TOP,
  FLAT,
};

class TileType {
 public:
  TileType(short id, const std::string& name, const std::string& id_str);

  void draw(const asw::Vec3<float>& position, const asw::Vec2<float>& offset);
  void drawWireframe(const asw::Vec3<float>& position,
                     const asw::Vec2<float>& offset);

  short getId() const;
  std::string getName() const;
  std::string getIdString() const;

  const asw::Quad<float>& getBoundingBox() const;

  bool hasAttribute(int attribute);

  void addImage(asw::Texture image);
  void addAttribute(int attribute);
  void setRenderMode(TileRenderMode mode);

  void bakeTexture();

 private:
  void renderCube();
  void renderFlat();

  float isoX(float x, float y) { return x - y; }

  /// @brief Calculate the Y coordinate of a point in isometric projection.
  /// @param x X coordinate of the point.
  /// @param y Y coordinate of the point.
  /// @param z Z coordinate of the point.
  /// @return Y coordinate of the point in isometric projection.
  float isoY(float x, float y, float z) { return (x + y) / 2 - z; }

  /// Project a 3D vector to 2D screen coordinates
  /// @param v The 3D vector to project.
  /// @param out The output 2D point.
  /// @note The output point is in screen coordinates. The offset is so that
  /// this can be rendered on a texture of TILE_SIZE.
  SDL_FPoint project(const asw::Vec3<float>& v) {
    SDL_FPoint out;
    out.x = isoX(v.x, v.y) * (TILE_SIZE / 2.0F) + TILE_SIZE / 2.0F;
    out.y = isoY(v.x, v.y, v.z) * (TILE_SIZE / 2.0F) + TILE_SIZE / 2.0F;
    return out;
  }

  short id;
  std::string name;
  std::string id_str;
  asw::Quad<float> bounds;

  std::bitset<ATTRIBUTE_MAX> attributes;
  std::vector<asw::Texture> images;
  asw::Texture image;

  TileRenderMode render_mode{TileRenderMode::CUBE};
};
