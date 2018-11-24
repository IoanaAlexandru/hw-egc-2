#include "game.h"

#include <iostream>
#include <string>
#include <vector>

#include <Core/Engine.h>

using namespace std;

namespace pool {

const float Game::kTableWidth = 2.4f, Game::kTableHeight = 0.2f,
            Game::kTableLength = 4.4f, Game::kTableThickness = 0.2f,
            Game::kBallRadius = 0.07f, Game::kPocketRadius = 0.15f;
const glm::vec3 Game::kTableSlateColor = glm::vec3(0, 0.5, 0.1),
                Game::kTableRailColor = glm::vec3(0.4, 0.05, 0.05),
                Game::kPlayerOneColor = glm::vec3(0.86, 0.20, 0.21),
                Game::kPlayerTwoColor = glm::vec3(0.96, 0.76, 0.05);

Game::Game() {}

Game::~Game() {}

void Game::Init() {
  {
    TopDownView();
    table = new Table("PoolTable", glm::vec3(0, 0, 0), kTableWidth,
                      kTableLength, kTableHeight, kTableThickness,
                      kTableSlateColor, kTableRailColor);

    glm::vec3 pure_black = glm::vec3(0, 0, 0);
    glm::vec3 corner = glm::vec3(-kTableWidth / 2 + kTableThickness, 0, 0);
    pockets.push_back(new Ball("pocket1", corner, kPocketRadius, pure_black));
    pockets.push_back(new Ball(
        "pocket2",
        corner + glm::vec3(0, 0, kTableLength / 2 - kTableThickness),
        kPocketRadius, pure_black));
    pockets.push_back(new Ball(
        "pocket3",
        corner + glm::vec3(0, 0, -kTableLength / 2 + kTableThickness),
        kPocketRadius, pure_black));
    corner = glm::vec3(kTableWidth / 2 - kTableThickness, 0, 0);
    pockets.push_back(new Ball("pocket4", corner, kPocketRadius, pure_black));
    pockets.push_back(new Ball(
        "pocket5",
        corner + glm::vec3(0, 0, kTableLength / 2 - kTableThickness),
        kPocketRadius, pure_black));
    pockets.push_back(new Ball(
        "pocket6",
        corner + glm::vec3(0, 0, -kTableLength / 2 + kTableThickness),
        kPocketRadius, pure_black));

  }

  {
    glm::vec3 ball_center = glm::vec3(0, kBallRadius, -kTableLength / 5);
    glm::vec3 ball_color = kPlayerOneColor;
    std::string ball_name;
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

        balls.push_back(new Ball(
            ball_name, ball_center + glm::vec3(2 * kBallRadius * j, 0, 0),
            kBallRadius, ball_color));
      }

      ball_center.x -= kBallRadius;
      ball_center.z -= row_offset;
    }

    ball_center =
        glm::vec3(0, kBallRadius, (kTableLength - 2 * kTableThickness) / 4);
    ball_color = glm::vec3(0.9, 0.9, 0.9);
    ball_name = "cue_ball";
    balls.push_back(new Ball(ball_name, ball_center, kBallRadius, ball_color));
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
    lightPosition = glm::vec3(0, 1.2, 0);
    materialShininess = 80;
    materialKd = 0.9f;
    materialKs = 1.5f;
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

void Game::Update(float deltaTimeSeconds) {
  {
    RenderMesh(table, shaders["VertexColor"], glm::mat4(1));
    for (auto ball : balls) {
      RenderSimpleMesh((Mesh *)ball, shaders["PoolShader"], ball->model_matrix_,
                       ball->color_);
    }
    for (auto pocket : pockets) {
      RenderSimpleMesh((Mesh *)pocket, shaders["PoolShader"],
                       pocket->model_matrix_, pocket->color_);
    }
  }

  // Render the point light in the scene
  {
    glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, lightPosition);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.001f));
    RenderMesh(meshes["sphere"], shaders["Simple"], modelMatrix);
  }
}

void Game::FrameEnd() { DrawCoordinatSystem(); }

void Game::RenderSimpleMesh(Mesh *mesh, Shader *shader,
                            const glm::mat4 &modelMatrix,
                            const glm::vec3 &color) {
  if (!mesh || !shader || !shader->GetProgramID()) return;

  // render an object using the specified shader and the specified position
  glUseProgram(shader->program);

  // Set shader uniforms for light & material properties
  // TODO: Set light position uniform
  GLint light = glGetUniformLocation(shader->program, "light_position");
  glUniform3fv(light, 1, glm::value_ptr(lightPosition));

  GLint shininess = glGetUniformLocation(shader->program, "material_shininess");

  // TODO: Set eye position (camera position) uniform
  GLint eye = glGetUniformLocation(shader->program, "eye_position");
  glm::vec3 eyePosition = GetSceneCamera()->transform->GetWorldPosition();
  glUniform3fv(eye, 1, glm::value_ptr(eyePosition));

  // TODO: Set material property uniforms (shininess, kd, ks, object color)
  glUniform1ui(shininess, materialShininess);

  GLint kd = glGetUniformLocation(shader->program, "material_kd");
  glUniform1f(kd, materialKd);

  GLint ks = glGetUniformLocation(shader->program, "material_ks");
  glUniform1f(ks, materialKs);

  GLint colorP = glGetUniformLocation(shader->program, "object_color");
  glUniform3fv(colorP, 1, glm::value_ptr(color));

  // Bind model matrix
  GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
  glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE,
                     glm::value_ptr(modelMatrix));

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

// Documentation for the input functions can be found in:
// "/Source/Core/Window/InputController.h" or
// https://github.com/UPB-Graphics/Framework-EGC/blob/master/Source/Core/Window/InputController.h

void Game::OnInputUpdate(float deltaTime, int mods) {
  float speed = 2;

  if (!window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT)) {
    glm::vec3 up = glm::vec3(0, 1, 0);
    glm::vec3 right = GetSceneCamera()->transform->GetLocalOXVector();
    glm::vec3 forward = GetSceneCamera()->transform->GetLocalOZVector();
    forward = glm::normalize(glm::vec3(forward.x, 0, forward.z));

    // Control light position using on W, A, S, D, E, Q
    if (window->KeyHold(GLFW_KEY_W))
      lightPosition -= forward * deltaTime * speed;
    if (window->KeyHold(GLFW_KEY_A)) lightPosition -= right * deltaTime * speed;
    if (window->KeyHold(GLFW_KEY_S))
      lightPosition += forward * deltaTime * speed;
    if (window->KeyHold(GLFW_KEY_D)) lightPosition += right * deltaTime * speed;
    if (window->KeyHold(GLFW_KEY_E)) lightPosition += up * deltaTime * speed;
    if (window->KeyHold(GLFW_KEY_Q)) lightPosition -= up * deltaTime * speed;
  }
}

void Game::OnKeyPress(int key, int mods) {
  // add key press event
}

void Game::OnKeyRelease(int key, int mods) {
  // add key release event
}

void Game::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) {
  // add mouse move event
}

void Game::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) {
  // add mouse button press event
}

void Game::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) {
  // add mouse button release event
}

void Game::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) {}

void Game::OnWindowResize(int width, int height) {}

void Game::TopDownView() {
  GetSceneCamera()->SetPosition(glm::vec3(0, 4, 0));
  GetSceneCamera()->RotateOX(-750);
  GetSceneCamera()->Update();
}
}  // namespace pool
