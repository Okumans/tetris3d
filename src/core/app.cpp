#include "core/app.hpp"
#include "GLFW/glfw3.h"
#include "core/camera_controller.hpp"
#include "core/shader_manager.hpp"
#include "game/space.hpp"
#include "game/tetris_manager.hpp"
#include "game/tetromino.hpp"
#include "glad/gl.h"
#include "glm/fwd.hpp"
#include "ui/ui_manager.hpp"
#include <print>

void App::render(double delta_time) {

  _handleProcessInput(delta_time);
  m_camera_controller.Update(delta_time);

  if (m_appState.gameStarted) {
    m_game.update(delta_time);
  }

  _updateUIElements();

  m_game.render(delta_time, m_camera);

  const Shader &tetromino_shader =
      ShaderManager::getShader(ShaderType::TETROMINO);
  const Shader &ui_shader = ShaderManager::getShader(ShaderType::UI);

  m_gameUIRenderer.renderHoldPiece(
      m_game.getHold().transform(&Tetromino::getType).value_or(BlockType::None),
      {5.0f, 10.0f, 0.0f}, tetromino_shader, ui_shader, 1.2f);
  m_gameUIRenderer.renderPieceQueue(m_game.getPiecesQueue(),
                                    {5.0f, 30.0f, 0.0f}, 5.0f, tetromino_shader,
                                    ui_shader);

  m_uiManager.render(m_appState.windowWidth, m_appState.windowHeight);
}

App::App(GLFWwindow *window)
    : m_window(window), m_camera(glm::vec3(0.0f, 10.0f, 30.0f)),
      m_camera_controller(m_camera),
      m_gameUIRenderer(m_game.getVAO(), m_uiManager.getVAO(), m_camera) {

  glfwSetWindowUserPointer(m_window, (void *)this);

  glfwSetKeyCallback(m_window, _glfwKeyCallback);
  glfwSetCursorPosCallback(m_window, _glfwMouseMoveCallback);
  glfwSetMouseButtonCallback(m_window, _glfwMouseButtonCallback);
  glfwSetScrollCallback(m_window, _glfwScrollCallback);
  glfwSetFramebufferSizeCallback(m_window, _glfwFramebufferSizeCallback);

  _setupResources();
  _setupUIElements();

  int width, height;
  glfwGetWindowSize(m_window, &width, &height);

  m_camera.UpdateSceneSize(width, height);
  m_camera_controller.SetPreset(CameraPreset::FRONT);
}

App::~App() = default;

void App::_setupResources() {
  ShaderManager::loadShader(ShaderType::UI, UI_VERTEX_SHADER_PATH,
                            UI_FRAGMENT_SHADER_PATH);
  ShaderManager::loadShader(ShaderType::TETROMINO, TETROMINO_VERTEX_SHADER_PATH,
                            TETROMINO_FRAGMENT_SHADER_PATH);

  m_font.loadDefaultFont();
}

void App::_setupUIElements() {
  // Virtual coordinates: Y is 0 to 40.
  m_uiManager.addTextElement("next_label", {3.0f, 4.0f, 0, 0}, "NEXT", m_font,
                             glm::vec4(1.0f), 0.125f);

  m_uiManager.addInteractiveElement("hold_btn", {2.0f, 24.0f, 6.0f, 2.0f},
                                    glm::vec4(0.0f),
                                    [this]() { this->m_game.hold(); });

  m_uiManager.addTextElement("hold_label", {3.0f, 24.5f, 0, 0}, "HOLD", m_font,
                             glm::vec4(1.0f), 0.125f);

  // Score UI
  m_uiManager.addTextElement("score_label", {3.0f, 32.0f, 0, 0}, "SCORE",
                             m_font, glm::vec4(1.0f, 0.8f, 0.0f, 1.0f), 0.1f);
  m_uiManager.addTextElement("score_value", {3.0f, 34.0f, 0, 0}, "0", m_font,
                             glm::vec4(1.0f), 0.15f);

  // Level UI
  m_uiManager.addTextElement("level_label", {3.0f, 36.5f, 0, 0}, "LEVEL",
                             m_font, glm::vec4(0.0f, 0.8f, 1.0f, 1.0f), 0.1f);
  m_uiManager.addTextElement("level_value", {3.0f, 38.5f, 0, 0}, "0", m_font,
                             glm::vec4(1.0f), 0.15f);

  // Start Screen
  m_uiManager.addInteractiveElement(
      "darken_screen", {0.0f, 0.0f, 100, 40.0f}, {0.0f, 0.0f, 0.0f, 0.7f},
      [this]() { this->m_appState.gameStarted = true; });

  m_uiManager.addTextElement("start_message", {0.0f, 20.5f, 0, 0},
                             "press any key to start!", m_font, glm::vec4(1.0f),
                             0.15f);
}

