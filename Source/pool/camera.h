#ifndef POOL_CAMERA_H_
#define POOL_CAMERA_H_

#include <include/glm.h>
#include <include/math.h>
#include <iostream>

namespace pool {
enum class CameraType { TopDown, FirstPerson, ThirdPerson };

class Camera {
 public:
  Camera(float aspect_ratio);
  ~Camera();

  void TopDown();
  void FirstPerson();
  void ThirdPerson(glm::vec3 ball_pos, glm::vec3 target_pos);

  void TranslateForward(float distance);
  void TranslateUpword(float distance);
  void TranslateRight(float distance);

  void RotateOx(float angle);
  void RotateOy(float angle);
  void RotateOz(float angle);

  glm::mat4 GetViewMatrix();
  glm::mat4 GetProjectionMatrix();

  glm::vec3 GetTargetPosition();
  void SetTargetPosition(glm::vec3 target_pos);

  // Get angle of rotation on Ox between the target and the camera
  float GetOxAngle();

 private:
  void ResetDefaults();

  void RotateFirstPersonOx(float angle);
  void RotateFirstPersonOy(float angle);
  void RotateFirstPersonOz(float angle);

  void RotateThirdPersonOx(float angle);
  void RotateThirdPersonOy(float angle);
  void RotateThirdPersonOz(float angle);

  glm::vec3 GetViewPoint(glm::vec3 target_pos, glm::vec3 ball_pos);

  CameraType type_;
  float distance_to_target_, aspect_ratio_, view_height_;
  glm::vec3 position_;
  glm::vec3 forward_;
  glm::vec3 right_;
  glm::vec3 up_;

  glm::mat4 projection_matrix_ = glm::mat4(1);
};
}  // namespace pool

#endif  // POOL_CAMERA_H_