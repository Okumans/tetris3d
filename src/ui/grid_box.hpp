#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

class GridBox {
public:
  GridBox(float width, float height, float depth);
  ~GridBox();
  void render(const glm::mat4 &view, const glm::mat4 &projection);

private:
  unsigned int m_vao, m_vbo;
  int m_vertexCount;
};
