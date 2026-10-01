#pragma once

#include <asw/asw.h>
#include <memory>
#include <vector>

class World;

// Day and night, glowing tiles and the lights on the worker
class WorldLighting {
 public:
  WorldLighting();

  void init(const asw::Camera& camera, float day_length);

  void update(float dt, World& world);

  void draw();

  // Time of day, in seconds since the first morning
  float getTime() const { return time; }
  void setTime(float new_time) { time = new_time; }

 private:
  // Spread light from glowing tiles across the map
  void computeGlow(World& world);

  // Draw the spread light as tile tops into the glow texture
  void renderGlow();

  asw::lighting::LightMap light_map;

  asw::lighting::AmbientCycle toxic_cycle;
  asw::lighting::AmbientCycle clean_cycle;

  std::unique_ptr<asw::lighting::TileLight> tile_light;

  // Height of the top tile in each column, x major
  std::vector<int> top_z;

  asw::Texture glow;
  asw::Quad<float> glow_bounds;

  float time{0.0F};
  float glow_timer{0.0F};
};
