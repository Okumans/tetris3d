#pragma once

#include <glm/glm.hpp>

struct TetrominoVertex {
  glm::vec3 pos;
  glm::vec3 normal;
  glm::vec2 uv;
};

struct UIVertex {
  glm::vec2 pos;
  glm::vec2 uv;
};

// void draw_ui_quad(GLuint ui_vao);
