#pragma once

#include <asw/asw.h>
#include <iostream>
#include <string>

#include "../lib/project.h"

using WorkerId = int;

enum class WorkerStatus { IDLE, WORKING, EN_ROUTE, RETURNING };

class Worker {
 public:
  Worker() : id(idCounter++) {}

  void setPosition(const asw::Vec3<int>& pos);

  WorkerId getId() const;

  int getTaskId() const;

  WorkerStatus getStatus() const;

  void draw(const asw::Vec2<float>& offset);

 private:
  static WorkerId idCounter;

  asw::Vec3<int> position;

  WorkerId id;
  int taskId;

  WorkerStatus status{WorkerStatus::IDLE};
};