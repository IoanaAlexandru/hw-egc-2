#include "game.h"

#include <iostream>
#include <string>
#include <vector>

#include <Core/Engine.h>
#include <Engine/Component/Camera/Camera.h>

using namespace std;

namespace pool {

const float Game::kTableWidth = 2.4f, Game::kTableHeight = 0.2f,
            Game::kTableLength = 4.4f, Game::kTableThickness = 0.2f,
            Game::kBallRadius = 0.07f, Game::kPocketRadius = 0.15f,
            Game::kCueLength = 2.5f;
const glm::vec3 Game::kTableSlateColor = glm::vec3(0, 0.5, 0.1),
                Game::kTableRailColor = glm::vec3(0.4, 0.05, 0.05),
                Game::kCueColor = glm::vec3(0.5, 0.15, 0.15),
                Game::kPlayerOneColor = glm::vec3(0.86, 0.20, 0.21),
                Game::kPlayerTwoColor = glm::vec3(0.96, 0.76, 0.05);
const float Game::kMovementSpeed = 2.0f, Game::kCueBallViewDistance = 0.8f,
            Game::kCueBallViewHeight = 0.3f, Game::kMaxCueOffset = 2.0f;

Game::Game() {}

Game::~Game() {}

void Game::Init() {
  {
    GetSceneCamera()->type = EngineComponents::CameraType::ThirdPerson;
    TopDownView();
  }

  {
    table_ = new Table("PoolTable", glm::vec3(0, 0, 0), kTableWidth,
                       kTableLength, kTableHeight, kTableThickness,
                       kTableSlateColor, kTableRailColor);

    glm::vec3 pure_black = glm::vec3(0, 0, 0);
    glm::vec3 corner = glm::vec3(-kTableWidth / 2 + kTableThickness, 0, 0);
    pockets_.push_back(new Ball("pocket1", corner, kPocketRadius, pure_black));
    pockets_.push_back(new Ball(
        "pocket2", corner + glm::vec3(0, 0, kTableLength / 2 - kTableThickness),
        kPocketRadius, pure_black));
    pockets_.push_back(
        new Ball("pocket3",
                 corner + glm::vec3(0, 0, -kTableLength / 2 + kTableThickness),
                 kPocketRadius, pure_black));
    corner = glm::vec3(kTableWidth / 2 - kTableThickness, 0, 0);
    pockets_.push_back(new Ball("pocket4", corner, kPocketRadius, pure_black));
    pockets_.push_back(new Ball(
        "pocket5", corner + glm::vec3(0, 0, kTableLength / 2 - kTableThickness),
        kPocketRadius, pure_black));
    pockets_.push_back(
        new Ball("pocket6",
                 corner + glm::vec3(0, 0, -kTableLength / 2 + kTableThickness),
                 kPocketRadius, pure_black));
  }

  {
    glm::vec3 ball_center =
        glm::vec3(0, kBallRadius, (kTableLength - 2 * kTableThickness) / 4);
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
    Mesh *mesh = new Mesh("box");
    mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "box.obj");
    meshes[mesh->GetMeshID()] = mesh;
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
    light_position_ = glm::vec3(0, 1.2, 0);
    ball_shininess_ = 80;
    ball_kd_ = 0.9f;
    ball_ks_ = 1.5f;
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
    if (GetSceneCamera()->type == EngineComponents::CameraType::FirstPerson) {
      bool none_moving = true;
      for (auto ball : balls_) {
        glm::vec3 center = ball->GetCenter();

        if (center.z + kBallRadius >= kTableLength / 2 - kTableThickness ||
            center.z - kBallRadius <= -kTableLength / 2 + kTableThickness)
          ball->ReflectZ();
        if (center.x + kBallRadius >= kTableWidth / 2 - kTableThickness ||
            center.x - kBallRadius <= -kTableWidth / 2 + kTableThickness)
          ball->ReflectX();

        if (ball->IsMoving()) {
          none_moving = false;

          for (auto another_ball : balls_) {
            if (another_ball == ball) continue;
            if (Ball::DynamicStaticCollision(ball, another_ball)) {
              Ball::Bounce(ball, another_ball);
              break;
            }
          }
        }
      }

      if (none_moving && !place_cue_ball_)
        ThirdPersonView();  // start next shot
    }
  }

