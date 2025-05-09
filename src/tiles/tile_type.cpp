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
    {0.5F, 0.5F, 1.0F},  // left
    {1.0F, 0.5F, 0.5F},  // right
};

// Cross faces
const asw::Vec3<int> FLAT_FACE[4] = {
    CUBE_VERTS[7],
    CUBE_VERTS[5],
    CUBE_VERTS[1],
    CUBE_VERTS[3],
};

// Cross face colour bias
const asw::Vec3<float> FLAT_FACE_COLOUR = {
    0.5F,
    0.5F,
    1.0F,
};

// triangles: 0-1-2 and 2-3-0
const int RENDER_ORDER[] = {0, 1, 2, 2, 3, 0};

/// TILE TYPE
///
/// Tile type methods
///

TileType::TileType(short id, const std::string& name, const std::string& id_str)
    : id(id), name(name), id_str(id_str) {}

auto TileType::getId() const -> short {
  return id;
}

std::string TileType::getIdString() const {
  return id_str;
}

std::string TileType::getName() const {
  return name;
}

const asw::Quad<float>& TileType::getBoundingBox() const {
  return bounds;
}

auto TileType::hasAttribute(int attribute) -> bool {
  if (attribute < 0 || attribute >= ATTRIBUTE_MAX) {
    return false;
  }

  return attributes.test(attribute);
}

void TileType::addImage(asw::Texture image) {
  if (image) {
    images.push_back(image);
  }
}

void TileType::addAttribute(int attribute) {
  if (attribute < 0 || attribute >= ATTRIBUTE_MAX) {
    return;
  }

  attributes.set(attribute);
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
                    bool hidden) {
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
  if (hidden) {
    iso_y += TILE_HEIGHT / 4;
  }

  if (iso_y < -TILE_SIZE || iso_y > 960) {
    return;
  }

  if (hidden) {
    SDL_SetTextureColorMod(image.get(), 128, 128, 128);
  } else {
    SDL_SetTextureColorMod(image.get(), 255, 255, 255);
  }

  asw::draw::sprite(image, asw::Vec2(iso_x, iso_y));
}

void TileType::bakeTexture(TileRenderMode mode, float alpha) {
  // Render image
  // 2x is because the texture is 2x the size of the world space of the tile
  image = asw::assets::createTexture(TILE_SIZE, TILE_SIZE);
  SDL_SetTextureBlendMode(image.get(), SDL_BLENDMODE_BLEND);
  SDL_SetTextureScaleMode(image.get(), SDL_SCALEMODE_NEAREST);

  // Set the render target to the image
  asw::display::setRenderTarget(image);

  // Render cube
  if (mode == TileRenderMode::CUBE) {
    renderCube(false);
  }

  else if (mode == TileRenderMode::CUBE_UNIQUE_TOP) {
    renderCube(true);
  }

  // Render flat
  else if (mode == TileRenderMode::FLAT) {
    renderFlat();
  }

  // Set render mode
  render_mode = mode;

  // Reset the render target to the default
  asw::display::setRenderTarget(nullptr);

  if (alpha < 1.0F) {
    SDL_SetTextureAlphaModFloat(image.get(), alpha);
  }
}

void TileType::renderCube(bool unique_top) {
  // Iterate over the faces
  for (int f = 0; f < 3; f++) {
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
    if (f == 0 && unique_top && images.size() > 1) {
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
  SDL_Vertex verts[4];

  for (int i = 0; i < 4; i++) {
    verts[i].position = project(FLAT_FACE[i]);

    // Colour
    verts[i].color.a = 1.0F;
    verts[i].color.r = FLAT_FACE_COLOUR.x;
    verts[i].color.g = FLAT_FACE_COLOUR.y;
    verts[i].color.b = FLAT_FACE_COLOUR.z;
  }

  verts[0].tex_coord = {0.0f, 0.0f};  // top-left
  verts[1].tex_coord = {1.0f, 0.0f};  // top-right
  verts[2].tex_coord = {1.0f, 1.0f};  // bottom-right
  verts[3].tex_coord = {0.0f, 1.0f};  // bottom-left

  SDL_RenderGeometry(asw::display::renderer, images.at(0).get(), verts, 4,
                     RENDER_ORDER, 6);
}