#include "pool/objects/cue.h"

#include <Core/Managers/ResourcePath.h>
#include <iostream>

namespace pool {
Cue::Cue(std::string name, glm::vec3 tip, float length, glm::vec3 color)
    : Mesh(name) {
  {
    LoadMesh(RESOURCE_PATH::MODELS + "Props", "pool_cue.obj");
    tip_ = tip;
    color_ = color;
    length = length;
    direction_ = glm::vec3(0, 0, 1);
    model_matrix_ = glm::translate(model_matrix_, tip);
    scale_ = glm::vec3(length / kDefaultLength);
    model_matrix_ = glm::scale(model_matrix_, scale_);
  }
}

Cue::~Cue(){};

void Cue::Reposition(glm::vec3 tip) {
  tip_ = tip;
  direction_ = glm::vec3(0, 0, 1);
  model_matrix_ = glm::translate(glm::mat4(1), tip_);
  model_matrix_ = glm::scale(model_matrix_, scale_);
}

void Cue::Rotate(float angle) {
  model_matrix_ = glm::rotate(model_matrix_, angle, glm::vec3(0, 1, 0));
  float x = direction_.x, z = direction_.z;
  direction_.x = x * cos(-angle) - z * sin(-angle);
  direction_.z = x * sin(-angle) + z * cos(-angle);
}
}  // namespace pool