  {
    RenderMesh(table_, shaders["VertexColor"], glm::mat4(1));
    for (auto ball : balls_) {
      ball->Update(delta_time_seconds);
      RenderSimpleMesh((Mesh *)ball, shaders["PoolShader"],
                       ball->GetModelMatrix(), 0, ball->GetColor());
    }
    for (auto pocket : pockets_) {
      RenderSimpleMesh((Mesh *)pocket, shaders["PoolShader"],
                       pocket->GetModelMatrix(), 0, pocket->GetColor());
    }

    if (GetSceneCamera()->type == EngineComponents::CameraType::ThirdPerson)
      RenderSimpleMesh((Mesh *)cue_, shaders["PoolShader"],
                       cue_->GetModelMatrix(), cue_offset_, cue_->GetColor());
  }

  // Render the point light in the scene
  {
    glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, light_position_);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.001f));
    RenderMesh(meshes["sphere"], shaders["Simple"], modelMatrix);
  }
}

void Game::FrameEnd() { DrawCoordinatSystem(); }

void Game::RenderSimpleMesh(Mesh *mesh, Shader *shader,
                            const glm::mat4 &model_matrix, float z_offset,
                            const glm::vec3 &color) {
  if (!mesh || !shader || !shader->GetProgramID()) return;

  // render an object using the specified shader and the specified position
  glUseProgram(shader->program);

  // Set shader uniforms for light & material properties
  GLint light = glGetUniformLocation(shader->program, "light_position");
  glUniform3fv(light, 1, glm::value_ptr(light_position_));

  GLint shininess = glGetUniformLocation(shader->program, "material_shininess");

  GLint eye = glGetUniformLocation(shader->program, "eye_position");
  glm::vec3 eyePosition = GetSceneCamera()->transform->GetWorldPosition();
  glUniform3fv(eye, 1, glm::value_ptr(eyePosition));

  glUniform1ui(shininess, ball_shininess_);

  GLint kd = glGetUniformLocation(shader->program, "material_kd");
  glUniform1f(kd, ball_kd_);

  GLint ks = glGetUniformLocation(shader->program, "material_ks");
  glUniform1f(ks, ball_ks_);

  GLint colorP = glGetUniformLocation(shader->program, "object_color");
  glUniform3fv(colorP, 1, glm::value_ptr(color));

  GLint offset = glGetUniformLocation(shader->program, "z_offset");
  glUniform1f(offset, z_offset);

  // Bind model matrix
  GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
  glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE,
                     glm::value_ptr(model_matrix));

  // Bind view matrix
  glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
  int loc_view_matrix = glGetUniformLocation(shader->program, "View");
  glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

  // Bind projection matrix
  glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
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
      glm::vec3 right = GetSceneCamera()->transform->GetLocalOXVector();
      glm::vec3 forward = GetSceneCamera()->transform->GetLocalOZVector();
      forward = glm::normalize(glm::vec3(forward.x, 0, forward.z));

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
    } else if (GetSceneCamera()->type ==
               EngineComponents::CameraType::FirstPerson) {
      Ball *cue_ball = balls_[kCueBallIndex];
      glm::vec3 pos = cue_ball->GetCenter();

      if (window->KeyHold(GLFW_KEY_W) &&
          pos.z > kTableLength / 6 + kTableThickness + kBallRadius)
        cue_ball->MoveUp(delta_time);
      if (window->KeyHold(GLFW_KEY_A) &&
          pos.x > -kTableWidth / 2 + kTableThickness + kBallRadius)
        cue_ball->MoveLeft(delta_time);
      if (window->KeyHold(GLFW_KEY_S) &&
          pos.z < kTableLength / 2 - kTableThickness - kBallRadius)
        cue_ball->MoveDown(delta_time);
      if (window->KeyHold(GLFW_KEY_D) &&
          pos.x < kTableWidth / 2 - kTableThickness - kBallRadius)
        cue_ball->MoveRight(delta_time);
    }
  }

  if (window->MouseHold(GLFW_MOUSE_BUTTON_LEFT)) {
    if (cue_offset_ >= kMaxCueOffset || cue_offset_ <= 0)
      cue_movement_speed_ *= -1;
    cue_offset_ += cue_movement_speed_ * delta_time;
  }
}

