#ifndef POOL_OBJECTS_TABLE_H_
#define POOL_OBJECTS_TABLE_H_

#include "Core/GPU/Mesh.h"

#pragma once

namespace pool {
class Table : public Mesh {
 public:
  // Create table with given center, size and color
  Table(std::string name, glm::vec3 center, float width, float length,
        float height, float thickness, glm::vec3 slate_color,
        glm::vec3 rail_color);
  ~Table();
  inline float GetHeight() { return height_; }
  inline float GetWidth() { return width_; }
  inline float GetLength() { return length_; }
  inline float GetThickness() { return thickness_; }

 protected:
  float height_, width_, length_, thickness_;
  glm::vec3 slate_color_, rail_color_;
};
}  // namespace pool

#endif  // POOL_OBJECTS_TABLE_H_
