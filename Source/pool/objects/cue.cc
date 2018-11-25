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
    model_matrix_ = glm::scale(model_matrix_, scale_);
  }
}

Cue::~Cue(){};

void Cue::Rotate(float delta_x) {
  // TODO how to calculate the right value instead of hard coding?
  model_matrix_ =
      glm::rotate(model_matrix_, delta_x * 0.0035f, glm::vec3(0, 1, 0));
}
}  // namespace pool