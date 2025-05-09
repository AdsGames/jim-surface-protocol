#pragma once

/// @brief Calculate the X coordinate of a point in isometric projection.
/// @param x X coordinate of the point.
/// @param y Y coordinate of the point.
/// @return X coordinate of the point in isometric projection.
inline float isoX(float x, float y) {
  return x - y;
}

/// @brief Calculate the Y coordinate of a point in isometric projection.
/// @param x X coordinate of the point.
/// @param y Y coordinate of the point.
/// @param z Z coordinate of the point.
/// @return Y coordinate of the point in isometric projection.
inline float isoY(float x, float y, float z) {
  return ((x + y) / 2) - z;
}