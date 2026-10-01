#pragma once

#include <nlohmann/json.hpp>
#include <unordered_map>

#include "../tasks/worker.h"
#include "../tiles/tile_map.h"
#include "./resource_manager.h"
#include "./sound_orchestrator.h"
#include "./world_effects.h"
#include "./world_lighting.h"

class World {
 public:
  /// Base
  void init();

  // input: false while the player is busy elsewhere, e.g. in the toolbar
  void update(float dt, bool input = true);

  void draw();

  /// Save Utils
  void save(nlohmann::json& data) const;
  void load(const nlohmann::json& data);

  /// Worker Utils
  Worker& getPlayer();

  /// Camera Utils
  // Put the worker in the middle of the view above the toolbar
  void centerCameraOnPlayer();
  asw::Camera& getCamera() { return camera; }

  /// Tile Utils
  TileMap& getTileMap() { return tile_map; }

  asw::Vec3<int> getPlayerWaypoint() { return playerWaypoint; }
  // Send the worker to a tile, or next to it if it is blocked. False if
  // there is no route, and the worker keeps its old one
  bool setPlayerWaypoint(const asw::Vec3<int>& waypoint);

  /// Effect Utils
  WorldEffects& getEffects() { return effects; }
  SoundOrchestrator& getSounds() { return sound_orchestrator; }

  // Resource Utils
  ResourceManager& getResourceManager() { return resource_manager; }

  bool getWaypointActive() const { return waypointActive; }
  void setWaypointActive(bool active) {
    if (active && !waypointActive) {
      waypoint_age = 0.0F;
    }
    waypointActive = active;
  }

  // Purity Progression
  float getProgression() const { return progression; }
  void updateProgression();

 private:
  asw::Vec3<int> playerWaypoint{0, 0, 0};

  asw::Texture waypointTexture;
  asw::Texture shadowTexture;
  bool waypointActive{false};

  Worker player;

  TileMap tile_map;

  ResourceManager resource_manager;

  SoundOrchestrator sound_orchestrator;

  asw::Camera camera;

  // After the camera, so it is destroyed first
  WorldLighting lighting;

  // Last pan direction, and how far the pan is from stopped (0) to full (1)
  asw::Vec2<float> pan_direction{0.0F, 0.0F};
  float pan_ramp{0.0F};

  float progression{0.0F};

  WorldEffects effects;

  // Seconds since the waypoint marker was placed, for its drop in
  float waypoint_age{0.0F};

  // Sky behind the map
  void drawSky(const asw::Quad<float>& view);

  // Move the worker to the nearest land if its column can not be driven on,
  // e.g. a tree grew there or an old save left it stuck
  void rescuePlayer();

  // Dots on the ground from the worker to the waypoint
  void drawRoute();

  struct Star {
    asw::Vec2<float> position;  // 0 to 1 across the screen
    float size;
    float phase;
  };
  std::vector<Star> stars;
};