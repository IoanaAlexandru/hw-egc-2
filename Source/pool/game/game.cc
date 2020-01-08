#include "pool/game/game.h"

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
                Game::kRed = glm::vec3(0.86, 0.20, 0.21),
                Game::kYellow = glm::vec3(0.96, 0.76, 0.05);
const float Game::kMovementSpeed = 2.0f, Game::kSensitivity = 0.001f;
const float Game::kMaxCueOffset = 2.0f;
const int Game::kBlackBallIndex = 5, Game::kCueBallIndex = 0;
const glm::mat4 Game::kTableModelMatrix =
    glm::scale(glm::mat4(1), glm::vec3(2.0f));
const std::string Game::kPoolShaderName = "PoolShader";
#pragma endregion

Game::Game() {}

Game::~Game() {}

void Game::Init() {
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
        new Ball("middle_left_pocket", corner, kPocketRadius, pure_black));
    pockets_.push_back(
        new Ball("lower_left_pocket",
                 corner + glm::vec3(kPocketRadius, 0, kTableLength / 2),
                 kPocketRadius, pure_black));
    pockets_.push_back(
        new Ball("upper_left_pocket",
                 corner + glm::vec3(kPocketRadius, 0, -kTableLength / 2),
                 kPocketRadius, pure_black));
    corner = glm::vec3(kTableWidth / 2 + kPocketRadius, 0, 0);
    pockets_.push_back(
        new Ball("middle_right_pocket", corner, kPocketRadius, pure_black));
    pockets_.push_back(
        new Ball("lower_right_pocket",
                 corner + glm::vec3(-kPocketRadius, 0, kTableLength / 2),
                 kPocketRadius, pure_black));
    pockets_.push_back(
        new Ball("upper_right_pocket",
                 corner + glm::vec3(-kPocketRadius, 0, -kTableLength / 2),
                 kPocketRadius, pure_black));
  }

  // Balls
  {
    glm::vec3 ball_center = glm::vec3(0, kBallRadius, kTableLength / 4);
    glm::vec3 ball_color = glm::vec3(0.9, 0.9, 0.9);
    std::string ball_name = "cue_ball";
    balls_.push_back(new Ball(ball_name, ball_center, kBallRadius, ball_color));

    ball_center = glm::vec3(0, kBallRadius, -kTableLength / 5);
    ball_color = kRed;
    float row_offset = std::sqrt(pow(2 * kBallRadius, 2) - pow(kBallRadius, 2));
    int ball_count = 0;
    for (auto i = 0; i < 5; i++) {
      for (auto j = 0; j < i + 1; j++) {
        if (i == 2 && j == 1) {
          ball_color = glm::vec3(0.2, 0.2, 0.2);
          ball_name = "black_ball";
        } else {
          ball_color = (ball_count + i) % 2 == 0 ? kRed : kYellow;
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
    render_lamp_ = false;
  }

  // Shader
  {
    Shader *shader = new Shader(kPoolShaderName.c_str());
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

  // Init game
  {
    camera_ = new Camera(window->props.aspectRatio);

    StartGame();
    Break();
  }
}

#pragma region GAME CONTROL

void Game::StartGame() {
  // Only print help once for each stage
  print_help_.emplace(GameStage::HIT_CUE_BALL, true);
  print_help_.emplace(GameStage::LOOK_AROUND, true);
  print_help_.emplace(GameStage::PLACE_CUE_BALL, true);

  std::cout << std::endl << "Welcome to 8-ball-pool!" << std::endl;
  player_one_ = GetPlayerName("Player1");
  player_two_ = GetPlayerName("Player2");

  current_player_ = &player_one_;
  press_space_to_continue_ = true;
  end_ = false;
}

Player Game::GetPlayerName(std::string default) {
  std::string name;
  std::cout << "Please enter name for " << default
            << " (press Enter for default): ";
  std::getline(std::cin, name);
  if (name.empty()) {
    name = default;
  }
  return Player(name);
}

void Game::EndGame() {
  LookAround();
  player_one_.PrintStats();
  player_two_.PrintStats();
}

void Game::TogglePlayer() {
  if (current_player_ == &player_one_)
    current_player_ = &player_two_;
  else
    current_player_ = &player_one_;
  std::cout << std::endl
            << current_player_->GetName()
            << "'s turn. Press SPACE to start your shot." << std::endl;

  current_player_->Reset();
  press_space_to_continue_ = true;
}

void Game::Help() {
  std::cout
      << std::endl
      << "=============================== HELP ==============================="
      << std::endl
      << "* General controls: Press V at any time to toggle LookAround mode"
      << std::endl
      << "and explore the world freely." << std::endl
      << "* Look around controls: Look around using the mouse and the WASDEQ"
      << std::endl
      << "keys by pressing RIGHT_MOUSE_BUTTON. Press V again to go back to the"
      << std::endl
      << "previous mode." << std::endl
      << "* Lamp controls: Press CTRL + the directional keys to move the lamp."
      << std::endl
      << "* Break controls: Place the cue ball using the WASD keys, then press"
      << std::endl
      << "SPACE to start your shot." << std::endl
      << "* Place cue ball controls: You can place the cue ball anywhere using"
      << std::endl
      << "the WASD keys." << std::endl
      << "* Hit cue ball controls: Press RIGHT_MOUSE_BUTTON and move mouse to"
      << std::endl
      << "position shot horizontally.Press LEFT_MOUSE_BUTTON to start moving"
      << std::endl
      << "cue, release to hit(the further the cue is from the ball, the"
      << std::endl
      << "stronger the shot)." << std::endl
      << "===================================================================="
      << std::endl;
  ;
}

#pragma endregion

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
    bool none_moving = true;
    for (auto ball : balls_) {
      if (!ball->IsPotted()) {
        glm::vec3 center = ball->GetCenter();
        PotStatus pot_status = PotStatus::OK;

        // Pocket collisions
        for (auto pocket : pockets_) {
          if (Ball::CheckCollision(ball, pocket, delta_time_seconds)) {
            ball->SetPotted(true);
            pot_status = current_player_->PotBall(ball->GetColor());
          }
        }

        // Rail collisions
        float down = center.z + kBallRadius - kTableLength / 2;
        float up = center.z - kBallRadius + kTableLength / 2;
        float right = center.x + kBallRadius - kTableWidth / 2;
        float left = center.x - kBallRadius + kTableWidth / 2;
        bool middle = abs(center.z) + kBallRadius < kPocketRadius;

        if (down > 0) {
          ball->ReflectZ(down);
          current_player_->HitRail();
        } else if (up < 0) {
          ball->ReflectZ(up);
          current_player_->HitRail();
        } else if (right > 0 && !middle) {
          ball->ReflectX(right);
          current_player_->HitRail();
        } else if (left < 0 && !middle) {
          ball->ReflectX(left);
          current_player_->HitRail();
        } else if ((right > 0 || left < 0) && middle) {
          ball->SetPotted(true);
          pot_status = current_player_->PotBall(ball->GetColor());
        }

        // Update players and check for potting faults
        if (ball->IsPotted()) {
          if (player_one_.GetColor() == kRed) player_two_.SetColor(kYellow);
          if (player_one_.GetColor() == kYellow) player_two_.SetColor(kRed);
          if (player_two_.GetColor() == kRed) player_one_.SetColor(kYellow);
          if (player_two_.GetColor() == kYellow) player_one_.SetColor(kRed);

          if (player_one_.GetColor() == ball->GetColor())
            player_one_.OwnBallPotted();
          if (player_two_.GetColor() == ball->GetColor())
            player_two_.OwnBallPotted();

          switch (pot_status) {
            case PotStatus::FAULT_CUE_BALL:
              std::cout << "Fault! Potted the cue ball." << std::endl;
              break;
            case PotStatus::FAULT_OPPONENT:
              std::cout << "Fault! Potted opponent ball." << std::endl;
              break;
            case PotStatus::LOSS:
              std::cout << current_player_->GetName()
                        << " lost by potting the black ball too early."
                        << std::endl;
              EndGame();
              break;
            case PotStatus::WIN:
              end_ = true;
              break;
            default:
              break;
          }
        }

        // Ball collisions
        if (ball->IsMoving()) {
          none_moving = false;
          for (auto another_ball : balls_) {
            if (another_ball == ball || another_ball->IsPotted()) continue;
            if (Ball::CheckCollision(ball, another_ball, delta_time_seconds)) {
              Ball::Bounce(ball, another_ball);
              if (ball == balls_[kCueBallIndex]) {
                HitStatus hit_status =
                    current_player_->HitBall(another_ball->GetColor());
                if (hit_status == HitStatus::FAULT_OPPONENT ||
                    hit_status == HitStatus::FAULT_BLACK)
                  std::cout << "Fault! You have to hit your own ball first."
                            << std::endl;
              }
              break;
            }
          }
        }
      }
    }

    // Check status and toggle player
    if (!press_space_to_continue_ && stage_ == GameStage::VIEW_SHOT &&
        none_moving) {
      if (current_player_->Fault()) {
        TogglePlayer();
        PlaceCueBall();
      } else if (current_player_->NoneHit()) {
        current_player_->AddFault();
        std::cout << "Fault! No balls were hit." << std::endl;
        TogglePlayer();
        PlaceCueBall();
      } else if (current_player_->NonePotted()) {
        TogglePlayer();
      }
    }

    // Check for endgame
    if (end_ && none_moving) {
      if (balls_[kCueBallIndex]->IsPotted())
        std::cout << current_player_->GetName()
                  << " lost by potting the cue ball with the black ball."
                  << std::endl;
      else
        std::cout << current_player_->GetName() << " won." << std::endl;
      EndGame();
      end_ = false;
    }

    // Continue if owned ball was potted
    if (!current_player_->NonePotted() && none_moving) {
      press_space_to_continue_ = true;
      current_player_->Reset();
    }

    // Bring cue ball back if accidentally potted when placing
    if ((stage_ == GameStage::PLACE_CUE_BALL || stage_ == GameStage::BREAK) &&
        balls_[kCueBallIndex]->IsPotted())
      balls_[kCueBallIndex]->Reset();
  }

  // Render objects
  {
    // Table
    RenderSimpleMesh(table_, shaders[kPoolShaderName], kTableModelMatrix, 0,
                     table_properties_, kTableColor);
    RenderSimpleMesh(table_metal_, shaders[kPoolShaderName], kTableModelMatrix,
                     0, metal_properties_, kMetalColor);
    RenderSimpleMesh(table_bed_, shaders[kPoolShaderName], kTableModelMatrix, 0,
                     velvet_properties_, kTableBedColor);

    // Balls
    for (auto ball : balls_) {
      ball->Update(delta_time_seconds);
      RenderSimpleMesh((Mesh *)ball, shaders[kPoolShaderName],
                       ball->GetModelMatrix(), 0, ball_properties_,
                       ball->GetColor());
    }

    // Cue
    // Change cue color to match player if colors were assigned
    glm::vec3 cue_color = current_player_->GetColor() == glm::vec3(1)
                              ? cue_->GetColor()
                              : 0.5f * current_player_->GetColor();
    if (stage_ == GameStage::HIT_CUE_BALL)
      RenderSimpleMesh((Mesh *)cue_, shaders[kPoolShaderName],
                       cue_->GetModelMatrix(), cue_offset_, cue_properties_,
                       cue_color);

    // Lamp (light source for shader)
    if (render_lamp_)
      RenderSimpleMesh(lamp_, shaders[kPoolShaderName],
                       glm::translate(glm::mat4(1), lamp_position_), 0,
                       metal_properties_, kMetalColor);
  }
}

void Game::FrameEnd() {
  //DrawCoordinatSystem(camera_->GetViewMatrix(), camera_->GetProjectionMatrix());
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
    } else if (stage_ == GameStage::PLACE_CUE_BALL ||
               stage_ == GameStage::BREAK) {
      // Move cue ball using W, A, S, D
      Ball *cue_ball = balls_[kCueBallIndex];
      glm::vec3 pos = cue_ball->GetCenter();

      float upper_limit = stage_ == GameStage::BREAK
                              ? kTableLength / 4
                              : -kTableLength / 2 + kBallRadius;

      if (window->KeyHold(GLFW_KEY_W) && pos.z > upper_limit)
        cue_ball->MoveUp(delta_time);
      if (window->KeyHold(GLFW_KEY_A) && pos.x > -kTableWidth / 2 + kBallRadius)
        cue_ball->MoveLeft(delta_time);
      if (window->KeyHold(GLFW_KEY_S) && pos.z < kTableLength / 2 - kBallRadius)
        cue_ball->MoveDown(delta_time);
      if (window->KeyHold(GLFW_KEY_D) && pos.x < kTableWidth / 2 - kBallRadius)
        cue_ball->MoveRight(delta_time);

      // Reverse movement if balls are touching
      for (auto ball : balls_) {
        if (cue_ball == ball || ball->IsPotted()) continue;
        if (Ball::AreTouching(cue_ball, ball)) {
          if (window->KeyHold(GLFW_KEY_W)) cue_ball->MoveDown(delta_time);
          if (window->KeyHold(GLFW_KEY_A)) cue_ball->MoveRight(delta_time);
          if (window->KeyHold(GLFW_KEY_S)) cue_ball->MoveUp(delta_time);
          if (window->KeyHold(GLFW_KEY_D)) cue_ball->MoveLeft(delta_time);
        }
      }
    }
  } else if (stage_ == GameStage::LOOK_AROUND) {
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
    if (cue_offset_ > kMaxCueOffset) cue_offset_ = kMaxCueOffset;
    if (cue_offset_ < 0) cue_offset_ = 0;
    if (cue_offset_ == kMaxCueOffset || cue_offset_ == 0)
      cue_movement_speed_ *= -1;
  }
}