void Game::OnKeyPress(int key, int mods) {
  if (key == GLFW_KEY_SPACE) place_cue_ball_ = false;
}

void Game::OnKeyRelease(int key, int mods) {
  // add key release event
}

void Game::OnMouseMove(int mouse_x, int mouse_y, int delta_x, int delta_y) {
  if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT) &&
      GetSceneCamera()->type == EngineComponents::CameraType::ThirdPerson) {
    GetSceneCamera()->MoveForward(kCueBallViewDistance);
    GetSceneCamera()->RotateOY((float)-delta_x);
    GetSceneCamera()->RotateOX((float)-delta_y);
    GetSceneCamera()->MoveForward(-kCueBallViewDistance);
    GetSceneCamera()->Update();

    cue_->Rotate((float)-delta_x);
  }
}

void Game::OnMouseBtnPress(int mouse_x, int mouse_y, int button, int mods) {
  // add mouse button press event
}

void Game::OnMouseBtnRelease(int mouse_x, int mouse_y, int button, int mods) {
  if (button == 1 &&  // GLFW_MOUSE_BUTTON_LEFT not working?
      cue_offset_ >= 0 &&
      GetSceneCamera()->type == EngineComponents::CameraType::ThirdPerson) {
    balls_[kCueBallIndex]->CueHit(cue_->GetDirection(), -cue_offset_);
    TopDownView();
  }
}

void Game::OnMouseScroll(int mouse_x, int mouse_y, int offset_x, int offset_y) {
}

void Game::OnWindowResize(int width, int height) {}

void Game::TopDownView() {
  if (GetSceneCamera()->type == EngineComponents::CameraType::ThirdPerson) {
    GetSceneCamera()->type = EngineComponents::CameraType::FirstPerson;
    GetSceneCamera()->SetPosition(glm::vec3(0, 4, 0));
    GetSceneCamera()->RotateOX(-750);
    GetSceneCamera()->Update();
  }
}

glm::vec2 Game::GetViewPoint(glm::vec2 target_pos, glm::vec2 ball_pos) {
  glm::vec2 v = glm::normalize(target_pos - ball_pos);
  return ball_pos - kCueBallViewDistance * v;
}

void Game::ThirdPersonView() {
  if (GetSceneCamera()->type == EngineComponents::CameraType::FirstPerson) {
    glm::vec2 default_target = glm::vec2(0, 0);  // look at center of table
    glm::vec3 ball_center = balls_[kCueBallIndex]->GetCenter();
    glm::vec2 view_point =
        GetViewPoint(default_target, glm::vec2(ball_center.x, ball_center.z));

    GetSceneCamera()->type = EngineComponents::CameraType::ThirdPerson;
    GetSceneCamera()->RotateOX(750);
    GetSceneCamera()->SetPosition(
        glm::vec3(view_point.x, kCueBallViewHeight, view_point.y));
    GetSceneCamera()->RotateOY((view_point.x - default_target.x) * 360);
    GetSceneCamera()->Update();

    place_cue_ball_ = false;

    cue_ = new Cue("cue", ball_center, kCueLength, kCueColor);
    cue_->Rotate((view_point.x - ball_center.x) * 360);
    cue_offset_ = 0;
    cue_movement_speed_ = kMovementSpeed;
  }
}
}  // namespace pool
