#include "pool/objects/cue.h"

#include <Core/Managers/ResourcePath.h>

namespace pool {
Cue::Cue(std::string name, glm::vec3 tip, float length, glm::vec3 color)
    : Mesh(name) {
  {
    LoadMesh(RESOURCE_PATH::MODELS + "Props", "pool_cue.obj");
    tip_ = initial_tip_ = tip;
    color_ = color;
    length = length;
    model_matrix_ = glm::translate(model_matrix_, tip);
    scale_ = glm::vec3(length / kDefaultLength);
    model_matrix_ =
        glm::scale(model_matrix_, scale_);
  }
}

Cue::~Cue(){};

// void Ball::MoveUp(float delta_time) {
//  center_.z -= delta_time * kDefaultSpeed;
//  UpdateModelMatrix();
//}
//
// void Ball::MoveDown(float delta_time) {
//  center_.z += delta_time * kDefaultSpeed;
//  UpdateModelMatrix();
//}
//
// void Ball::MoveRight(float delta_time) {
//  center_.x += delta_time * kDefaultSpeed;
//  UpdateModelMatrix();
//}
//
// void Ball::MoveLeft(float delta_time) {
//  center_.x -= delta_time * kDefaultSpeed;
//  UpdateModelMatrix();
//}

void Cue::UpdateModelMatrix() {
  model_matrix_ = glm::translate(glm::mat4(1), tip_);
  model_matrix_ = glm::scale(model_matrix_, scale_);
}
}  // namespace pool