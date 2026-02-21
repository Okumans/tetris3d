#pragma once

#include "camera.h"
#include "core/camera_controller.hpp"
#include "game/tetris_manager.hpp"
#include "glad/gl.h"
#include "ui/ui_manager.hpp"
#include <GLFW/glfw3.h>

#ifndef SHADER_PATH
#define SHADER_PATH ASSETS_PATH "/shaders"
#endif

#ifndef ICONS_PATH
#define ICONS_PATH ASSETS_PATH "/icons"
#endif

#define UI_VERTEX_SHADER_PATH SHADER_PATH "/ui.vert.glsl"
#define UI_FRAGMENT_SHADER_PATH SHADER_PATH "/ui.frag.glsl"
#define TETROMINO_VERTEX_SHADER_PATH SHADER_PATH "/tetromino.vert.glsl"
#define TETROMINO_FRAGMENT_SHADER_PATH SHADER_PATH "/tetromino.frag.glsl"

struct InputState {
  bool isFirstMouse = false;
  float mouseLastX, mouseLastY;

  void setMousePosition(float pos_x, float pos_y) {}
};

struct AppState {
  int windowWidth, windowHeight;
  InputState inputState;
};

class App {
private:
  GLFWwindow *m_window;

  Camera m_camera;
  CameraController m_camera_controller;

  AppState m_appState;
  TetrisManager m_game;
  UIManager m_uiManager;

  // VAOs
  GLuint m_tetrominoVAO;

public:
  App(GLFWwindow *window);
  ~App();
  void render(double delta_time);

private:
  // GLFW static callbacks adapters
  static void _glfwMouseMoveCallback(GLFWwindow *window, double pos_x,
                                     double pos_y);
  static void _glfwMouseButtonCallback(GLFWwindow *window, int button,
                                       int action, int mods);
  static void _glfwScrollCallback(GLFWwindow *window, double offset_x,
                                  double offset_y);
  static void _glfwFramebufferSizeCallback(GLFWwindow *window, int width,
                                           int height);
  // internal event handler
  void _handleProcessInput(double delta_time);
  void _handleMouseMoveCallback(double pos_x, double pos_y);
  void _handleMouseClickCallback(int button, int action, int mods);
  void _handleScrollCallback(double offset_x, double offset_y);
  void _handleFramebufferSizeCallback(int width, int height);

  void _setupShaders();
  void _setupUIElements();
};
