#pragma once

#include <asw/asw.h>

// Input actions shared by keyboard, mouse and controllers
namespace controls {

// Camera pan: keys, D-pad and right stick
inline constexpr const char* CAMERA_UP = "camera_up";
inline constexpr const char* CAMERA_DOWN = "camera_down";
inline constexpr const char* CAMERA_LEFT = "camera_left";
inline constexpr const char* CAMERA_RIGHT = "camera_right";

// Tool select, keyboard
inline constexpr const char* TOOL_DRILL = "tool_drill";
inline constexpr const char* TOOL_PURIFIER = "tool_purifier";
inline constexpr const char* TOOL_TREE = "tool_tree";

// Choose the previous or next tool. LB and RB
inline constexpr const char* TOOL_PREV = "tool_prev";
inline constexpr const char* TOOL_NEXT = "tool_next";

// Use the chosen tool on the tile under the pointer. Left click
inline constexpr const char* USE = "use";

// Drill, whatever tool is chosen. Right trigger
inline constexpr const char* DRILL = "drill";

// Use the chosen tool, like left click. Left trigger
inline constexpr const char* USE_TOOL = "use_tool";

// Move the worker to the tile under the pointer. Right click and A
inline constexpr const char* WAYPOINT = "waypoint";

// Jump the camera to the worker. Space and X, like centring on a unit in
// an RTS
inline constexpr const char* CENTER_CAMERA = "center_camera";

// Move controller focus into the toolbar, for upgrades and tool buttons
inline constexpr const char* TOOLBAR = "toolbar";

// Screens
inline constexpr const char* BACK = "back";
inline constexpr const char* PAUSE = "pause";
inline constexpr const char* FULLSCREEN = "fullscreen";

// Bind every action, call once after asw::core::init
void bind();

// Check if a key or controller button that skips a screen was pressed
bool any_skip();

// Rumble every connected controller, strength from 0 to 1
void rumble(float low, float high, int duration_ms);

// Rumble the trigger motors, on controllers that have them
void rumble_triggers(float left, float right, int duration_ms);

// True while a button that acts on the world is held: click, trigger or A
bool any_world_action_held();

// Move the controller cursor with the left stick. Call once per update
void update_pointer(float dt);

// Where the player points: the mouse, or the controller cursor after the
// left stick moves it. The mouse takes over again when it moves
asw::Vec2<float> pointer();

// True while the controller cursor is in use, so it should be drawn
bool is_controller_pointer();

// Draw the controller cursor, if it is in use
void draw_pointer();

// UI navigation for menus: arrows, D-pad, left stick, A and B. Back runs
// the given action, e.g. BACK to leave the menu
asw::ui::Navigation menu_navigation(const char* back);

// UI navigation that does nothing, so play input does not move UI focus
asw::ui::Navigation off_navigation();

}  // namespace controls
