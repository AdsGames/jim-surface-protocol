#pragma once

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