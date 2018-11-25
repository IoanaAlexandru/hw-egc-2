#ifndef POOL_GAME_H_
#define POOL_GAME_H_

#pragma once
#include <Component/SimpleScene.h>
#include <Component/Transform/Transform.h>
#include <Core/GPU/Mesh.h>
#include "pool/objects/ball.h"
#include "pool/objects/table.h"

namespace pool {
class Game : public SimpleScene {
 public:
  Game();
  ~Game();

  void Init() override;

 private:
  void FrameStart() override;
  void Update(float delta_time_seconds) override;
  void FrameEnd() override;

  void RenderSimpleMesh(Mesh *mesh, Shader *shader,
                        const glm::mat4 &model_matrix,
                        const glm::vec3 &color = glm::vec3(1));

  void OnInputUpdate(float delta_time, int mods) override;
  void OnKeyPress(int key, int mods) override;
  void OnKeyRelease(int key, int mods) override;
  void OnMouseMove(int mouse_x, int mouse_y, int delta_x, int delta_y) override;
  void OnMouseBtnPress(int mouse_x, int mouse_y, int button, int mods) override;
  void OnMouseBtnRelease(int mouse_x, int mouse_y, int button, int mods) override;
  void OnMouseScroll(int mouse_x, int mouse_y, int offset_x, int offset_y) override;
  void OnWindowResize(int width, int height) override;

  glm::vec2 GetViewPoint(glm::vec2 target_pos, glm::vec2 ball_pos);
  void TopDownView();
  void ThirdPersonView();

  static const float kTableWidth, kTableHeight, kTableLength, kTableThickness,
      kBallRadius, kPocketRadius;
  static const glm::vec3 kTableSlateColor, kTableRailColor, kPlayerOneColor,
      kPlayerTwoColor;
  static const float kMovementSpeed, kCueBallViewDistance, kCueBallViewHeight;
  static const int kBlackBallIndex = 5, kCueBallIndex = 0;

  glm::vec3 light_position_;
  unsigned int ball_shininess_;
  float ball_kd_;
  float ball_ks_;

  Table *table_;
  std::vector<Ball *> pockets_;
  std::vector<Ball *> balls_;
};
}  // namespace pool

#endif  // POOL_GAME_H_