void App::_updateUIElements() {
  // Update UI text and positions
  float vWidth = m_uiManager.getVirtualWidth();
  float rightMargin = 2.0f;

  if (auto level_label =
          dynamic_cast<TextElement *>(m_uiManager.getElement("level_label"))) {
    float w = m_font.getTextWidth(level_label->text, level_label->scale);
    level_label->bounds.x = vWidth - rightMargin - w;
    level_label->bounds.y = 1.5f;
  }

  if (auto level_value =
          dynamic_cast<TextElement *>(m_uiManager.getElement("level_value"))) {
    level_value->text = std::to_string(m_game.getLevel());
    float w = m_font.getTextWidth(level_value->text, level_value->scale);
    level_value->bounds.x = vWidth - rightMargin - w;
    level_value->bounds.y = 3.5f;
  }

  if (auto score_label =
          dynamic_cast<TextElement *>(m_uiManager.getElement("score_label"))) {
    float w = m_font.getTextWidth(score_label->text, score_label->scale);
    score_label->bounds.x = vWidth - rightMargin - w;
    score_label->bounds.y = 6.0f;
  }

  if (auto score_value =
          dynamic_cast<TextElement *>(m_uiManager.getElement("score_value"))) {
    score_value->text = std::to_string(m_game.getScore());
    float w = m_font.getTextWidth(score_value->text, score_value->scale);
    score_value->bounds.x = vWidth - rightMargin - w;
    score_value->bounds.y = 7.5f;
  }

  if (auto darken_screen = dynamic_cast<InteractiveElement *>(
          m_uiManager.getElement("darken_screen"))) {
    darken_screen->bounds.w = vWidth;
    darken_screen->visible = !m_appState.gameStarted;
  }

  if (auto start_message = dynamic_cast<TextElement *>(
          m_uiManager.getElement("start_message"))) {
    float w = m_font.getTextWidth(start_message->text, start_message->scale);
    start_message->bounds.x = (vWidth - rightMargin - w) / 2;
    start_message->bounds.y = 20.0f;
    start_message->color.a =
        0.3f + 0.7f * (0.5f * (std::cos(glfwGetTime() * 2.0) + 1.0f));
    start_message->visible = !m_appState.gameStarted;
  }
}

void App::_handleProcessInput(double delta_time) {
  // Movement
  bool left = glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS;
  bool right = glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS;
  bool up = glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS;
  bool down = glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS;

  if (m_appState.gameStarted) {
    bool softDropActive = (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS);
    m_game.setSoftDrop(softDropActive);
  }

  // Preset Selection
  if (glfwGetKey(m_window, GLFW_KEY_1) == GLFW_PRESS)
    m_camera_controller.SetPreset(CameraPreset::FRONT);
  if (glfwGetKey(m_window, GLFW_KEY_2) == GLFW_PRESS)
    m_camera_controller.SetPreset(CameraPreset::TOP);
  if (glfwGetKey(m_window, GLFW_KEY_3) == GLFW_PRESS)
    m_camera_controller.SetPreset(CameraPreset::ISOMETRIC);

  m_camera_controller.HandleRotationInput(left, right, up, down, delta_time);
}

