#include "game.h"

#include <iostream>
#include <string>
#include <vector>

#include <Core/Engine.h>
#include <Engine/Component/Camera/Camera.h>

using namespace std;

namespace pool {

#pragma region CONSTANTS
const float Game::kTableWidth = 2.16f, Game::kTableLength = 4.26f,
            Game::kBallRadius = 0.07f, Game::kCueLength = 2.6f,
            Game::kPocketRadius = 0.12f, Game::kTableBedBorder = 0.08f;
const glm::vec3 Game::kTableBedColor = glm::vec3(0, 0.5, 0.1),
                Game::kTableColor = glm::vec3(0.3, 0.05, 0.05),
                Game::kMetalColor = glm::vec3(0.8, 0.8, 0.8),
                Game::kCueColor = glm::vec3(0.5, 0.15, 0.15),
                Game::kPlayerOneColor = glm::vec3(0.86, 0.20, 0.21),
                Game::kPlayerTwoColor = glm::vec3(0.96, 0.76, 0.05);
const float Game::kMovementSpeed = 2.0f, Game::kSensitivity = 0.001f;
const float Game::kMaxCueOffset = 2.0f;
const int Game::kBlackBallIndex = 5, Game::kCueBallIndex = 0;
const glm::mat4 Game::kTableModelMatrix =
    glm::scale(glm::mat4(1), glm::vec3(2.0f));
#pragma endregion

Game::Game() {}

Game::~Game() {}

void Game::Init() {
  {
    render_lamp_ = true;
    camera_ = new Camera(window->props.aspectRatio);
    PlaceCueBall();
  }

  // Table
  {
    table_ = new Mesh("table");
    table_->LoadMesh(RESOURCE_PATH::MODELS + "Props", "table.obj");

    table_bed_ = new Mesh("table_bed");
    table_bed_->LoadMesh(RESOURCE_PATH::MODELS + "Props", "table_bed.obj");

    table_metal_ = new Mesh("table_metal");
    table_metal_->LoadMesh(RESOURCE_PATH::MODELS + "Props", "table_metal.obj");
  }

  // Pockets
  {
    glm::vec3 pure_black = glm::vec3(0, 0, 0);
    glm::vec3 corner = glm::vec3(-kTableWidth / 2 - kPocketRadius, 0, 0);
    pockets_.push_back(
        new Ball("middle_left", corner, kPocketRadius, pure_black));
    pockets_.push_back(new Ball(
        "lower_left", corner + glm::vec3(kPocketRadius, 0, kTableLength / 2),
        kPocketRadius, pure_black));
    pockets_.push_back(new Ball(
        "upper_left", corner + glm::vec3(kPocketRadius, 0, -kTableLength / 2),
        kPocketRadius, pure_black));
    corner = glm::vec3(kTableWidth / 2 + kPocketRadius, 0, 0);
    pockets_.push_back(
        new Ball("middle_right", corner, kPocketRadius, pure_black));
    pockets_.push_back(new Ball(
        "lower_right", corner + glm::vec3(-kPocketRadius, 0, kTableLength / 2),
        kPocketRadius, pure_black));
    pockets_.push_back(new Ball(
        "upper_right", corner + glm::vec3(-kPocketRadius, 0, -kTableLength / 2),
        kPocketRadius, pure_black));
  }

  // Balls
  {
    glm::vec3 ball_center = glm::vec3(0, kBallRadius, kTableLength / 4);
    glm::vec3 ball_color = glm::vec3(0.9, 0.9, 0.9);
    std::string ball_name = "cue_ball";
    balls_.push_back(new Ball(ball_name, ball_center, kBallRadius, ball_color));

    ball_center = glm::vec3(0, kBallRadius, -kTableLength / 5);
    ball_color = kPlayerOneColor;
    float row_offset = std::sqrt(pow(2 * kBallRadius, 2) - pow(kBallRadius, 2));
    int ball_count = 0;
    for (auto i = 0; i < 5; i++) {
      for (auto j = 0; j < i + 1; j++) {
        if (i == 2 && j == 1) {
          ball_color = glm::vec3(0.2, 0.2, 0.2);
          ball_name = "black_ball";
        } else {
          ball_color =
              (ball_count + i) % 2 == 0 ? kPlayerOneColor : kPlayerTwoColor;
          ball_name = "ball" + std::to_string(ball_count++);
        }

        balls_.push_back(new Ball(
            ball_name, ball_center + glm::vec3(2 * kBallRadius * j, 0, 0),
            kBallRadius, ball_color));
      }

      ball_center.x -= kBallRadius;
      ball_center.z -= row_offset;
    }
  }

  // Cue
  {
    cue_ = new Cue("cue", balls_[kCueBallIndex]->GetCenter(), kCueLength,
                   kCueColor);
    cue_offset_ = 0;
    cue_movement_speed_ = kMovementSpeed;
  }

  // Lamp
  {
    lamp_ = new Mesh("lamp");
    lamp_->LoadMesh(RESOURCE_PATH::MODELS + "Props", "lamp.obj");
  }

  // Shader
  {
    Shader *shader = new Shader("PoolShader");
    shader->AddShader("Source/pool/shaders/VertexShader.glsl",
                      GL_VERTEX_SHADER);
    shader->AddShader("Source/pool/shaders/FragmentShader.glsl",
                      GL_FRAGMENT_SHADER);
    shader->CreateAndLink();
    shaders[shader->GetName()] = shader;
  }

  // Light & material properties
  {
    lamp_position_ = glm::vec3(0, 1.7, 0);

    ball_properties_.shininess = 96;
    ball_properties_.kd = 1.8f;
    ball_properties_.ks = 1.5f;

    cue_properties_.shininess = 96;
    cue_properties_.kd = 1.5f;
    cue_properties_.ks = 1.5f;

    metal_properties_.shininess = 96;
    metal_properties_.kd = 1.92f;
    metal_properties_.ks = 1.5f;

    table_properties_.shininess = 96;
    table_properties_.kd = 0.5f;
    table_properties_.ks = 1.5f;

    velvet_properties_.shininess = 50;
    velvet_properties_.kd = 1.2f;
    velvet_properties_.ks = 1.5f;
  }
}

void Game::FrameStart() {
  // clears the color buffer (using the previously set color) and depth buffer
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::ivec2 resolution = window->GetResolution();
  // Sets the screen area where to draw
  glViewport(0, 0, resolution.x, resolution.y);
}

void Game::Update(float delta_time_seconds) {
  // Collisions
  {
    for (auto ball : balls_) {
      if (!ball->IsPotted()) {
        glm::vec3 center = ball->GetCenter();

        // Pockets
        for (auto pocket : pockets_) {
          if (Ball::CheckCollision(ball, pocket, delta_time_seconds)) {
            ball->SetPotted(true);
          }
        }
        if (ball->IsPotted()) continue;

        // Rails
        float down = center.z + kBallRadius - kTableLength / 2;
        float up = center.z - kBallRadius + kTableLength / 2;
        float right = center.x + kBallRadius - kTableWidth / 2;
        float left = center.x - kBallRadius + kTableWidth / 2;
        bool middle = abs(center.z) + kBallRadius < kPocketRadius;

        if (down > 0)
          ball->ReflectZ(down);
        else if (up < 0)
          ball->ReflectZ(up);
        else if (right > 0 && !middle)
          ball->ReflectX(right);
        else if (left < 0 && !middle)
          ball->ReflectX(left);
        else if ((right > 0 || left < 0) && middle)
          ball->SetPotted(true);

        // Balls
        if (ball->IsMoving()) {
          for (auto another_ball : balls_) {
            if (another_ball == ball || another_ball->IsPotted()) continue;
            if (Ball::CheckCollision(ball, another_ball, delta_time_seconds)) {
              Ball::Bounce(ball, another_ball);
              break;
            }
          }
        }
      }
    }
  }

  // Check status of special balls
  {
    if (balls_[kCueBallIndex]->IsPotted()) {
      balls_[kCueBallIndex]->Reset();
      PlaceCueBall();
    }
  }

  // Render objects
  {
    // Table
    RenderSimpleMesh(table_, shaders["PoolShader"], kTableModelMatrix, 0,
                     table_properties_, kTableColor);
    RenderSimpleMesh(table_metal_, shaders["PoolShader"], kTableModelMatrix, 0,
                     metal_properties_, kMetalColor);
    RenderSimpleMesh(table_bed_, shaders["PoolShader"], kTableModelMatrix, 0,
                     velvet_properties_, kTableBedColor);

    // Balls
    for (auto ball : balls_) {
      ball->Update(delta_time_seconds);
      RenderSimpleMesh((Mesh *)ball, shaders["PoolShader"],
                       ball->GetModelMatrix(), 0, ball_properties_,
                       ball->GetColor());
    }

    // Cue
    if (stage_ == GameStage::HitCueBall)
      RenderSimpleMesh((Mesh *)cue_, shaders["PoolShader"],
                       cue_->GetModelMatrix(), cue_offset_, cue_properties_,
                       cue_->GetColor());

    // Light point
    if (render_lamp_)
      RenderSimpleMesh(lamp_, shaders["PoolShader"],
                       glm::translate(glm::mat4(1), lamp_position_), 0,
                       metal_properties_, kMetalColor);
  }
}

void Game::FrameEnd() {
  DrawCoordinatSystem(camera_->GetViewMatrix(), camera_->GetProjectionMatrix());
}

void Game::RenderSimpleMesh(Mesh *mesh, Shader *shader,
                            const glm::mat4 &model_matrix, float z_offset,
                            MaterialProperties properties,
                            const glm::vec3 &color) {
  if (!mesh || !shader || !shader->GetProgramID()) return;

  // Render an object using the specified shader and the specified position
  glUseProgram(shader->program);

  // Set shader uniforms for light & material properties
  GLint light_loc = glGetUniformLocation(shader->program, "light_position");
  glUniform3fv(light_loc, 1, glm::value_ptr(lamp_position_));

  GLint shininess_loc =
      glGetUniformLocation(shader->program, "material_shininess");
  glUniform1ui(shininess_loc, properties.shininess);

  GLint eye_position_loc =
      glGetUniformLocation(shader->program, "eye_position");
  glm::vec3 eye_position = glm::vec3(0, 0, 0);
  glUniform3fv(eye_position_loc, 1, glm::value_ptr(eye_position));

  GLint kd_loc = glGetUniformLocation(shader->program, "material_kd");
  glUniform1f(kd_loc, properties.kd);

  GLint ks_loc = glGetUniformLocation(shader->program, "material_ks");
  glUniform1f(ks_loc, properties.ks);

  GLint color_loc = glGetUniformLocation(shader->program, "object_color");
  glUniform3fv(color_loc, 1, glm::value_ptr(color));

  GLint z_offset_loc = glGetUniformLocation(shader->program, "z_offset");
  glUniform1f(z_offset_loc, z_offset);

  // Bind model matrix
  GLint model_matrix_loc = glGetUniformLocation(shader->program, "Model");
  glUniformMatrix4fv(model_matrix_loc, 1, GL_FALSE,
                     glm::value_ptr(model_matrix));

  // Bind view matrix
  glm::mat4 view_matrix = camera_->GetViewMatrix();
  int view_matrix_loc = glGetUniformLocation(shader->program, "View");
  glUniformMatrix4fv(view_matrix_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));

