#include "game.h"

#include <iostream>
#include <string>
#include <vector>

#include <Core/Engine.h>
#include <Engine/Component/Camera/Camera.h>

using namespace std;

namespace pool {

const float Game::kTableWidth = 2.16f, Game::kTableLength = 4.26f,
            Game::kBallRadius = 0.07f, Game::kCueLength = 2.6f,
            Game::kPocketRadius = 0.12f, Game::kTableBedBorder = 0.08f;
const glm::vec3 Game::kTableBedColor = glm::vec3(0, 0.5, 0.1),
                Game::kTableColor = glm::vec3(0.3, 0.05, 0.05),
                Game::kTableMetalColor = glm::vec3(0.8, 0.8, 0.8),
                Game::kCueColor = glm::vec3(0.5, 0.15, 0.15),
                Game::kPlayerOneColor = glm::vec3(0.86, 0.20, 0.21),
                Game::kPlayerTwoColor = glm::vec3(0.96, 0.76, 0.05);
const float Game::kMovementSpeed = 2.0f, Game::kMaxCueOffset = 2.0f,
            Game::kSensitivity = 0.001f;
const glm::mat4 Game::kTableModelMatrix =
    glm::scale(glm::mat4(1), glm::vec3(2.0f));

Game::Game() {}

Game::~Game() {}

void Game::Init() {
  {
    camera_ = new Camera(window->props.aspectRatio);
    PlaceCueBall();
  }

  {
    table_ = new Mesh("table");
    table_->LoadMesh(RESOURCE_PATH::MODELS + "Props", "table.obj");

    table_bed_ = new Mesh("table_bed");
    table_bed_->LoadMesh(RESOURCE_PATH::MODELS + "Props", "table_bed.obj");

    table_metal_ = new Mesh("table_metal");
    table_metal_->LoadMesh(RESOURCE_PATH::MODELS + "Props", "table_metal.obj");

    glm::vec3 pure_black = glm::vec3(0, 0, 0);
    glm::vec3 corner = glm::vec3(-kTableWidth / 2 - kPocketRadius, 0, 0);
    pockets_.push_back(
        new Ball("middle_left", corner, kPocketRadius, pure_black));
    pockets_.push_back(new Ball(
        "upper_left", corner + glm::vec3(kPocketRadius, 0, kTableLength / 2),
        kPocketRadius, pure_black));
    pockets_.push_back(new Ball(
        "lower_left", corner + glm::vec3(kPocketRadius, 0, -kTableLength / 2),
        kPocketRadius, pure_black));
    corner = glm::vec3(kTableWidth / 2 + kPocketRadius, 0, 0);
    pockets_.push_back(
        new Ball("middle_right", corner, kPocketRadius, pure_black));
    pockets_.push_back(new Ball(
        "upper_right", corner + glm::vec3(-kPocketRadius, 0, kTableLength / 2),
        kPocketRadius, pure_black));
    pockets_.push_back(new Ball(
        "lower_right", corner + glm::vec3(-kPocketRadius, 0, -kTableLength / 2),
        kPocketRadius, pure_black));
  }

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

  {
    Mesh *mesh = new Mesh("sphere");
    mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "sphere.obj");
    meshes[mesh->GetMeshID()] = mesh;
  }

  // Create a shader program for drawing face polygon with the color of the
  // normal
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
    light_position_ = glm::vec3(0, 1.7, 0);

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
}  // namespace pool

void Game::FrameStart() {
  // clears the color buffer (using the previously set color) and depth buffer
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::ivec2 resolution = window->GetResolution();
  // sets the screen area where to draw
  glViewport(0, 0, resolution.x, resolution.y);
}

