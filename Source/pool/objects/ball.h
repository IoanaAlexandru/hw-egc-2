#ifndef POOL_BALL_H_
#define POOL_BALL_H_

#include "Core/GPU/Mesh.h"

#pragma once

namespace pool {
class Ball : Mesh {
 public:
  Ball(std::string name, glm::vec3 center, float radius, glm::vec3 color);
  ~Ball();

  inline glm::mat4 getModelMatrix() { return model_matrix_; }
  inline glm::vec3 getCenter() { return center_; }
  inline glm::vec3 getColor() { return color_; }
  inline float getradius() { return radius_; }

  void moveUp(float delta_time);
  void moveDown(float delta_time);
  void moveRight(float delta_time);
  void moveLeft(float delta_time);

 private:
  void updateModelMatrix();

  float kDefaultRadius = 0.5, kDefaultSpeed = 1.8;

  glm::mat4 model_matrix_ = glm::mat4(1);
  glm::vec3 color_;
  float radius_;
  glm::vec3 center_, initial_center_, scale_;
};
}  // namespace pool

#endif  // POOL_BALL_H_
