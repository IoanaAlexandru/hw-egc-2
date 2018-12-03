#ifndef POOL_PLAYER_H_
#define POOL_PLAYER_H_

#include <iostream>

#include <include/glm.h>

namespace pool {
enum class PotStatus { OK, FAULT_CUE_BALL, FAULT_OPPONENT, WIN, LOSS };
enum class HitStatus { OK, FAULT_OPPONENT, FAULT_BLACK };

class Player {
 public:
  Player();
  Player(std::string name);
  ~Player();

  inline glm::vec3 GetColor() { return color_; }
  inline void SetColor(glm::vec3 color) { color_ = color; }
  inline std::string GetName() { return name_; }
  inline bool NonePotted() { return none_potted_; }
  inline bool NoneHit() { return none_hit_; }
  inline bool Fault() { return fault_; }
  inline void AddFault() { faults_++; }

  void Reset();

  inline void OwnBallPotted() { own_balls_left_--; }
  inline void HitRail() { hit_rail_ = true; }

  HitStatus HitBall(glm::vec3 ball_color);
  PotStatus PotBall(glm::vec3 ball_color);

  void PrintStats();

 private:
  // Player data
  std::string name_;
  glm::vec3 color_;

  // Player stats
  int faults_;
  int own_balls_potted_;
  int opponent_balls_potted_;
  int cue_balls_potted_;
  int own_balls_left_;
  int best_combo_;

  // Status
  glm::vec3 first_hit_;
  bool hit_rail_;
  bool none_potted_;
  bool none_hit_;
  bool fault_;
  int combo_;
};
}  // namespace pool

#endif  // POOL_PLAYER_H_