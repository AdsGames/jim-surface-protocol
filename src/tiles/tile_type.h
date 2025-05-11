#pragma once

#include <asw/asw.h>
#include <bitset>
#include <string>
#include <vector>

#include "../lib/project.h"
#include "../tasks/action.h"

constexpr int TILE_SIZE = 64;
constexpr int TILE_WIDTH = TILE_SIZE;
constexpr int TILE_HEIGHT = TILE_SIZE / 2;
constexpr float TILE_WIDTH_F = static_cast<float>(TILE_WIDTH);
constexpr float TILE_HEIGHT_F = static_cast<float>(TILE_HEIGHT);

enum class TileRenderMode {
  NONE,
  CUBE,
  CUBE_UNIQUE_TOP,
  CUBE_TOP_ONLY,
  FLAT,
};

class TileType {
 public:
  TileType(short id, const std::string& name, const std::string& id_str);

  void draw(const asw::Vec3<int>& position,
            const asw::Vec2<float>& offset,
            bool left_border,
            bool right_border);

  short getId() const;
  const std::string& getName() const;
  const std::string& getIdString() const;

  const asw::Quad<float>& getBoundingBox() const;

  void addImage(asw::Texture image);

  // Action Zone
  void addAction(const ActionResult& action);

  const std::vector<ActionResult>& getActions() const { return actions; }

  std::vector<ActionResult> getActionsOfType(ActionType type) const {
    std::vector<ActionResult> results;
    for (const auto& action : actions) {
      if (action.type == type) {
        results.push_back(action);
      }
    }
    return results;
  }

  void bakeTexture(TileRenderMode mode, float alpha);

  bool isOpaque() const {
    return render_mode == TileRenderMode::CUBE ||
           render_mode == TileRenderMode::CUBE_UNIQUE_TOP ||
           render_mode == TileRenderMode::CUBE_TOP_ONLY;
  }

 private:
  void renderCube(int texture_count, int face_count);
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

  std::vector<asw::Texture> images;
  std::vector<asw::Texture> rendered_images;
  asw::Texture image;
  TileRenderMode render_mode{TileRenderMode::NONE};
  std::vector<ActionResult> actions;
};