void Game::OnKeyPress(int key, int mods) {
  // Press SPACE to start shot if cue ball isn't moving
  if (key == GLFW_KEY_SPACE && (stage_ != GameStage::HIT_CUE_BALL) &&
      !balls_[kCueBallIndex]->IsMoving() && press_space_to_continue_)
    HitCueBall();

  // Press L to hide/unhide lamp
  if (key == GLFW_KEY_L) render_lamp_ = !render_lamp_;

  // Press V to toggle LookAround mode
  if (key == GLFW_KEY_V) LookAround();

  // Press H to print help
  if (key == GLFW_KEY_H) Help();
}

void Game::OnKeyRelease(int key, int mods) {}

void Game::OnMouseMove(int mouse_x, int mouse_y, int delta_x, int delta_y) {
  if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT)) {
    if (stage_ == GameStage::HIT_CUE_BALL) {
      // Move cue and camera left and right
      camera_->RotateOy((float)-delta_x * kSensitivity);
      cue_->Rotate((float)-delta_x * kSensitivity);
    }
    if (stage_ == GameStage::LOOK_AROUND) {
      // Move camera left/right/up/down
      camera_->RotateOy((float)-delta_x * kSensitivity);
      camera_->RotateOx((float)-delta_y * kSensitivity);
    }
  }
}

