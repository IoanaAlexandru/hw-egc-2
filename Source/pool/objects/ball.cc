#include "pool/objects/ball.h"

#include <algorithm>

#include <Core/Managers/ResourcePath.h>

namespace pool {
Ball::Ball(std::string name, glm::vec3 center, float radius, glm::vec3 color)
    : Mesh(name) {
  {
    LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "sphere.obj");
    center_ = center;
    initial_center_ = center;
    color_ = color;
    radius_ = radius;
    movement_vector_ = glm::vec3(0);
    model_matrix_ = glm::translate(model_matrix_, center);
    scale_ = glm::vec3(radius / kDefaultRadius);
    model_matrix_ = glm::scale(model_matrix_, scale_);
  }
}

Ball::~Ball(){};

void Ball::Update(float delta_time) {
  glm::vec3 movement = delta_time * movement_vector_;
  center_ += movement;
  UpdateModelMatrix();

  movement_vector_ *= 0.995;
  if (abs(movement_vector_.x) < 0.1 && abs(movement_vector_.z) < 0.1) {
    movement_vector_ = glm::vec3(0, 0, 0);
  }
}

void Ball::MoveUp(float delta_time) {
  center_.z -= delta_time * kDefaultSpeed;
  UpdateModelMatrix();
}

void Ball::MoveDown(float delta_time) {
  center_.z += delta_time * kDefaultSpeed;
  UpdateModelMatrix();
}

void Ball::MoveRight(float delta_time) {
  center_.x += delta_time * kDefaultSpeed;
  UpdateModelMatrix();
}

void Ball::MoveLeft(float delta_time) {
  center_.x -= delta_time * kDefaultSpeed;
  UpdateModelMatrix();
}

void Ball::CueHit(glm::vec3 direction, float distance) {
  movement_vector_ = direction * distance;
}

void Ball::ReflectX() { movement_vector_.x *= -1; }

void Ball::ReflectZ() { movement_vector_.z *= -1; }

bool Ball::AreTouching(Ball *ball1, Ball *ball2) {
  return glm::distance(ball1->GetCenter(), ball2->GetCenter()) <=
         ball1->GetRadius() + ball2->GetRadius();
}

/*
Collision detection between a moving ball and a stationary one as per the
algorithm at
http://www.gamasutra.com/view/feature/131424/pool_hall_lessons_fast_accurate_.php?page=2
*/
bool Ball::DynamicStaticCollision(Ball *ball1, Ball *ball2, float delta_time) {
  glm::vec3 v = ball1->GetMoveVec();
  glm::vec3 c1 = ball1->GetCenter();
  glm::vec3 c2 = ball2->GetCenter();
  float r1 = ball1->GetRadius();
  float r2 = ball2->GetRadius();
  float speed = ball1->GetSpeed() * delta_time;

  // If the length of the movement vector is less than the distance between the
  // centers of the balls minus their radii, they can't hit
  float dist = glm::distance(c2, c1);
  float sum_radii = r2 + r1;
  dist -= sum_radii;
  if (speed < dist) return false;

  // Normalize movement vector
  glm::vec3 n = glm::normalize(v);

  // Find vector between centers
  glm::vec3 c = c2 - c1;
  float c_len = glm::length(c);

  // Make sure ball1 is moving towards ball2
  float d = glm::dot(n, c);
  if (d <= 0) return false;

  // If the closest that ball1 will get to ball2 is more than the sum of their
  // radii, they can't hit
  float t = sum_radii * sum_radii - c_len * c_len + d * d;
  if (0 >= t) return false;

  // Compute the distance the ball has to travel along the movement vector
  float distance = d - sqrt(t);

  // Make sure the distance is not greater than the ball's speed
  if (speed < distance) return false;

  return true;
}

bool Ball::DynamicDynamicCollision(Ball *ball1, Ball *ball2, float delta_time) {
  glm::vec3 v1 = ball1->GetMoveVec();
  glm::vec3 v2 = ball2->GetMoveVec();

  // We consider ball2 to be stationary
  ball1->SetMoveVec(v1 - v2);
  bool colliding = DynamicStaticCollision(ball1, ball2, delta_time);

  // Reverting movement vector
  ball1->SetMoveVec(v1);
  return colliding;
}

/*
Check if moving ball1 collides with ball2.
*/
bool Ball::CheckCollision(Ball *ball1, Ball *ball2, float delta_time) {
  if (ball2->IsMoving()) return DynamicDynamicCollision(ball1, ball2, delta_time);
  return DynamicStaticCollision(ball1, ball2, delta_time);
}

/*
Make balls bounce off each other as per the algorithm at
http://www.gamasutra.com/view/feature/131424/pool_hall_lessons_fast_accurate_.php?page=3.
 */
void Ball::Bounce(Ball *ball1, Ball *ball2) {
  // Get movement vectors and masses of balls
  glm::vec3 v1 = ball1->GetMoveVec();
  glm::vec3 v2 = ball2->GetMoveVec();
  float m1 = ball1->GetMass();
  float m2 = ball2->GetMass();

  // Find normalized vector between the centers of the balls
  glm::vec3 n = ball1->GetCenter() - ball2->GetCenter();
  n = glm::normalize(n);

  // Find the length of the component of each of the movement
  // vectors along n.
  float a1 = glm::dot(v1, n);
  float a2 = glm::dot(v2, n);

  float optimized = (2.0f * (a1 - a2)) / (m1 + m2);

  // Calculate new movement vectors
  ball1->SetMoveVec(v1 - optimized * m2 * n);
  ball2->SetMoveVec(v2 + optimized * m1 * n);
}

void Ball::UpdateModelMatrix() {
  model_matrix_ = glm::translate(glm::mat4(1), center_);
  model_matrix_ = glm::scale(model_matrix_, scale_);
}
}  // namespace pool