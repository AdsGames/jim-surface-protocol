#include "worker.h"

WorkerId Worker::idCounter = 0;

void Worker::setPosition(const asw::Vec3<int>& pos) {
  position = pos;
}

WorkerId Worker::getId() const {
  return id;
}

int Worker::getTaskId() const {
  return taskId;
}

WorkerStatus Worker::getStatus() const {
  return status;
}

void Worker::draw(const asw::Vec2<float>& offset) {
  // Calc screen position
  auto iso_x = isoX(position.x, position.y);
  auto iso_y = isoY(position.x, position.y, position.z);
  auto screen_pos = asw::Vec2<float>(iso_x, iso_y) * 32 - offset;

  // Draw of top of tile
  asw::draw::line(screen_pos + asw::Vec2<float>(0, 32 / 2),
                  screen_pos + asw::Vec2<float>(32 / 2, 0),
                  asw::util::makeColor(255, 255, 0));

  asw::draw::line(screen_pos + asw::Vec2<float>(32 / 2, 0),
                  screen_pos + asw::Vec2<float>(32, 32 / 2),
                  asw::util::makeColor(255, 0, 255));

  asw::draw::line(screen_pos + asw::Vec2<float>(32, 32 / 2),
                  screen_pos + asw::Vec2<float>(32 / 2, 32),
                  asw::util::makeColor(0, 255, 0));

  asw::draw::line(screen_pos + asw::Vec2<float>(32 / 2, 32),
                  screen_pos + asw::Vec2<float>(0, 32 / 2),
                  asw::util::makeColor(0, 255, 255));
}