void Game::OnMouseBtnPress(int mouse_x, int mouse_y, int button, int mods) {}

void Game::OnMouseBtnRelease(int mouse_x, int mouse_y, int button, int mods) {
  if (button == 1 &&  // GLFW_MOUSE_BUTTON_LEFT not working?
      cue_offset_ >= 0 && stage_ == GameStage::HIT_CUE_BALL) {
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

void Game::Break() {
  std::cout << std::endl
            << current_player_->GetName() << " is breaking. Good luck!"
            << std::endl;
  std::cout
      << "Place the cue ball using the WASD keys, then press SPACE to start"
      << std::endl
      << "your shot. You can press H at any time to view detailed controls."
      << std::endl;

  prev_stage_ = stage_ = GameStage::BREAK;
  camera_->TopDown();
}

void Game::ViewShot() {
  prev_stage_ = stage_;
  stage_ = GameStage::VIEW_SHOT;
  camera_->TopDown();
}

void Game::PlaceCueBall() {
  if (print_help_[GameStage::PLACE_CUE_BALL]) {
    std::cout << std::endl
              << "You can place the cue ball anywhere using the WASD keys."
              << std::endl;
    print_help_[GameStage::PLACE_CUE_BALL] = false;
  }

  prev_stage_ = stage_;
  stage_ = GameStage::PLACE_CUE_BALL;
  balls_[kCueBallIndex]->Reset();
  camera_->TopDown();
}

Ball *Game::GetClosestOwnedBall(glm::vec3 point) {
  float smallest_dist = 1000;
  Ball *closest_ball = NULL;
  for (auto ball : balls_) {
    if (ball->IsPotted() || ball == balls_[kCueBallIndex]) continue;
    if (ball->GetColor() == current_player_->GetColor() ||
        current_player_->GetColor() == glm::vec3(1)) {
      float dist = glm::distance(point, ball->GetCenter());
      if (dist < smallest_dist) {
        smallest_dist = dist;
        closest_ball = ball;
      }
    }
  }
  if (smallest_dist == 1000) return balls_[kBlackBallIndex];
  return closest_ball;
}

void Game::HitCueBall() {
  if (print_help_[GameStage::HIT_CUE_BALL]) {
    std::cout
        << std::endl
        << "Press RIGHT_MOUSE_BUTTON and move mouse to position shot"
        << std::endl
        << "horizontally.Press LEFT_MOUSE_BUTTON to start moving cue, release"
        << std::endl
        << "to hit(the further the cue is from the ball, the stronger the "
        << std::endl
        << "shot)." << std::endl;
    print_help_[GameStage::HIT_CUE_BALL] = false;
  }

  press_space_to_continue_ = false;

  prev_stage_ = stage_;
  stage_ = GameStage::HIT_CUE_BALL;
  glm::vec3 ball_center = balls_[kCueBallIndex]->GetCenter();
  glm::vec3 default_target = GetClosestOwnedBall(ball_center)->GetCenter();
  camera_->ThirdPerson(ball_center, default_target);

  // Reposition cue
  cue_->Reposition(ball_center);
  if (ball_center.x < 0)
    cue_->Rotate((float)(-camera_->GetOxAngle() + M_PI));
  else
    cue_->Rotate((float)(-camera_->GetOxAngle() + M_PI));
  cue_offset_ = 0;
}

void Game::LookAround() {
  if (print_help_[GameStage::LOOK_AROUND]) {
    std::cout << std::endl
              << "Look around using the mouse and the WASDEQ keys" << std::endl
              << "(first-person view) by pressing RIGHT_MOUSE_BUTTON. Press V "
              << std::endl
              << "again to go back to the previous mode." << std::endl;
    print_help_[GameStage::LOOK_AROUND] = false;
  }

  if (stage_ != GameStage::LOOK_AROUND) {
    prev_stage_ = stage_;
    stage_ = GameStage::LOOK_AROUND;
    camera_->FirstPerson();
  } else {
    switch (prev_stage_) {
      case GameStage::HIT_CUE_BALL:
        HitCueBall();
        break;
      case GameStage::VIEW_SHOT:
        ViewShot();
        break;
      case GameStage::PLACE_CUE_BALL:
        PlaceCueBall();
        break;
      case GameStage::BREAK:
        Break();
        break;
      default:
        break;
    }
  }
}

#pragma endregion
}  // namespace pool
