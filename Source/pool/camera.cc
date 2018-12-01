#include "camera.h"

namespace pool {
Camera::Camera(float aspect_ratio) {
  aspect_ratio_ = aspect_ratio;
  ResetDefaults();
}

Camera::~Camera() {}

void Camera::TopDown() { ResetDefaults(); }

void Camera::FirstPerson(glm::vec3 target_pos) {
  type_ = CameraType::FirstPerson;
  // TODO
}

void Camera::ThirdPerson(
    glm::vec3 ball_pos,
    glm::vec3 target_pos) {  // TODO change to target_pos, direction
  type_ = CameraType::ThirdPerson;
  position_ = GetViewPoint(target_pos, ball_pos);
  position_.y = view_height_;
  forward_ = (target_pos - position_) / distance_to_target_;
  up_ = glm::vec3(0, 1, 0);
  right_ = glm::cross(forward_, up_) / glm::dot(up_, up_);
}

void Camera::TranslateForward(float distance) {
  position_ = position_ + glm::normalize(forward_) * distance;
}

void Camera::TranslateUpword(float distance) {
  position_ = position_ + glm::normalize(up_) * distance;
}

void Camera::TranslateRight(float distance) {
  position_ = position_ + glm::normalize(right_) * distance;
}

void Camera::RotateOx(float angle) {
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

void Camera::RotateOy(float angle) {
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

void Camera::RotateOz(float angle) {
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

glm::mat4 Camera::GetViewMatrix() {
  return glm::lookAt(position_, position_ + forward_, up_);
}

glm::mat4 Camera::GetProjectionMatrix() { return projection_matrix_; }

glm::vec3 Camera::GetTargetPosition() {
  return position_ + forward_ * distance_to_target_;
}

void Camera::SetTargetPosition(glm::vec3 target_pos) {
  forward_ = (target_pos - position_) / distance_to_target_;
}

float Camera::GetOxAngle() {
  glm::vec3 target_pos = GetTargetPosition();
  glm::vec2 v1 = glm::vec2(target_pos.x, target_pos.z);
  glm::vec2 v2 = glm::vec2(position_.x, position_.z);

  glm::vec2 vec1 = glm::normalize(glm::vec2(0, 1));
  glm::vec2 vec2 = glm::normalize(v1 - v2);
  return acos(glm::dot(vec1, vec2));
}

void Camera::ResetDefaults() {
  type_ = CameraType::TopDown;
  position_ = glm::vec3(0, 4.25, 0);
  forward_ = glm::vec3(0, -1, 0);
  up_ = glm::vec3(0, 0, -1);
  right_ = glm::vec3(-1, 0, 0);
  distance_to_target_ = 0.8f;
  view_height_ = 0.3f;
  projection_matrix_ =
      glm::perspective(RADIANS(60), aspect_ratio_, 0.01f, 200.0f);
}

void Camera::RotateFirstPersonOx(float angle) {
  forward_ = glm::normalize(
      glm::vec3(glm::rotate(glm::mat4(1), angle, glm::vec3(1, 0, 0)) *
                glm::vec4(forward_, 1)));

  up_ = glm::cross(right_, forward_);
}

void Camera::RotateFirstPersonOy(float angle) {
  forward_ = glm::normalize(
      glm::vec3(glm::rotate(glm::mat4(1), angle, glm::vec3(0, 1, 0)) *
                glm::vec4(forward_, 1)));

  right_ = glm::normalize(
      glm::vec3(glm::rotate(glm::mat4(1), angle, glm::vec3(0, 1, 0)) *
                glm::vec4(right_, 1)));

  up_ = glm::cross(right_, forward_);
}

void Camera::RotateFirstPersonOz(float angle) {
  right_ = glm::normalize(
      glm::vec3(glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 0, 1)) *
                glm::vec4(right_, 1)));

  up_ = glm::cross(right_, forward_);
}

void Camera::RotateThirdPersonOx(float angle) {
  TranslateForward(distance_to_target_);
  RotateFirstPersonOx(angle);
  TranslateForward(-distance_to_target_);
}

void Camera::RotateThirdPersonOy(float angle) {
  TranslateForward(distance_to_target_);
  RotateFirstPersonOy(angle);
  TranslateForward(-distance_to_target_);
}

void Camera::RotateThirdPersonOz(float angle) {
  TranslateForward(distance_to_target_);
  RotateFirstPersonOz(angle);
  TranslateForward(-distance_to_target_);
}

glm::vec3 Camera::GetViewPoint(glm::vec3 target_pos, glm::vec3 ball_pos) {
  glm::vec3 v = glm::normalize(target_pos - ball_pos);
  return ball_pos - distance_to_target_ * v;
}
}  // namespace pool