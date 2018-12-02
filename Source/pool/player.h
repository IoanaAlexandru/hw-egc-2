#ifndef POOL_PLAYER_H_
#define POOL_PLAYER_H_

#include <iostream>

#include <include/glm.h>

namespace pool {
enum class PotStatus { OK, FAULT_CUE_BALL, FAULT_OPPONENT, WIN, LOSS };
enum class HitStatus { OK, FAULT_OPPONENT, FAULT_BLACK };

class Player {
 public:
  Player() { Player("Player"); }

  Player(std::string name) {
    color_ = glm::vec3(1);
    first_hit_ = glm::vec3(1);
    none_potted_ = true;
    faults_ = 0;
    own_balls_potted_ = 0;
    opponent_balls_potted_ = 0;
    cue_balls_potted_ = 0;
    own_balls_left_ = 7;
    best_combo_ = 0;
    combo_ = 0;
    this->name_ = name;
  }

  ~Player(){};

  inline glm::vec3 GetColor() { return color_; }
  inline void SetColor(glm::vec3 color) { color_ = color; }
  inline std::string GetName() { return name_; }
  inline bool NonePotted() { return none_potted_; }
  inline bool Fault() { return fault_; }

  inline void Reset() {
    if (combo_ > best_combo_) best_combo_ = combo_;
    combo_ = 0;
    first_hit_ = glm::vec3(1);
    none_potted_ = true;
    fault_ = false;
  }

  inline void OwnBallPotted() { own_balls_left_--; }

  inline HitStatus HitBall(glm::vec3 ball_color) {
    if (first_hit_ == glm::vec3(1))
      first_hit_ = ball_color;
    else
      return HitStatus::OK;  // not the first ball hit so it doesn't matter
    if (first_hit_ == color_ || color_ == glm::vec3(1)) return HitStatus::OK;
    if (first_hit_ == glm::vec3(0.2, 0.2, 0.2)) {
      if (own_balls_left_ > 0) {
        fault_ = true;
        faults_++;
        return HitStatus::FAULT_BLACK;
      } else {
        return HitStatus::OK;
      }
    }
    fault_ = true;
    faults_++;
    return HitStatus::FAULT_OPPONENT;
  }

  inline PotStatus PotBall(glm::vec3 ball_color) {
    none_potted_ = false;

    if (ball_color == glm::vec3(0.2, 0.2, 0.2)) {
      if (own_balls_left_ > 0)
        return PotStatus::LOSS;
      else
        return PotStatus::WIN;
    }
    if (ball_color == glm::vec3(0.9, 0.9, 0.9)) {
      cue_balls_potted_++;
      fault_ = true;
      faults_++;
      return PotStatus::FAULT_CUE_BALL;
    }
    if (color_ == glm::vec3(1)) color_ = ball_color;  // initialise color
    if (color_ == ball_color) {
      own_balls_potted_++;
      return PotStatus::OK;
    }
    opponent_balls_potted_++;
    fault_ = true;
    faults_++;
    return PotStatus::FAULT_OPPONENT;
  }

  inline void PrintStats() {
    std::cout << std::endl
              << name_ << "'s stats:" << std::endl
              << "> Faults: " << faults_ << std::endl
              << "> Owned balls potted: " << own_balls_potted_ << std::endl
              << "> Opponent's balls potted: " << opponent_balls_potted_
              << std::endl
              << "> Cue balls potted: " << cue_balls_potted_ << std::endl;
  }

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
  bool none_potted_;
  bool fault_;
  int combo_;
};
}  // namespace pool

#endif  // POOL_PLAYER_H_