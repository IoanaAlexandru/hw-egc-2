#ifndef POOL_GAME_H_
#define POOL_GAME_H_

#include <Component/SimpleScene.h>
#include <Component/Transform/Transform.h>
#include <Core/GPU/Mesh.h>

#include "pool/game/player.h"
#include "pool/camera.h"
#include "pool/objects/ball.h"
#include "pool/objects/cue.h"
#include "pool/shadows/ShadowMapFBO.h"

namespace pool {
typedef struct {
  int shininess;
  float kd, ks;
} MaterialProperties;

enum class GameStage { BREAK, PLACE_CUE_BALL, HIT_CUE_BALL, VIEW_SHOT, LOOK_AROUND };

class Game : public SimpleScene {
 public:
  Game();
  ~Game();

  void Init() override;

  void StartGame();
  Player GetPlayerName(std::string default);
  void EndGame();
  void TogglePlayer();
  void Help();

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
  
  Ball* GetClosestOwnedBall(glm::vec3 point);

  /*
  Same as PlaceCueBall, but ball can only be placed in the first quarter of the
  table.
  */
  void Break();
  /*
  View shot from above.
  Can toggle LookAround mode with V, or to HitCueBall mode with SPACE if cue
  ball isn't moving.
  */
  void ViewShot();
  /*
  Place cue ball with the WASD keys.
  Press V to toggle LookAround mode, or press SPACE to start shot.
  */
  void PlaceCueBall();
  /*
  Press RIGHT_MOUSE_BUTTON and move mouse to position shot horizontally. Press
  LEFT_MOUSE_BUTTON to start moving cue, release to hit (the further the cue is
  from the ball, the stronger the shot).
  */
  void HitCueBall();
  /*
  Toggle LookAround mode. When enabled, look around using the mouse and the
  WASDEQ keys (first-person view) by pressing RIGHT_MOUSE_BUTTON.
  */
  void LookAround();

  void setDefaultFrameBuffer();

#pragma region CONSTANTS
  // Object size constants
  static const float kTableWidth, kTableLength, kBallRadius, kCueLength,
      kPocketRadius, kTableBedBorder;
  // Color constants
  static const glm::vec3 kTableBedColor, kTableColor, kMetalColor, kCueColor,
      kRed, kYellow;
  // Speed constants
  static const float kMovementSpeed, kSensitivity;

  static const float kMaxCueOffset;
  static const int kBlackBallIndex, kCueBallIndex;
  static const glm::mat4 kTableModelMatrix;
  static const std::string kPoolShaderName;
#pragma endregion

  // 3D scene elements

  Camera *camera_;
  Mesh *table_, *table_bed_, *table_metal_, *lamp_;
  Cue *cue_;
  std::vector<Ball *> balls_;
  std::vector<Ball *> pockets_;

  // Object properties

  MaterialProperties ball_properties_, cue_properties_, metal_properties_,
      table_properties_, velvet_properties_;
  glm::vec3 lamp_position_;
  bool render_lamp_;
  float cue_offset_, cue_movement_speed_;

  // Game elements

  GameStage stage_, prev_stage_;
  Player player_one_, player_two_;
  Player *current_player_;
  std::unordered_map<GameStage, bool> print_help_;
  bool press_space_to_continue_, end_;
};
}  // namespace pool

#endif  // POOL_GAME_H_
