#include "pool/objects/ball.h"

#include <Core/Managers/ResourcePath.h>

namespace pool {
Ball::Ball(std::string name, glm::vec3 center, float radius, glm::vec3 color)
    : Mesh(name) {
  {
    LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "sphere.obj");
    center_ = initial_center_ = center;
    color_ = color;
    radius_ = radius;
    model_matrix_ = glm::translate(model_matrix_, center);
    model_matrix_ =
        glm::scale(model_matrix_, glm::vec3(radius / kDefaultRadius));
  }
}

Ball::~Ball(){};
}  // namespace pool