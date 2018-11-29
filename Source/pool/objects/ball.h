#ifndef POOL_BALL_H_
#define POOL_BALL_H_

#include "Core/GPU/Mesh.h"

namespace pool {
class Ball : public Mesh {
 public:
  Ball(std::string name, glm::vec3 center, float radius, glm::vec3 color);
  ~Ball();

  void Update(float delta_time);

  inline glm::mat4 GetModelMatrix() { return model_matrix_; }
  inline glm::vec3 GetCenter() { return center_; }
  inline glm::vec3 GetColor() { return color_; }
  inline float GetRadius() { return radius_; }

  inline bool IsMoving() { return movement_vector_ != glm::vec3(0, 0, 0); }

  void MoveUp(float delta_time);
  void MoveDown(float delta_time);
  void MoveRight(float delta_time);
  void MoveLeft(float delta_time);

  void CueHit(glm::vec3 direction, float distance);

  void ReflectX();
  void ReflectZ();

  static bool AreTouching(Ball* ball1, Ball* ball2);

 private:
  void UpdateModelMatrix();

  float kDefaultRadius = 0.5f, kDefaultSpeed = 1.8f;

  glm::mat4 model_matrix_ = glm::mat4(1);
  glm::vec3 color_;
  float radius_;
  glm::vec3 center_, initial_center_, scale_;
  glm::vec3 movement_vector_;
};
}  // namespace pool

#endif  // POOL_BALL_H_
