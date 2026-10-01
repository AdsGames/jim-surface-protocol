#pragma once

#include <asw/asw.h>

/// @brief Calculate the X coordinate of a point in isometric projection.
/// @param v 3D vector.
/// @return X coordinate of the point in isometric projection.
inline float isoX(const asw::Vec3<int>& v) {
  return v.x - v.y;
}

/// @brief Calculate the Y coordinate of a point in isometric projection.
/// @param v 3D vector.
/// @return Y coordinate of the point in isometric projection.
inline float isoY(const asw::Vec3<int>& v) {
  return ((v.x + v.y) / 2.0F) - v.z;
}

/// @brief Calculate the X coordinate of a point in isometric projection.
/// @param v 3D vector.
/// @return X coordinate of the point in isometric projection.
inline float isoXf(const asw::Vec3<float>& v) {
  return v.x - v.y;
}

/// @brief Calculate the Y coordinate of a point in isometric projection.
/// @param v 3D vector.
/// @return Y coordinate of the point in isometric projection.
inline float isoYf(const asw::Vec3<float>& v) {
  return ((v.x + v.y) / 2.0F) - v.z;
}

/// @brief Get the top face of a tile in world space.
/// @param v Tile index.
/// @param tile_height Height of the top face in pixels. Its width is double.
/// @return The four corners of the face, clockwise from the left.
inline asw::Polygonf isoDiamond(const asw::Vec3<int>& v, float tile_height) {
  const auto origin = asw::Vec2(isoX(v), isoY(v)) * tile_height;
  return {
      origin + asw::Vec2(0.0F, tile_height / 2),
      origin + asw::Vec2(tile_height, 0.0F),
      origin + asw::Vec2(tile_height * 2, tile_height / 2),
      origin + asw::Vec2(tile_height, tile_height),
  };
}
