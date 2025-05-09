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
  auto iso_x = isoX(position);
  auto iso_y = isoY(position);

  auto screen_pos = asw::Vec2(iso_x, iso_y) * 32 - offset;

  // Draw of top of tile
  asw::draw::line(screen_pos + asw::Vec2(0.0F, 32.0F / 2.0F),
                  screen_pos + asw::Vec2(32.0F / 2.0F, 0.0F),
                  asw::util::makeColor(255, 255, 0));

  asw::draw::line(screen_pos + asw::Vec2(32.0F / 2.0F, 0.0F),
                  screen_pos + asw::Vec2(32.0F, 32.0F / 2.0F),
                  asw::util::makeColor(255, 0, 255));

  asw::draw::line(screen_pos + asw::Vec2(32.0F, 32.0F / 2.0F),
                  screen_pos + asw::Vec2(32.0F / 2.0F, 32.0F),
                  asw::util::makeColor(0, 255, 0));

  asw::draw::line(screen_pos + asw::Vec2(32.0F / 2.0F, 32.0F),
                  screen_pos + asw::Vec2(0.0F, 32.0F / 2.0F),
                  asw::util::makeColor(0, 255, 255));
}