void Game::Update(float delta_time_seconds) {
  // Collisions
  {
    for (auto ball : balls_) {
      glm::vec3 center = ball->GetCenter();

      for (auto pocket : pockets_) {
        if (Ball::CheckCollision(ball, pocket, delta_time_seconds))
          ball->SetPotted(true);
      }
      if (ball->IsPotted()) continue;

      if (center.z + kBallRadius > kTableLength / 2 ||
          center.z - kBallRadius < -kTableLength / 2) {
        ball->ReflectZ();
      }
      if (center.x + kBallRadius > kTableWidth / 2 ||
          center.x - kBallRadius < -kTableWidth / 2) {
        if (abs(center.z) + kBallRadius < kPocketRadius)
          ball->SetPotted(true);
        else
          ball->ReflectX();
      }

      if (ball->IsMoving()) {
        for (auto another_ball : balls_) {
          if (another_ball == ball) continue;
          if (Ball::CheckCollision(ball, another_ball, delta_time_seconds)) {
            Ball::Bounce(ball, another_ball);
            break;
          }
        }
      }
    }
  }

  {
    RenderSimpleMesh(table_, shaders["PoolShader"], kTableModelMatrix, 0,
                     table_properties_, kTableColor);
    RenderSimpleMesh(table_metal_, shaders["PoolShader"], kTableModelMatrix, 0,
                     metal_properties_, kTableMetalColor);
    RenderSimpleMesh(table_bed_, shaders["PoolShader"], kTableModelMatrix, 0,
                     velvet_properties_, kTableBedColor);

    for (auto ball : balls_) {
      ball->Update(delta_time_seconds);
      RenderSimpleMesh((Mesh *)ball, shaders["PoolShader"],
                       ball->GetModelMatrix(), 0, ball_properties_,
                       ball->GetColor());
    }

    if (stage_ == GameStage::HitCueBall)
      RenderSimpleMesh((Mesh *)cue_, shaders["PoolShader"],
                       cue_->GetModelMatrix(), cue_offset_, cue_properties_,
                       cue_->GetColor());
  }

  // Render the point light in the scene
  {
    glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, light_position_);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.001f));
    RenderMesh(meshes["sphere"], shaders["Simple"], modelMatrix);
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

  // render an object using the specified shader and the specified position
  glUseProgram(shader->program);

  // Set shader uniforms for light & material properties
  GLint light = glGetUniformLocation(shader->program, "light_position");
  glUniform3fv(light, 1, glm::value_ptr(light_position_));

  GLint shininess = glGetUniformLocation(shader->program, "material_shininess");

  GLint eye = glGetUniformLocation(shader->program, "eye_position");
  glm::vec3 eyePosition = glm::vec3(0, 0, 0);
  glUniform3fv(eye, 1, glm::value_ptr(eyePosition));

  glUniform1ui(shininess, properties.shininess);

  GLint kd = glGetUniformLocation(shader->program, "material_kd");
  glUniform1f(kd, properties.kd);

  GLint ks = glGetUniformLocation(shader->program, "material_ks");
  glUniform1f(ks, properties.ks);

  GLint colorP = glGetUniformLocation(shader->program, "object_color");
  glUniform3fv(colorP, 1, glm::value_ptr(color));

  GLint offset = glGetUniformLocation(shader->program, "z_offset");
  glUniform1f(offset, z_offset);

  // Bind model matrix
  GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
  glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE,
                     glm::value_ptr(model_matrix));

  // Bind view matrix
  glm::mat4 viewMatrix = camera_->GetViewMatrix();
  int loc_view_matrix = glGetUniformLocation(shader->program, "View");
  glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

  // Bind projection matrix
  glm::mat4 projectionMatrix = camera_->GetProjectionMatrix();
  int loc_projection_matrix =
      glGetUniformLocation(shader->program, "Projection");
  glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE,
                     glm::value_ptr(projectionMatrix));

  // Draw the object
  glBindVertexArray(mesh->GetBuffers()->VAO);
  glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()),
                 GL_UNSIGNED_SHORT, 0);
}

