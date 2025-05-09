#include "tile_type.h"

#include <algorithm>

// Draw as a cube
const asw::Vec3<float> CUBE_VERTS[8] = {
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
const asw::Vec3<float> CUBE_FACES[3][4] = {
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
const asw::Vec3<float> FLAT_FACE[4] = {
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

void TileType::setRenderMode(TileRenderMode mode) {
  if (mode == TileRenderMode::CUBE_UNIQUE_TOP && images.size() != 2) {
    std::cerr << "Error: cube_unique_top requires 2 images" << '\n';
    return;
  }

  render_mode = mode;
}

// Its cubin' time
//   zz
// xzzzzy
// xxzzyy
// xxxyyy
// xxxyyy
//  xxyy
//   xy
void TileType::draw(const asw::Vec3<float>& position,
                    const asw::Vec2<float>& offset) {
  // Render image
  if (image == nullptr) {
    return;
  }

  // Calc screen position
  auto iso_x = isoX(position.x, position.y);
  auto iso_y = isoY(position.x, position.y, position.z);
  auto screen_pos = asw::Vec2<float>(iso_x, iso_y) * TILE_HEIGHT - offset;

  asw::draw::sprite(image, screen_pos);
}

void TileType::drawWireframe(const asw::Vec3<float>& position,
                             const asw::Vec2<float>& offset) {
  // Calc screen position
  auto iso_x = isoX(position.x, position.y);
  auto iso_y = isoY(position.x, position.y, position.z);
  auto screen_pos = asw::Vec2<float>(iso_x, iso_y) * TILE_HEIGHT - offset;

  // Draw of top of tile
  asw::draw::line(screen_pos + asw::Vec2<float>(0, TILE_HEIGHT / 2),
                  screen_pos + asw::Vec2<float>(TILE_WIDTH / 2, 0),
                  asw::util::makeColor(255, 255, 0));

  asw::draw::line(screen_pos + asw::Vec2<float>(TILE_WIDTH / 2, 0),
                  screen_pos + asw::Vec2<float>(TILE_WIDTH, TILE_HEIGHT / 2),
                  asw::util::makeColor(255, 0, 255));

  asw::draw::line(screen_pos + asw::Vec2<float>(TILE_WIDTH, TILE_HEIGHT / 2),
                  screen_pos + asw::Vec2<float>(TILE_WIDTH / 2, TILE_HEIGHT),
                  asw::util::makeColor(0, 255, 0));

  asw::draw::line(screen_pos + asw::Vec2<float>(TILE_WIDTH / 2, TILE_HEIGHT),
                  screen_pos + asw::Vec2<float>(0, TILE_HEIGHT / 2),
                  asw::util::makeColor(0, 255, 255));
}

void TileType::bakeTexture() {
  // Render image
  // 2x is because the texture is 2x the size of the world space of the tile
  image = asw::assets::createTexture(TILE_SIZE, TILE_SIZE);
  SDL_SetTextureBlendMode(image.get(), SDL_BLENDMODE_BLEND);
  SDL_SetTextureScaleMode(image.get(), SDL_SCALEMODE_NEAREST);

  // Set the render target to the image
  asw::display::setRenderTarget(image);

  // Render cube
  if (render_mode == TileRenderMode::CUBE ||
      render_mode == TileRenderMode::CUBE_UNIQUE_TOP) {
    renderCube();
  }

  // Render flat
  else if (render_mode == TileRenderMode::FLAT) {
    renderFlat();
  }

  // Reset the render target to the default
  asw::display::setRenderTarget(nullptr);
}

void TileType::renderCube() {
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
    if (render_mode == TileRenderMode::CUBE_UNIQUE_TOP && f == 0) {
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