  // Bind projection matrix
  glm::mat4 projection_matrix = camera_->GetProjectionMatrix();
  int loc_projection_matrix =
      glGetUniformLocation(shader->program, "Projection");
  glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE,
                     glm::value_ptr(projection_matrix));

  // Draw the object
  glBindVertexArray(mesh->GetBuffers()->VAO);
  glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()),
                 GL_UNSIGNED_SHORT, 0);
}

#pragma region INPUT UPDATE

void Game::OnInputUpdate(float delta_time, int mods) {
  if (!window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT)) {
    if (mods == GLFW_MOD_CONTROL) {
      // Control light position using CTRL + W, A, S, D, E, Q
      glm::vec3 up = glm::vec3(0, 1, 0);
      glm::vec3 right = glm::vec3(1, 0, 0);
      glm::vec3 forward = glm::vec3(0, 0, 1);

      if (window->KeyHold(GLFW_KEY_W))
        lamp_position_ -= forward * delta_time * kMovementSpeed;
      if (window->KeyHold(GLFW_KEY_A))
        lamp_position_ -= right * delta_time * kMovementSpeed;
      if (window->KeyHold(GLFW_KEY_S))
        lamp_position_ += forward * delta_time * kMovementSpeed;
      if (window->KeyHold(GLFW_KEY_D))
        lamp_position_ += right * delta_time * kMovementSpeed;
      if (window->KeyHold(GLFW_KEY_E))
        lamp_position_ += up * delta_time * kMovementSpeed;
      if (window->KeyHold(GLFW_KEY_Q))
        lamp_position_ -= up * delta_time * kMovementSpeed;
    } else if (stage_ == GameStage::PlaceCueBall) {
      // Move cue ball using W, A, S, D
      Ball *cue_ball = balls_[kCueBallIndex];
      glm::vec3 pos = cue_ball->GetCenter();

      if (window->KeyHold(GLFW_KEY_W) && pos.z > kTableLength / 4)
        cue_ball->MoveUp(delta_time);
      if (window->KeyHold(GLFW_KEY_A) && pos.x > -kTableWidth / 2 + kBallRadius)
        cue_ball->MoveLeft(delta_time);
      if (window->KeyHold(GLFW_KEY_S) &&
          pos.z < kTableLength / 2 - kBallRadius - kPocketRadius)
        cue_ball->MoveDown(delta_time);
      if (window->KeyHold(GLFW_KEY_D) && pos.x < kTableWidth / 2 - kBallRadius)
        cue_ball->MoveRight(delta_time);
    }
  } else if (stage_ == GameStage::LookAround) {
    // Move camera using W, A, S, D, E, Q
    if (window->KeyHold(GLFW_KEY_W)) camera_->TranslateForward(delta_time);
    if (window->KeyHold(GLFW_KEY_A)) camera_->TranslateRight(-delta_time);
    if (window->KeyHold(GLFW_KEY_S)) camera_->TranslateForward(-delta_time);
    if (window->KeyHold(GLFW_KEY_D)) camera_->TranslateRight(delta_time);
    if (window->KeyHold(GLFW_KEY_Q)) camera_->TranslateUpword(-delta_time);
    if (window->KeyHold(GLFW_KEY_E)) camera_->TranslateUpword(delta_time);
  }

  if (window->MouseHold(GLFW_MOUSE_BUTTON_LEFT)) {
    // Move cue closer/further from the ball to choose shot intensity
    cue_offset_ += cue_movement_speed_ * delta_time;
    if (cue_offset_ >= kMaxCueOffset || cue_offset_ <= 0)
      cue_movement_speed_ *= -1;
  }
}

