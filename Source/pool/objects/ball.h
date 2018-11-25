#ifndef POOL_BALL_H_
#define POOL_BALL_H_

#include "Core/GPU/Mesh.h"

#pragma once

namespace pool {
class Ball : Mesh {
 public:
  Ball(std::string name, glm::vec3 center, float radius, glm::vec3 color);
  ~Ball();

  inline glm::mat4 GetModelMatrix() { return model_matrix_; }
  inline glm::vec3 GetCenter() { return center_; }
  inline glm::vec3 GetColor() { return color_; }
  inline float GetRadius() { return radius_; }

  void MoveUp(float delta_time);
  void MoveDown(float delta_time);
  void MoveRight(float delta_time);
  void MoveLeft(float delta_time);

 private:
  void UpdateModelMatrix();

  float kDefaultRadius = 0.5, kDefaultSpeed = 1.8;

  glm::mat4 model_matrix_ = glm::mat4(1);
  glm::vec3 color_;
  float radius_;
  glm::vec3 center_, initial_center_, scale_;
};
}  // namespace pool

#endif  // POOL_BALL_H_
