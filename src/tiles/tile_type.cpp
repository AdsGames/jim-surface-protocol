#include "tile_type.h"

#include <algorithm>

/// PRERENDER
///
/// Define vertices for prerendered assets
///

// Draw as a cube
const asw::Vec3<int> CUBE_VERTS[8] = {
    {0, 0, 0},  // 0
    {1, 0, 0},  // 1
    {1, 1, 0},  // 2
    {0, 1, 0},  // 3
    {0, 0, 1},  // 4
    {1, 0, 1},  // 5
    {1, 1, 1},  // 6
    {0, 1, 1},  // 7
};

// Cube faces
const asw::Vec3<int> CUBE_FACES[3][4] = {
    {CUBE_VERTS[4], CUBE_VERTS[5], CUBE_VERTS[6], CUBE_VERTS[7]},  // top
    {CUBE_VERTS[7], CUBE_VERTS[6], CUBE_VERTS[2], CUBE_VERTS[3]},  // left
    {CUBE_VERTS[5], CUBE_VERTS[6], CUBE_VERTS[2], CUBE_VERTS[1]},  // right
};

// Face Colour Bias
const asw::Vec3<float> CUBE_FACE_COLOUR[3] = {
    {1.0F, 1.0F, 1.0F},  // top
    {0.8F, 0.8F, 0.8F},  // left
    {0.5F, 0.5F, 0.5F},  // right
};

// triangles: 0-1-2 and 2-3-0
const int RENDER_ORDER[] = {0, 1, 2, 2, 3, 0};

/// TILE TYPE
///
/// Tile type methods
///

TileType::TileType(short id, const std::string& name, const std::string& id_str)
    : id(id), name(name), id_str(id_str) {}

short TileType::getId() const {
  return id;
}

const std::string& TileType::getIdString() const {
  return id_str;
}

const std::string& TileType::getName() const {
  return name;
}

const asw::Quad<float>& TileType::getBoundingBox() const {
  return bounds;
}

void TileType::addImage(asw::Texture image) {
  if (image) {
    images.push_back(image);
  }
}

void TileType::addAction(const ActionResult& action) {
  actions.push_back(action);
}

/// Its cubin' time
/// ...zz...
/// .xzzzzy.
/// .xxzzyy.
/// .xxxyyy.
/// .xxxyyy.
/// ..xxyy..
/// ...xy...
///
void TileType::draw(const asw::Vec3<int>& position,
                    const asw::Vec2<float>& offset,
                    bool left_border,
                    bool right_border,
                    bool selected) {
  // Render image
  if (image == nullptr) {
    return;
  }

  // Calc screen position
  auto iso_x = (isoX(position) * TILE_HEIGHT) - offset.x;
  if (iso_x < -TILE_SIZE || iso_x > 1280) {
    return;
  }

  auto iso_y = (isoY(position) * TILE_HEIGHT) - offset.y;
  if (iso_y < -TILE_SIZE || iso_y > 960) {
    return;
  }

  auto iso_pos = asw::Vec2(iso_x, iso_y);

  if (render_mode == TileRenderMode::CUBE_TOP_ONLY) {
    iso_pos.y += TILE_SIZE * 0.2F;
  }

  asw::draw::sprite(image, iso_pos);

  if (selected) {
    asw::draw::set_blend_mode(image, asw::BlendMode::Add);
    asw::draw::sprite(image, iso_pos);
    asw::draw::set_blend_mode(image, asw::BlendMode::Blend);
  }

  if (left_border && render_mode != TileRenderMode::FLAT) {
    asw::draw::line(iso_pos + asw::Vec2(0.0F, TILE_HEIGHT_F / 2),
                    iso_pos + asw::Vec2(TILE_WIDTH_F / 2, 0.0F),
                    asw::Color(0, 0, 0, 128));
  }

  if (right_border && render_mode != TileRenderMode::FLAT) {
    asw::draw::line(iso_pos + asw::Vec2(TILE_WIDTH_F / 2, 0.0F),
                    iso_pos + asw::Vec2(TILE_WIDTH_F, TILE_HEIGHT_F / 2),
                    asw::Color(0, 0, 0, 128));
  }
}

void TileType::bakeTexture(TileRenderMode mode, float alpha) {
  auto width = TILE_SIZE;
  auto height = TILE_SIZE;

  // Render image
  image = asw::assets::create_texture(width, height);

  asw::draw::set_blend_mode(image, asw::BlendMode::Blend);
  SDL_SetTextureScaleMode(image.get(), SDL_SCALEMODE_NEAREST);

  // Set the render target to the image
  asw::display::set_render_target(image);

  // Fill transparent
  asw::draw::rect_fill(asw::Quad<float>(0.0F, 0.0F, height, width),
                       asw::Color(0, 0, 0, 0));

  // Render cube
  if (mode == TileRenderMode::CUBE) {
    renderCube(1, 3);
  }

  else if (mode == TileRenderMode::CUBE_UNIQUE_TOP) {
    renderCube(2, 3);
  }

  // Render cube top only
  else if (mode == TileRenderMode::CUBE_TOP_ONLY) {
    renderCube(1, 1);
  }

  // Render flat
  else if (mode == TileRenderMode::FLAT) {
    renderFlat();
  }

  // Set render mode
  render_mode = mode;

  // Reset the render target to the default
  asw::display::set_render_target(nullptr);

  if (alpha < 1.0F) {
    SDL_SetTextureAlphaModFloat(image.get(), alpha);
  }
}

void TileType::renderCube(int texture_count, int face_count) {
  if (face_count > 3 || face_count < 1) {
    face_count = 3;
  }

  // Iterate over the faces
  for (int f = 0; f < face_count; f++) {
    SDL_Vertex verts[4];

    for (int i = 0; i < 4; i++) {
      verts[i].position = project(CUBE_FACES[f][i]);

      // Colour
      verts[i].color.a = 1.0F;
      verts[i].color.r = CUBE_FACE_COLOUR[f].x;
      verts[i].color.g = CUBE_FACE_COLOUR[f].y;
      verts[i].color.b = CUBE_FACE_COLOUR[f].z;
    }

    verts[0].tex_coord = {0.0f, 0.0f};  // top-left
    verts[1].tex_coord = {1.0f, 0.0f};  // top-right
    verts[2].tex_coord = {1.0f, 1.0f};  // bottom-right
    verts[3].tex_coord = {0.0f, 1.0f};  // bottom-left

    // Special top face
    if (f == 0 && texture_count == 2 && images.size() > 1) {
      SDL_RenderGeometry(asw::display::renderer, images.at(1).get(), verts, 4,
                         RENDER_ORDER, 6);
    }

    // Default cube
    else {
      SDL_RenderGeometry(asw::display::renderer, images.at(0).get(), verts, 4,
                         RENDER_ORDER, 6);
    }
  }
}

void TileType::renderFlat() {
  asw::draw::stretch_sprite(images.at(0),
                            asw::Quad<float>(0.0F, 0.0F, TILE_SIZE, TILE_SIZE));
}