void Game::OnKeyPress(int key, int mods) {
  // Press SPACE to start shot if cue ball isn't moving
  if (key == GLFW_KEY_SPACE &&
      (stage_ == GameStage::ViewShot || stage_ == GameStage::PlaceCueBall) &&
      !balls_[kCueBallIndex]->IsMoving())
    HitCueBall();

  // Press L to hide/unhide lamp
  if (key == GLFW_KEY_L) render_lamp_ = !render_lamp_;

  // Press V to toggle LookAround mode
  if (key == GLFW_KEY_V) LookAround();
}

void Game::OnKeyRelease(int key, int mods) {}

void Game::OnMouseMove(int mouse_x, int mouse_y, int delta_x, int delta_y) {
  if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT)) {
    if (stage_ == GameStage::HitCueBall) {
      // Move cue and camera left and right
      camera_->RotateOy((float)-delta_x * kSensitivity);
      cue_->Rotate((float)-delta_x * kSensitivity);
    }
    if (stage_ == GameStage::LookAround) {
      // Move camera left/right/up/down
      camera_->RotateOy((float)-delta_x * kSensitivity);
      camera_->RotateOx((float)-delta_y * kSensitivity);
    }
  }
}

void Game::OnMouseBtnPress(int mouse_x, int mouse_y, int button, int mods) {}

