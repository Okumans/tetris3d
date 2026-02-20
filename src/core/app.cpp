#include "core/app.hpp"
#include "GLFW/glfw3.h"
#include "core/shader_manager.hpp"
#include "glad/gl.h"
#include "ui/ui_manager.hpp"
#include <print>

void App::render(double delta_time) {
  _handleProcessInput(delta_time);
  m_game.render(m_camera, delta_time);
  m_uiManager.render(m_appState.windowWidth, m_appState.windowHeight);
}

App::App(GLFWwindow *window)
    : m_window(window), m_camera(glm::vec3(0.0f, 10.0f, 30.0f)) {

  glfwSetWindowUserPointer(m_window, (void *)this);

  glfwSetCursorPosCallback(m_window, _glfwMouseMoveCallback);
  glfwSetMouseButtonCallback(m_window, _glfwMouseButtonCallback);
  glfwSetScrollCallback(m_window, _glfwScrollCallback);
  glfwSetFramebufferSizeCallback(m_window, _glfwFramebufferSizeCallback);

  _setupShaders();
  _setupUIElements();

  int width, height;
  glfwGetWindowSize(m_window, &width, &height);
  m_camera.UpdateSceneSize(width, height);
}

App::~App() { glDeleteVertexArrays(1, &m_tetrominoVAO); }

void App::_setupShaders() {
  ShaderManager::loadShader(ShaderType::UI, UI_VERTEX_SHADER_PATH,
                            UI_FRAGMENT_SHADER_PATH);
  ShaderManager::loadShader(ShaderType::TETROMINO, TETROMINO_VERTEX_SHADER_PATH,
                            TETROMINO_FRAGMENT_SHADER_PATH);
}

void App::setCameraPreset(std::string_view preset) {
  if (preset == "front") {
    m_camera.Position = glm::vec3(0.0f, 10.0f, 30.0f);
    m_camera.SetYaw(-90.0f);
    m_camera.SetPitch(0.0f);
  } else if (preset == "top") {
    m_camera.Position = glm::vec3(0.0f, 40.0f, 0.0f);
    m_camera.SetYaw(-90.0f);
    m_camera.SetPitch(-89.0f); // Avoid gimbal lock at -90
  } else if (preset == "isometric") {
    m_camera.Position = glm::vec3(20.0f, 25.0f, 20.0f);
    m_camera.SetYaw(-135.0f);
    m_camera.SetPitch(-25.0f);
  }
}

void App::_setupUIElements() {
  m_uiManager.addElement("test_click", {10, 10, 200, 200}, {0.0f, 1.0f, 0.0f},
                         [this](UIElement *self) {
                           std::println("Hello world everyone, from ({}, {})",
                                        this->m_appState.inputState.mouseLastX,
                                        this->m_appState.inputState.mouseLastY);
                         });
}

void App::_handleProcessInput(double delta_time) {
  // Preset Selection
  if (glfwGetKey(m_window, GLFW_KEY_1) == GLFW_PRESS)
    setCameraPreset("front");
  if (glfwGetKey(m_window, GLFW_KEY_2) == GLFW_PRESS)
    setCameraPreset("top");
  if (glfwGetKey(m_window, GLFW_KEY_3) == GLFW_PRESS)
    setCameraPreset("isometric");

  // Manual Rotation with A and D
  if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
    m_camera.SetYaw(m_camera.GetYaw() -
                    m_rotationSpeed * static_cast<float>(delta_time));
    m_camera.ProcessMouseMovement(0, 0);
  }
  if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
    m_camera.SetYaw(m_camera.GetYaw() +
                    m_rotationSpeed * static_cast<float>(delta_time));
    m_camera.ProcessMouseMovement(0, 0);
  }
}

// internal event handler
void App::_handleMouseMoveCallback(double pos_x, double pos_y) {
  // Reserve for future controls
}

void App::_handleMouseClickCallback(int button, int action, int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    m_uiManager.handleClick(m_appState.inputState.mouseLastX,
                            m_appState.inputState.mouseLastY);
  }
}

void App::_handleScrollCallback(double offset_x, double offset_y) {
  float scrollSensitivity = 5.0f;
  m_camera.SetYaw(m_camera.GetYaw() +
                  static_cast<float>(offset_y) * scrollSensitivity);

  // Update vectors to apply changes
  m_camera.ProcessMouseMovement(0, 0);
}

void App::_handleFramebufferSizeCallback(int width, int height) {
  glViewport(0, 0, width, height);
  m_appState.windowWidth = width;
  m_appState.windowHeight = height;
  m_camera.UpdateSceneSize(width, height);
}

// GLFW static callbacks adapters
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
