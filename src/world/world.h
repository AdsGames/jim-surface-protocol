#pragma once

#include <nlohmann/json.hpp>
#include <unordered_map>

#include "../tasks/worker.h"
#include "../tiles/tile_map.h"
#include "./resource_manager.h"
#include "./sound_orchestrator.h"
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
  void setPlayerWaypoint(const asw::Vec3<int>& waypoint) {
    playerWaypoint = waypoint;
  }

  // Resource Utils
  ResourceManager& getResourceManager() { return resource_manager; }

  bool getWaypointActive() const { return waypointActive; }
  void setWaypointActive(bool active) { waypointActive = active; }

  // Purity Progression
  float getProgression() const { return progression; }

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
};