void Game::OnMouseBtnRelease(int mouse_x, int mouse_y, int button, int mods) {
  if (button == 1 &&  // GLFW_MOUSE_BUTTON_LEFT not working?
      cue_offset_ >= 0 && stage_ == GameStage::HitCueBall) {
    // Release LEFT_MOUSE_BUTTON to hit cue ball
    balls_[kCueBallIndex]->CueHit(cue_->GetDirection(), -cue_offset_);
    ViewShot();
  }
}

void Game::OnMouseScroll(int mouse_x, int mouse_y, int offset_x, int offset_y) {
}

void Game::OnWindowResize(int width, int height) {}

#pragma endregion

#pragma region GAME STAGES

void Game::ViewShot() {
  prev_stage_ = stage_;
  stage_ = GameStage::ViewShot;
  camera_->TopDown();
}

void Game::PlaceCueBall() {
  prev_stage_ = stage_;
  stage_ = GameStage::PlaceCueBall;
  camera_->TopDown();
}

void Game::HitCueBall() {
  prev_stage_ = stage_;
  stage_ = GameStage::HitCueBall;
  glm::vec3 default_target = glm::vec3(0);  // look at center of table
  glm::vec3 ball_center = balls_[kCueBallIndex]->GetCenter();
  camera_->ThirdPerson(ball_center, default_target);

  // Reposition cue
  cue_->Reposition(ball_center);
  if (ball_center.x > 0)
    cue_->Rotate((float)(-camera_->GetOxAngle() + M_PI));
  else
    cue_->Rotate((float)(camera_->GetOxAngle() - M_PI));
  cue_offset_ = 0;
}

void Game::LookAround() {
  if (stage_ != GameStage::LookAround) {
    prev_stage_ = stage_;
    stage_ = GameStage::LookAround;
    camera_->FirstPerson();
  } else {
    switch (prev_stage_) {
      case GameStage::HitCueBall:
        HitCueBall();
        break;
      case GameStage::ViewShot:
        ViewShot();
        break;
      case GameStage::PlaceCueBall:
        PlaceCueBall();
        break;
      default:
        break;
    }
  }
}

#pragma endregion
}  // namespace pool
