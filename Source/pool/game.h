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
  void Update(float deltaTimeSeconds) override;
  void FrameEnd() override;

  void RenderSimpleMesh(Mesh *mesh, Shader *shader,
                        const glm::mat4 &modelMatrix,
                        const glm::vec3 &color = glm::vec3(1));

  void OnInputUpdate(float deltaTime, int mods) override;
  void OnKeyPress(int key, int mods) override;
  void OnKeyRelease(int key, int mods) override;
  void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
  void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
  void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
  void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
  void OnWindowResize(int width, int height) override;

  void TopDownView();
  void ThirdPersonView();

  static const float kTableWidth, kTableHeight, kTableLength, kTableThickness,
      kBallRadius, kPocketRadius;
  static const glm::vec3 kTableSlateColor, kTableRailColor, kPlayerOneColor,
      kPlayerTwoColor;
  static const float kMovementSpeed;
  static const int kBlackBallIndex = 5, kCueBallIndex = 0;

  glm::vec3 lightPosition;
  unsigned int materialShininess;
  float materialKd;
  float materialKs;

  Table *table;
  std::vector<Ball *> pockets;
  std::vector<Ball *> balls;
};
}  // namespace pool

#endif  // POOL_GAME_H_
