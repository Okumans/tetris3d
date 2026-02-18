#include "tetris_manager.hpp"
#include "core/geometry.hpp"
#include "core/shader_manager.hpp"
#include "game/space.hpp"
#include "game/tetromino.hpp"
#include "glad/gl.h"
#include "glm/ext/matrix_transform.hpp"
#include "shader.h"

#include <print>
#include <random>

TetrisManager::TetrisManager() : m_activePiece(_get_random_piece()) {
  _setupBuffers();
}

TetrisManager::~TetrisManager() {}

void TetrisManager::render(const Camera &camera, double delta_time) {
  glEnable(GL_DEPTH_TEST);

  Shader shader = ShaderManager::getShader(ShaderType::TETROMINO);
  shader.use();

  shader.setVec3("u_viewPos", camera.Position);
  shader.setMat4("u_view", camera.GetViewMatrix());
  shader.setMat4("u_projection", camera.GetProjectionMatrix());

  // Draw On Grid Piece
  for (int y = 0; y < SPACE_HEIGHT; ++y) {
    for (int x = 0; x < SPACE_WIDTH; ++x) {
      for (int z = 0; z < SPACE_DEPTH; ++z) {
        const auto &cell = m_space.at(x, y, z);
        if (cell.isOccupied()) {
          std::println("cell at ({}, {}, {})", x, y, z);
          glm::vec3 worldPos = m_space.gridToWorld(x, y, z);
          _drawCell(worldPos.x, worldPos.y, worldPos.z, shader);
        }
      }
    }
  }

  // Draw Active Piece
  for (const auto &gridPos : m_activePiece.getGlobalPositions()) {
    glm::vec3 worldPos = m_space.gridToWorld(gridPos.x, gridPos.y, gridPos.z);
    std::println("active cell at ({}, {}, {})", gridPos.x, gridPos.y,
                 gridPos.z);
    _drawCell(worldPos.x, worldPos.y, worldPos.z, shader);
  }
}

// Control activePiece
void TetrisManager::rotateX(bool clockwise) {}
void TetrisManager::rotateY(bool clockwise) {}
void TetrisManager::rotateZ(bool clockwise) {}
void TetrisManager::moveLeft() {}
void TetrisManager::moveRight() {}
void TetrisManager::moveInward() {}
void TetrisManager::moveOutward() {}

Tetromino TetrisManager::_get_random_piece() {
  static std::mt19937 gen(std::random_device{}());

  std::uniform_int_distribution<int> dist(
      static_cast<int>(BlockType::Straight),
      static_cast<int>(BlockType::RightStep));

  BlockType randomType = static_cast<BlockType>(dist(gen));

  glm::ivec3 spawnGridPos = {SPACE_WIDTH / 2, SPACE_HEIGHT - 1,
                             SPACE_DEPTH / 2};

  return Tetromino(randomType, spawnGridPos);
}

void TetrisManager::_setupBuffers() {
  TetrominoVertex cubeVertices[] = {
      // Back face (Normal: 0, 0, -1)
      {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
      {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
      {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
      {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
      {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
      {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},

      // Front face (Normal: 0, 0, 1)
      {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
      {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
      {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
      {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
      {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
      {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},

      // Left face (Normal: -1, 0, 0)
      {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
      {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
      {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
      {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
      {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
      {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},

      // Right face (Normal: 1, 0, 0)
      {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
      {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
      {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
      {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
      {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
      {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},

      // Bottom face (Normal: 0, -1, 0)
      {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
      {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
      {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
      {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
      {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
      {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},

      // Top face (Normal: 0, 1, 0)
      {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
      {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
      {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
      {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
      {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
      {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}};

  // Create Tertromino VBO
  glCreateBuffers(1, &m_vbo);
  glNamedBufferStorage(m_vbo, sizeof(cubeVertices), cubeVertices, 0);

  // Setup Tertromino VAO
  glCreateVertexArrays(1, &m_vao);

  // index 0: vec3; position attribute
  glEnableVertexArrayAttrib(m_vao, 0);
  glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE,
                            offsetof(TetrominoVertex, pos));
  glVertexArrayAttribBinding(m_vao, 0, 0);

  // index 1: vec3; normal attribute
  glEnableVertexArrayAttrib(m_vao, 1);
  glVertexArrayAttribFormat(m_vao, 1, 3, GL_FLOAT, GL_FALSE,
                            offsetof(TetrominoVertex, normal));
  glVertexArrayAttribBinding(m_vao, 1, 0);

  // index 2: vec2; uv attribute
  glEnableVertexArrayAttrib(m_vao, 2);
  glVertexArrayAttribFormat(m_vao, 2, 2, GL_FLOAT, GL_FALSE,
                            offsetof(TetrominoVertex, uv));
  glVertexArrayAttribBinding(m_vao, 2, 0);

  // Link VAO <-> VBO
  glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(TetrominoVertex));
}

void TetrisManager::_drawCell(int x, int y, int z, const Shader &shader) {
  const GridCell &cell = m_space.at(x, y, z);

  glm::vec3 worldPos = m_space.gridToWorld(x, y, z);

  glm::mat4 model = glm::translate(glm::mat4(1.0f), worldPos);
  shader.setMat4("u_model", model);
  shader.setVec3("u_color", TetrominoFactory::getColor(cell.type));

  glDrawArrays(GL_TRIANGLES, 0, 36);
}
