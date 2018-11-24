#include "pool/objects/table.h"
#include "Core/Engine.h"

namespace pool {
Table::Table(std::string name, glm::vec3 center, float width, float length,
             float height, float thickness, glm::vec3 slate_color, glm::vec3 rail_color)
    : Mesh(name) {
  height_ = height;
  width_ = width;
  slate_color_ = slate_color;
  rail_color_ = rail_color;

  glm::vec3 corner = center + glm::vec3(-width / 2, 0, length / 2);
  glm::vec3 inside_corner = corner + glm::vec3(thickness, 0, -thickness);
  float inside_width = width - 2 * thickness,
        inside_length = length - 2 * thickness;
  glm::vec3 inside_rail_color = rail_color - glm::vec3(0.1, 0.1, 0.1);
  glm::vec3 bottom_inside_rail_color = rail_color - glm::vec3(0.2, 0.2, 0.2);

  std::vector<VertexFormat> vertices = {
      VertexFormat(corner, rail_color),
      VertexFormat(corner + glm::vec3(width, 0, 0), rail_color),
      VertexFormat(corner + glm::vec3(width, 0, -length), rail_color),
      VertexFormat(corner + glm::vec3(0, 0, -length), rail_color),
      VertexFormat(inside_corner, bottom_inside_rail_color),
      VertexFormat(inside_corner + glm::vec3(inside_width, 0, 0),
                   bottom_inside_rail_color),
      VertexFormat(inside_corner + glm::vec3(inside_width, 0, -inside_length),
                   bottom_inside_rail_color),
      VertexFormat(inside_corner + glm::vec3(0, 0, -inside_length),
                   bottom_inside_rail_color),
      VertexFormat(corner + glm::vec3(0, height, 0), rail_color),
      VertexFormat(corner + glm::vec3(width, height, 0), rail_color),
      VertexFormat(corner + glm::vec3(width, height, -length), rail_color),
      VertexFormat(corner + glm::vec3(0, height, -length), rail_color),
      VertexFormat(inside_corner + glm::vec3(0, height, 0), inside_rail_color),
      VertexFormat(inside_corner + glm::vec3(inside_width, height, 0),
                   inside_rail_color),
      VertexFormat(
          inside_corner + glm::vec3(inside_width, height, -inside_length),
          inside_rail_color),
      VertexFormat(inside_corner + glm::vec3(0, height, -inside_length),
                   inside_rail_color),
      VertexFormat(corner, slate_color),
      VertexFormat(corner + glm::vec3(width, 0, 0), slate_color),
      VertexFormat(corner + glm::vec3(width, 0, -length), slate_color),
      VertexFormat(corner + glm::vec3(0, 0, -length), slate_color)};

  std::vector<unsigned short> indices = {
      0,  1,  9,  0,  9,  8,  1,  2, 10, 1,
      10, 9,  3,  2,  10, 3,  10, 11, 0,  3,  11, 0,  11, 8, 4,  5,
      13, 4,  13, 12, 5,  6,  14, 5,  14, 13, 7,  6,  14, 7, 14, 15,
      4,  7,  15, 4,  15, 12, 8,  9,  12, 12, 9,  13, 13, 9, 10, 13,
      10, 14, 14, 10, 15, 10, 11, 15, 15, 11, 12, 11, 8,  12, 16, 17, 18, 16, 18, 19};

  InitFromData(vertices, indices);
}

Table::~Table() {}


}  // namespace pool
