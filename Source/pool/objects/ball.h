#ifndef POOL_BALL_H_
#define POOL_BALL_H_

#include "Core/GPU/Mesh.h"

#pragma once

namespace pool {
class Ball : Mesh {
 public:
  Ball(std::string name, glm::vec3 center, float radius, glm::vec3 color);
  ~Ball();

  float kDefaultRadius = 0.5;

  glm::mat4 model_matrix_ = glm::mat4(1);
  glm::vec3 color_;
  float radius_;
  glm::vec3 center_, initial_center_;
};
}  // namespace pool

#endif  // POOL_BALL_H_
