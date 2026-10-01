#pragma once

#include <asw/asw.h>
#include <string>
#include <vector>

class TileMap;
class World;

// Particles that show what happens in the world: drilling, purifying,
// building, toxic water and the worker's dust
class WorldEffects {
 public:
  void init(TileMap& map);

  void update(float dt, World& world);

  // Particles lit by the scene, e.g. dust. Draw before the light map
  void drawLit(const asw::Camera& camera);

  // Particles that give off light, e.g. sparkles. Draw after the light map
  void drawGlow(const asw::Camera& camera);

  // Text popups. Draw last, so they read at night too
  void drawText(asw::Camera& camera);

  // Events, with the tile they happen on. Call before the tile changes, so
  // the effect sits on the ground the tile showed
  void drilling(const asw::Vec3<int>& tile, float dt);
  void tileBroken(const asw::Vec3<int>& tile, asw::Camera& camera);
  void tilePurified(const asw::Vec3<int>& tile);
  void built(const asw::Vec3<int>& tile);

  // Text that rises off a tile and fades, e.g. "+1 scrap"
  void popup(const asw::Vec3<int>& tile,
             const std::string& text,
             asw::Color color);

 private:
  // Find the toxic water tops that bubble
  void scanTiles(World& world);

  // Emit from random tile tops in view, count may be a fraction
  void emitOver(asw::ParticleEmitter& emitter,
                const std::vector<asw::Vec2<float>>& tops,
                const asw::Quad<float>& view,
                float& pending,
                float count,
                float lift);

  // Middle of the ground a tile shows, in world space
  asw::Vec2<float> groundOf(const asw::Vec3<int>& tile) const;

  static void emitAt(asw::ParticleEmitter& emitter,
                     const asw::Vec2<float>& position,
                     uint32_t count);

  // Lit by the scene
  asw::ParticleEmitter debris;
  asw::ParticleEmitter dust;
  asw::ParticleEmitter bubbles;
  asw::ParticleEmitter fog;

  // Give off light
  asw::ParticleEmitter sparkles;

  struct Popup {
    asw::Vec2<float> position;  // World space
    std::string text;
    asw::Color color;
    float age{0.0F};
  };
  std::vector<Popup> popups;
  asw::Font popup_font;

  TileMap* tile_map{nullptr};

  // Tile tops in world space, refreshed now and then
  std::vector<asw::Vec2<float>> toxic_tops;
  float scan_timer{0.0F};

  // Fractions of a particle owed, so slow rates still emit
  float debris_pending{0.0F};
  float bubbles_pending{0.0F};
  float fog_pending{0.0F};
  float dust_pending{0.0F};
};
