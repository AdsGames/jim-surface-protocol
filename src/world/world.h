#pragma once

#include <unordered_map>

#include "../tasks/worker.h"
#include "../tiles/tile_map.h"
#include "./resource_manager.h"
#include "./sound_orchestrator.h"

class World {
 public:
  /// Base
  void init();

  void update(float dt);

  void draw();

  /// Worker Utils
  Worker& getPlayer();

  /// Camera Utils
  asw::Quad<float>& getCamera() { return camera; }

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

  asw::Quad<float> camera{-640, -480, 1280, 960};

  float progression{0.0F};
};