void Game::OnInputUpdate(float delta_time, int mods) {
  if (!window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT)) {
    if (mods == GLFW_MOD_CONTROL) {
      glm::vec3 up = glm::vec3(0, 1, 0);
      glm::vec3 right = glm::vec3(1, 0, 0);
      glm::vec3 forward = glm::vec3(0, 0, 1);

      // Control light position using on W, A, S, D, E, Q
      if (window->KeyHold(GLFW_KEY_W))
        light_position_ -= forward * delta_time * kMovementSpeed;
      if (window->KeyHold(GLFW_KEY_A))
        light_position_ -= right * delta_time * kMovementSpeed;
      if (window->KeyHold(GLFW_KEY_S))
        light_position_ += forward * delta_time * kMovementSpeed;
      if (window->KeyHold(GLFW_KEY_D))
        light_position_ += right * delta_time * kMovementSpeed;
      if (window->KeyHold(GLFW_KEY_E))
        light_position_ += up * delta_time * kMovementSpeed;
      if (window->KeyHold(GLFW_KEY_Q))
        light_position_ -= up * delta_time * kMovementSpeed;
    } else if (stage_ == GameStage::PlaceCueBall) {
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
  } else {
    if (window->KeyHold(GLFW_KEY_W)) camera_->TranslateForward(delta_time);
    if (window->KeyHold(GLFW_KEY_A)) camera_->TranslateRight(-delta_time);
    if (window->KeyHold(GLFW_KEY_S)) camera_->TranslateForward(-delta_time);
    if (window->KeyHold(GLFW_KEY_D)) camera_->TranslateRight(delta_time);
    if (window->KeyHold(GLFW_KEY_Q)) camera_->TranslateUpword(-delta_time);
    if (window->KeyHold(GLFW_KEY_E)) camera_->TranslateUpword(delta_time);
  }

  if (window->MouseHold(GLFW_MOUSE_BUTTON_LEFT)) {
    cue_offset_ += cue_movement_speed_ * delta_time;
    if (cue_offset_ >= kMaxCueOffset || cue_offset_ <= 0)
      cue_movement_speed_ *= -1;
  }
}

void Game::OnKeyPress(int key, int mods) {
  if (key == GLFW_KEY_SPACE &&
      (stage_ == GameStage::ViewShot || stage_ == GameStage::PlaceCueBall) &&
      !balls_[kCueBallIndex]->IsMoving())
    HitCueBall();
}

void Game::OnKeyRelease(int key, int mods) {
  // add key release event
}

void Game::OnMouseMove(int mouse_x, int mouse_y, int delta_x, int delta_y) {
  if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT) &&
      stage_ == GameStage::HitCueBall) {
    camera_->RotateOy((float)-delta_x * kSensitivity);
    cue_->Rotate((float)-delta_x * kSensitivity);
  }
}

void Game::OnMouseBtnPress(int mouse_x, int mouse_y, int button, int mods) {
  // add mouse button press event
}

void Game::OnMouseBtnRelease(int mouse_x, int mouse_y, int button, int mods) {
  if (button == 1 &&  // GLFW_MOUSE_BUTTON_LEFT not working?
      cue_offset_ >= 0 && stage_ == GameStage::HitCueBall) {
    balls_[kCueBallIndex]->CueHit(cue_->GetDirection(), -cue_offset_);
    ViewShot();
  }
}

void Game::OnMouseScroll(int mouse_x, int mouse_y, int offset_x, int offset_y) {
}

void Game::OnWindowResize(int width, int height) {}

void Game::ViewShot() {
  stage_ = GameStage::ViewShot;
  camera_->TopDown();
}

void Game::PlaceCueBall() {
  stage_ = GameStage::PlaceCueBall;
  camera_->TopDown();
}

void Game::HitCueBall() {
  stage_ = GameStage::HitCueBall;
  glm::vec3 default_target = glm::vec3(0);
  glm::vec3 ball_center = balls_[kCueBallIndex]->GetCenter();
  camera_->ThirdPerson(ball_center, default_target);

  cue_ = new Cue("cue", ball_center, kCueLength, kCueColor);

  if (ball_center.x > 0)
    cue_->Rotate((float)(-camera_->GetOxAngle() + M_PI));
  else
    cue_->Rotate((float)(camera_->GetOxAngle() - M_PI));
  cue_offset_ = 0;
  cue_movement_speed_ = kMovementSpeed;
}
}  // namespace pool
