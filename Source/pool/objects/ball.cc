#include "pool/objects/ball.h"

#include <Core/Managers/ResourcePath.h>

namespace pool {
Ball::Ball(std::string name, glm::vec3 center, float radius, glm::vec3 color)
    : Mesh(name) {
  {
    LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "sphere.obj");
    center_ = center;
    initial_center_ = center;
    color_ = color;
    radius_ = radius;
    model_matrix_ = glm::translate(model_matrix_, center);
    scale_ = glm::vec3(radius / kDefaultRadius);
    model_matrix_ =
        glm::scale(model_matrix_, scale_);
  }
}

Ball::~Ball(){};

void Ball::MoveUp(float delta_time) {
  center_.z -= delta_time * kDefaultSpeed;
  UpdateModelMatrix();
}

void Ball::MoveDown(float delta_time) {
  center_.z += delta_time * kDefaultSpeed;
  UpdateModelMatrix();
}

void Ball::MoveRight(float delta_time) {
  center_.x += delta_time * kDefaultSpeed;
  UpdateModelMatrix();
}

void Ball::MoveLeft(float delta_time) {
  center_.x -= delta_time * kDefaultSpeed;
  UpdateModelMatrix();
}

void Ball::UpdateModelMatrix() {
  model_matrix_ = glm::translate(glm::mat4(1), center_);
  model_matrix_ = glm::scale(model_matrix_, scale_);
}
}  // namespace pool