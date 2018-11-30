#ifndef POOL_GAME_H_
#define POOL_GAME_H_

#include <Component/SimpleScene.h>
#include <Component/Transform/Transform.h>
#include <Core/GPU/Mesh.h>
#include "pool/objects/ball.h"
#include "pool/objects/cue.h"

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
                        const glm::mat4 &model_matrix, float z_offset,
                        const glm::vec3 &color = glm::vec3(1));

  void OnInputUpdate(float delta_time, int mods) override;
  void OnKeyPress(int key, int mods) override;
  void OnKeyRelease(int key, int mods) override;
  void OnMouseMove(int mouse_x, int mouse_y, int delta_x, int delta_y) override;
  void OnMouseBtnPress(int mouse_x, int mouse_y, int button, int mods) override;
  void OnMouseBtnRelease(int mouse_x, int mouse_y, int button,
                         int mods) override;
  void OnMouseScroll(int mouse_x, int mouse_y, int offset_x,
                     int offset_y) override;
  void OnWindowResize(int width, int height) override;

  glm::vec2 GetViewPoint(glm::vec2 target_pos, glm::vec2 ball_pos);
  void TopDownView();
  void ThirdPersonView();

  static const float kTableWidth, kTableLength, kBallRadius, kCueLength, kPocketRadius, kTableBedBorder;
  static const glm::vec3 kTableBedColor, kTableColor, kTableMetalColor,
      kCueColor, kPlayerOneColor, kPlayerTwoColor;
  static const float kMovementSpeed, kCueBallViewDistance, kCueBallViewHeight,
      kMaxCueOffset;
  static const int kBlackBallIndex = 5, kCueBallIndex = 0;
  static const glm::mat4 kTableModelMatrix;

  glm::vec3 light_position_;
  unsigned int ball_shininess_;
  float ball_kd_;
  float ball_ks_;
  float cue_offset_;
  float cue_movement_speed_;

  Mesh *table_, *table_bed_, *table_metal_;
  Cue *cue_;
  std::vector<Ball *> balls_;
  std::vector<Ball *> pockets_;

  bool place_cue_ball_ = true;
};
}  // namespace pool

#endif  // POOL_GAME_H_
