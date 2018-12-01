#ifndef POOL_CAMERA_H_
#define POOL_CAMERA_H_

#include <include/glm.h>
#include <include/math.h>
#include <iostream>

namespace pool {
enum class CameraType { TopDown, FirstPerson, ThirdPerson };

class Camera {
 public:
  Camera(float aspect_ratio) {
    aspect_ratio_ = aspect_ratio;
    ResetDefaults();
  }

  Camera(const glm::vec3 &position, const glm::vec3 &center,
         const glm::vec3 &up) {
    Set(position, center, up);
  }

  ~Camera() {}

  void TopDown() { ResetDefaults(); }

  void FirstPerson(glm::vec3 target_pos) {
    type_ = CameraType::FirstPerson;
    // TODO
  }

  void ThirdPerson(
      glm::vec3 ball_pos,
      glm::vec3 target_pos) {  // TODO change to target_pos, direction
    type_ = CameraType::ThirdPerson;
    position_ = GetViewPoint(target_pos, ball_pos);
    position_.y = view_height_;
    forward_ = (target_pos - position_) / distance_to_target_;
    up_ = glm::vec3(0, 1, 0);
    right_ = glm::cross(forward_, up_) / glm::dot(up_, up_);
  }

  // Update camera
  void Set(const glm::vec3 &position, const glm::vec3 &center,
           const glm::vec3 &up) {
    position_ = position;
    forward_ = glm::normalize(center - position);
    right_ = glm::cross(forward_, up);
    up_ = glm::cross(right_, forward_);
  }

  void TranslateForward(float distance) {
    position_ = position_ + glm::normalize(forward_) * distance;
  }

  void TranslateUpword(float distance) {
    position_ = position_ + glm::normalize(up_) * distance;
  }

  void TranslateRight(float distance) {
    position_ = position_ + glm::normalize(right_) * distance;
  }

  void RotateOx(float angle) {
    switch (type_) {
      case CameraType::FirstPerson:
        RotateFirstPersonOx(angle);
        break;
      case CameraType::ThirdPerson:
        RotateThirdPersonOx(angle);
        break;
      default:
        break;
    }
  }

  void RotateOy(float angle) {
    switch (type_) {
      case CameraType::FirstPerson:
        RotateFirstPersonOy(angle);
        break;
      case CameraType::ThirdPerson:
        RotateThirdPersonOy(angle);
        break;
      default:
        break;
    }
  }

  void RotateOz(float angle) {
    switch (type_) {
      case CameraType::FirstPerson:
        RotateFirstPersonOz(angle);
        break;
      case CameraType::ThirdPerson:
        RotateThirdPersonOz(angle);
        break;
      default:
        break;
    }
  }

  glm::mat4 GetViewMatrix() {
    return glm::lookAt(position_, position_ + forward_, up_);
  }

  glm::mat4 GetProjectionMatrix() { return projectionMatrix; }

  glm::vec3 GetTargetPosition() {
    return position_ + forward_ * distance_to_target_;
  }

  void SetTargetPosition(glm::vec3 target_pos) {
    forward_ = (target_pos - position_) / distance_to_target_;
  }

  float GetOxAngle() {
    glm::vec3 target_pos = GetTargetPosition();
    glm::vec2 v1 = glm::vec2(target_pos.x, target_pos.z);
    glm::vec2 v2 = glm::vec2(position_.x, position_.z);

    glm::vec2 vec1 = glm::normalize(glm::vec2(0, 1));
    glm::vec2 vec2 = glm::normalize(v1 - v2);
    return acos(glm::dot(vec1, vec2));
  }

 private:
  void ResetDefaults() {
    type_ = CameraType::TopDown;
    position_ = glm::vec3(0, 4.25, 0);
    forward_ = glm::vec3(0, -1, 0);
    up_ = glm::vec3(0, 0, -1);
    right_ = glm::vec3(-1, 0, 0);
    distance_to_target_ = 0.8f;
    view_height_ = 0.3f;
    projectionMatrix =
        glm::perspective(RADIANS(60), aspect_ratio_, 0.01f, 200.0f);
  }

  void RotateFirstPersonOx(float angle) {
    forward_ = glm::normalize(
        glm::vec3(glm::rotate(glm::mat4(1), angle, glm::vec3(1, 0, 0)) *
                  glm::vec4(forward_, 1)));

    up_ = glm::cross(right_, forward_);
  }

  void RotateFirstPersonOy(float angle) {
    forward_ = glm::normalize(
        glm::vec3(glm::rotate(glm::mat4(1), angle, glm::vec3(0, 1, 0)) *
                  glm::vec4(forward_, 1)));

    right_ = glm::normalize(
        glm::vec3(glm::rotate(glm::mat4(1), angle, glm::vec3(0, 1, 0)) *
                  glm::vec4(right_, 1)));

    up_ = glm::cross(right_, forward_);
  }

  void RotateFirstPersonOz(float angle) {
    right_ = glm::normalize(
        glm::vec3(glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 0, 1)) *
                  glm::vec4(right_, 1)));

    up_ = glm::cross(right_, forward_);
  }

  void RotateThirdPersonOx(float angle) {
    TranslateForward(distance_to_target_);
    RotateFirstPersonOx(angle);
    TranslateForward(-distance_to_target_);
  }

  void RotateThirdPersonOy(float angle) {
    TranslateForward(distance_to_target_);
    RotateFirstPersonOy(angle);
    TranslateForward(-distance_to_target_);
  }

  void RotateThirdPersonOz(float angle) {
    TranslateForward(distance_to_target_);
    RotateFirstPersonOz(angle);
    TranslateForward(-distance_to_target_);
  }

  glm::vec3 GetViewPoint(glm::vec3 target_pos, glm::vec3 ball_pos) {
    glm::vec3 v = glm::normalize(target_pos - ball_pos);
    return ball_pos - distance_to_target_ * v;
  }

  CameraType type_;
  float distance_to_target_, aspect_ratio_, view_height_;
  glm::vec3 position_;
  glm::vec3 forward_;
  glm::vec3 right_;
  glm::vec3 up_;

  glm::mat4 projectionMatrix = glm::mat4(1);
  float left, right, bottom, top, zNear, zFar, aspect, fov;
};
}  // namespace pool

#endif  // POOL_CAMERA_H_