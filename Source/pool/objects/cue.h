#ifndef POOL_CUE_H_
#define POOL_CUE_H_

#include "Core/GPU/Mesh.h"

#pragma once

namespace pool {
class Cue : Mesh {
 public:
  Cue(std::string name, glm::vec3 tip, float length, glm::vec3 color);
  ~Cue();

  inline glm::mat4 GetModelMatrix() { return model_matrix_; }
  inline glm::vec3 GetTip() { return tip_; }
  inline glm::vec3 GetColor() { return color_; }
  inline float GetLength() { return length_; }

  // void MoveUp(float delta_time);
  // void MoveDown(float delta_time);
  // void MoveRight(float delta_time);
  // void MoveLeft(float delta_time);

 private:
  void UpdateModelMatrix();

  float kDefaultLength = 18.5, kDefaultSpeed = 1.8;

  glm::mat4 model_matrix_ = glm::mat4(1);
  glm::vec3 color_;
  float length_;
  glm::vec3 tip_, initial_tip_, scale_;
};
}  // namespace pool

#endif  // POOL_CUE_H_