void App::_handleKeyCallback(int key, int scancode, int action, int mods) {
  using RelativeDir = TetrisManager::RelativeDir;
  using RelativeRotation = TetrisManager::RelativeRotation;

  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    if (!m_appState.gameStarted) {
      m_appState.gameStarted = true;
      return;
    }

    bool shift = (mods & GLFW_MOD_SHIFT);
    bool ctrl = (mods & GLFW_MOD_CONTROL);

    switch (key) {
    case GLFW_KEY_UP:
      if (shift)
        m_game.rotateRelative(RelativeRotation::PITCH, true, m_camera);
      else
        m_game.moveRelative(RelativeDir::BACK, m_camera);
      break;

    case GLFW_KEY_DOWN:
      if (shift)
        m_game.rotateRelative(RelativeRotation::PITCH, false, m_camera);
      else
        m_game.moveRelative(RelativeDir::FORWARD, m_camera);
      break;

    case GLFW_KEY_LEFT:
      if (shift)
        m_game.rotateRelative(RelativeRotation::ROLL, true, m_camera);
      else if (ctrl)
        m_game.rotateRelative(RelativeRotation::Y_AXIS, true, m_camera);
      else
        m_game.moveRelative(RelativeDir::LEFT, m_camera);
      break;

    case GLFW_KEY_RIGHT:
      if (shift)
        m_game.rotateRelative(RelativeRotation::ROLL, false, m_camera);
      else if (ctrl)
        m_game.rotateRelative(RelativeRotation::Y_AXIS, false, m_camera);
      else
        m_game.moveRelative(RelativeDir::RIGHT, m_camera);
      break;

    case GLFW_KEY_ENTER:
      m_game.hardDrop();
      break;

    case GLFW_KEY_H:
      m_game.hold();
      break;
    }
  }
}

// internal event handler
void App::_handleMouseMoveCallback(double pos_x, double pos_y) {
  m_appState.inputState.mouseLastX = pos_x;
  m_appState.inputState.mouseLastY = pos_y;
}

void App::_handleMouseClickCallback(int button, int action, int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    m_uiManager.handleClick(m_appState.inputState.mouseLastX,
                            m_appState.inputState.mouseLastY);
  }
}

void App::_handleScrollCallback(double offset_x, double offset_y) {
  // TODO: implement zooming in camera control, and utilized it here

  // float scrollSensitivity = 5.0f;
  // m_camera_controller.SetYaw(m_camera_controller.GetYaw() +
  //                            static_cast<float>(offset_y) *
  //                            scrollSensitivity);
  //
  // // Update vectors to apply changes
  // m_camera_controller.ProcessMouseMovement(0, 0);
}

void App::_handleFramebufferSizeCallback(int width, int height) {
  glViewport(0, 0, width, height);
  m_appState.windowWidth = width;
  m_appState.windowHeight = height;
  m_camera.UpdateSceneSize(width, height);
}

// GLFW static callbacks adapters
void App::_glfwKeyCallback(GLFWwindow *window, int key, int scancode,
                           int action, int mods) {
  App *app = static_cast<App *>(glfwGetWindowUserPointer(window));

  if (app) {
    app->_handleKeyCallback(key, scancode, action, mods);
  }
}
void App::_glfwMouseMoveCallback(GLFWwindow *window, double x_pos,
                                 double y_pos) {
  App *app = static_cast<App *>(glfwGetWindowUserPointer(window));

  if (app) {
    app->_handleMouseMoveCallback(x_pos, y_pos);
  }
}

void App::_glfwMouseButtonCallback(GLFWwindow *window, int button, int action,
                                   int mods) {
  App *app = static_cast<App *>(glfwGetWindowUserPointer(window));

  if (app) {
    app->_handleMouseClickCallback(button, action, mods);
  }
}

void App::_glfwScrollCallback(GLFWwindow *window, double offset_x,
                              double offset_y) {
  App *app = static_cast<App *>(glfwGetWindowUserPointer(window));

  if (app) {
    app->_handleScrollCallback(offset_x, offset_y);
  }
}

void App::_glfwFramebufferSizeCallback(GLFWwindow *window, int width,
                                       int height) {
  App *app = static_cast<App *>(glfwGetWindowUserPointer(window));

  if (app) {
    app->_handleFramebufferSizeCallback(width, height);
  }
}
