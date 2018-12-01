#ifndef POOL_GAME_H_
#define POOL_GAME_H_

#include <Component/SimpleScene.h>
#include <Component/Transform/Transform.h>
#include <Core/GPU/Mesh.h>

#include "pool/camera.h"
#include "pool/objects/ball.h"
#include "pool/objects/cue.h"

namespace pool {
typedef struct {
  int shininess;
  float kd, ks;
} MaterialProperties;

enum class GameStage {
  PlaceCueBall, HitCueBall, ViewShot
};

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
                        MaterialProperties properties,
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

  void ViewShot();
  void PlaceCueBall();
  void HitCueBall();

  static const float kTableWidth, kTableLength, kBallRadius, kCueLength,
      kPocketRadius, kTableBedBorder;
  static const glm::vec3 kTableBedColor, kTableColor, kTableMetalColor,
      kCueColor, kPlayerOneColor, kPlayerTwoColor;
  static const float kMovementSpeed, kMaxCueOffset, kSensitivity;
  static const int kBlackBallIndex = 5, kCueBallIndex = 0;
  static const glm::mat4 kTableModelMatrix;

  Camera *camera_;

  glm::vec3 light_position_;
  MaterialProperties ball_properties_, cue_properties_, metal_properties_,
      table_properties_, velvet_properties_;
  float cue_offset_;
  float cue_movement_speed_;

  Mesh *table_, *table_bed_, *table_metal_;
  Cue *cue_;
  std::vector<Ball *> balls_;
  std::vector<Ball *> pockets_;

  GameStage stage_;
};
}  // namespace pool

#endif  // POOL_GAME_H_
