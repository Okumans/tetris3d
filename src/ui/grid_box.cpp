#include "grid_box.hpp"
#include "glad/gl.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

GridBox::GridBox(float width, float height, float depth) {
  // Offset by 0.5 to align with your gridToWorld logic
  float x = width / 2.0f;
  float y = height;
  float z = depth / 2.0f;

  std::vector<float> vertices = {// Bottom square
                                 -x, 0, -z, x, 0, -z, x, 0, -z, x, 0, z, x, 0,
                                 z, -x, 0, z, -x, 0, z, -x, 0, -z,
                                 // Top square
                                 -x, y, -z, x, y, -z, x, y, -z, x, y, z, x, y,
                                 z, -x, y, z, -x, y, z, -x, y, -z,
                                 // Vertical pillars
                                 -x, 0, -z, -x, y, -z, x, 0, -z, x, y, -z, x, 0,
                                 z, x, y, z, -x, 0, z, -x, y, z};

  m_vertexCount = vertices.size() / 3;

  glCreateVertexArrays(1, &m_vao);
  glCreateBuffers(1, &m_vbo);

  glNamedBufferStorage(m_vbo, vertices.size() * sizeof(float), vertices.data(),
                       0);

  glEnableVertexArrayAttrib(m_vao, 0);
  glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(m_vao, 0, 0);
  glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, 3 * sizeof(float));
}

GridBox::~GridBox() {
  glDeleteVertexArrays(1, &m_vao);
  glDeleteBuffers(1, &m_vbo);
}

void GridBox::render(const glm::mat4 &view, const glm::mat4 &projection) {
  // You can use your existing Tetromino shader or a simple solid color shader
  glBindVertexArray(m_vao);
  glDrawArrays(GL_LINES, 0, m_vertexCount);
}
