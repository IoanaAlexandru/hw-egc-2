#include "pool/game/player.h"

namespace pool {
Player::Player() { Player("Player"); }

Player::Player(std::string name) {
  color_ = glm::vec3(1);
  first_hit_ = glm::vec3(1);
  none_potted_ = true;
  none_hit_ = true;
  hit_rail_ = false;
  faults_ = 0;
  own_balls_potted_ = 0;
  opponent_balls_potted_ = 0;
  cue_balls_potted_ = 0;
  own_balls_left_ = 7;
  best_combo_ = 0;
  combo_ = 0;
  fault_ = false;
  this->name_ = name;
}

Player::~Player(){};

void Player::Reset() {
  if (combo_ > best_combo_) best_combo_ = combo_;
  combo_ = 0;
  first_hit_ = glm::vec3(1);
  hit_rail_ = false;
  none_potted_ = true;
  none_hit_ = true;
  fault_ = false;
}

HitStatus Player::HitBall(glm::vec3 ball_color) {
  none_hit_ = false;

  if (first_hit_ == glm::vec3(1) && !hit_rail_)
    first_hit_ = ball_color;
  else
    return HitStatus::OK;  // not the first ball hit so it doesn't matter
  if (first_hit_ == glm::vec3(0.2, 0.2, 0.2)) {
    if (own_balls_left_ > 0) {
      fault_ = true;
      faults_++;
      return HitStatus::FAULT_BLACK;
    } else {
      return HitStatus::OK;
    }
  }
  if (first_hit_ == color_ || color_ == glm::vec3(1)) return HitStatus::OK;
  fault_ = true;
  faults_++;
  return HitStatus::FAULT_OPPONENT;
}

PotStatus Player::PotBall(glm::vec3 ball_color) {
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
    combo_++;
    return PotStatus::OK;
  }
  opponent_balls_potted_++;
  fault_ = true;
  faults_++;
  return PotStatus::FAULT_OPPONENT;
}

void Player::PrintStats() {
  std::cout << std::endl
            << name_.c_str() << "'s stats:" << std::endl
            << "> Faults: " << faults_ << std::endl
            << "> Owned balls potted: " << own_balls_potted_ << std::endl
            << "> Opponent's balls potted: " << opponent_balls_potted_
            << std::endl
            << "> Cue balls potted: " << cue_balls_potted_ << std::endl
            << "> Best combo: " << best_combo_ << std::endl;
}
}  // namespace pool