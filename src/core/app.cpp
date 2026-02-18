#include "core/app.hpp"
#include "GLFW/glfw3.h"
#include "core/geometry.hpp"
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
    : m_window(window), m_camera(glm::vec3(0.0f, 15.0f, 25.0f)) {

  glfwSetWindowUserPointer(m_window, (void *)this);

  glfwSetCursorPosCallback(m_window, _glfwMouseMoveCallback);
  glfwSetMouseButtonCallback(m_window, _glfwMouseButtonCallback);
  glfwSetScrollCallback(m_window, _glfwScrollCallback);
  glfwSetFramebufferSizeCallback(m_window, _glfwFramebufferSizeCallback);

  _setupShaders();
  _setupBuffers();
  _setupUIElements();

  m_camera.UpdateSceneSize(800, 600);
}

App::~App() { glDeleteVertexArrays(1, &m_tetrominoVAO); }

void App::_setupShaders() {
  ShaderManager::loadShader(ShaderType::UI, UI_VERTEX_SHADER_PATH,
                            UI_FRAGMENT_SHADER_PATH);
  ShaderManager::loadShader(ShaderType::TETROMINO, TETROMINO_VERTEX_SHADER_PATH,
                            TETROMINO_FRAGMENT_SHADER_PATH);
}

void App::_setupBuffers() {
  // Setup Tertromino VAO
  glCreateVertexArrays(1, &m_tetrominoVAO);

  // index 0: vec3; position attribute
  glEnableVertexArrayAttrib(m_tetrominoVAO, 0);
  glVertexArrayAttribFormat(m_tetrominoVAO, 0, 3, GL_FLOAT, GL_FALSE,
                            offsetof(TetrominoVertex, pos));
  glVertexArrayAttribBinding(m_tetrominoVAO, 0, 0);

  // index 1: vec3; normal attribute
  glEnableVertexArrayAttrib(m_tetrominoVAO, 1);
  glVertexArrayAttribFormat(m_tetrominoVAO, 1, 3, GL_FLOAT, GL_FALSE,
                            offsetof(TetrominoVertex, normal));
  glVertexArrayAttribBinding(m_tetrominoVAO, 1, 0);

  // index 2: vec2; uv attribute
  glEnableVertexArrayAttrib(m_tetrominoVAO, 2);
  glVertexArrayAttribFormat(m_tetrominoVAO, 2, 2, GL_FLOAT, GL_FALSE,
                            offsetof(TetrominoVertex, uv));
  glVertexArrayAttribBinding(m_tetrominoVAO, 2, 0);
}

void App::_setupUIElements() {
  m_uiManager.addElement("test_click", {100, 100, 200, 200}, {0.0f, 1.0f, 0.0f},
                         [this](UIElement *self) {
                           std::println("Hello world everyone, from ({}, {})",
                                        this->m_appState.inputState.mouseLastX,
                                        this->m_appState.inputState.mouseLastY);
                         });
}

void App::_handleProcessInput(double delta_time) {
  if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
    m_camera.ProcessKeyboard(FORWARD, delta_time);
  if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
    m_camera.ProcessKeyboard(BACKWARD, delta_time);
  if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
    m_camera.ProcessKeyboard(LEFT, delta_time);
  if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
    m_camera.ProcessKeyboard(RIGHT, delta_time);
}

// internal event handler
void App::_handleMouseMoveCallback(double pos_x, double pos_y) {
  if (m_appState.inputState.isFirstMouse) {
    m_appState.inputState.mouseLastX = pos_x;
    m_appState.inputState.mouseLastY = pos_y;
    m_appState.inputState.isFirstMouse = false;
  }

  float offset_x = pos_x - m_appState.inputState.mouseLastX;
  float offset_y = m_appState.inputState.mouseLastY -
                   pos_y; // reversed since y-coordinates go from bottom to top

  m_appState.inputState.mouseLastX = pos_x;
  m_appState.inputState.mouseLastY = pos_y;

  m_camera.ProcessMouseMovement(offset_x, offset_y);
}

void App::_handleMouseClickCallback(int button, int action, int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    m_uiManager.handleClick(m_appState.inputState.mouseLastX,
                            m_appState.inputState.mouseLastY);
  }
}

void App::_handleScrollCallback(double offset_x, double offset_y) {
  m_camera.ProcessMouseScroll(static_cast<float>(offset_y));
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
