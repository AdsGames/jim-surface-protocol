#pragma once

// Input actions shared by keyboard and controllers
namespace controls {

// Camera pan, keyboard and any controller
inline constexpr const char* CAMERA_UP = "camera_up";
inline constexpr const char* CAMERA_DOWN = "camera_down";
inline constexpr const char* CAMERA_LEFT = "camera_left";
inline constexpr const char* CAMERA_RIGHT = "camera_right";

// Tool select
inline constexpr const char* TOOL_DRILL = "tool_drill";
inline constexpr const char* TOOL_PURIFIER = "tool_purifier";
inline constexpr const char* TOOL_TREE = "tool_tree";

// Screens
inline constexpr const char* BACK = "back";
inline constexpr const char* FULLSCREEN = "fullscreen";

// Bind every action, call once after asw::core::init
void bind();

// Check if a key or controller button that skips a screen was pressed
bool any_skip();

}  // namespace controls
