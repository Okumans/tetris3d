#include "tetris_manager.hpp"
#include "core/geometry.hpp"
#include "core/shader_manager.hpp"
#include "game/space.hpp"
#include "game/tetromino.hpp"
#include "glad/gl.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "shader.h"

#include <random>

TetrisManager::TetrisManager() : m_activePiece(_get_random_piece()) {
  _setupBuffers();
}

TetrisManager::~TetrisManager() {}

void TetrisManager::update(double delta_time) {
  double base_speed =
      std::max(0.05, m_baseDropDelay - (m_level * m_delayDecreaseRate));
  double current_tick_delay =
      m_isSoftDropping ? (base_speed / 10.0) : base_speed;

  m_dropTimer += delta_time;

  // on drop
  while (m_dropTimer >= current_tick_delay) {
    m_dropTimer -= current_tick_delay;

    if (!_moveDown()) {
      // check if line clear, then commit the operation (move this pice to the
      // tetris space)
      _commit();
      m_activePiece = _get_random_piece();
    }
  }
}

void TetrisManager::render(const Camera &camera, double delta_time) {
  glEnable(GL_DEPTH_TEST);

  Shader shader = ShaderManager::getShader(ShaderType::TETROMINO);
  shader.use();

  shader.setVec3("u_viewPos", camera.Position);
  shader.setMat4("u_view", camera.GetViewMatrix());
  shader.setMat4("u_projection", camera.GetProjectionMatrix());

  shader.setMat4("u_model", glm::mat4(1.0f));
  shader.setVec3("u_color", glm::vec3(0.5f)); // Grey outline
  m_gridBox.render(camera.GetViewMatrix(), camera.GetProjectionMatrix());

  glBindVertexArray(m_vao);

  // Draw On Grid Piece
  for (int y = 0; y < SPACE_HEIGHT; ++y) {
    for (int x = 0; x < SPACE_WIDTH; ++x) {
      for (int z = 0; z < SPACE_DEPTH; ++z) {
        const auto &cell = m_space.at(x, y, z);
        if (cell.isOccupied()) {
          glm::vec3 world_pos = m_space.gridToWorld(x, y, z);
          glm::vec3 color = TetrominoFactory::getColor(cell.type);
          _drawCell(world_pos, color, shader);
        }
      }
    }
  }

  // Draw Active Piece
  glm::vec3 active_piece_color = m_activePiece.getColor();
  for (const auto &gridPos : m_activePiece.getGlobalPositions()) {
    glm::vec3 world_pos = m_space.gridToWorld(gridPos.x, gridPos.y, gridPos.z);
    _drawCell(world_pos, active_piece_color, shader);
  }
}

// Control activePiece
bool TetrisManager::rotateX(bool clockwise) {
  m_activePiece.rotateX(clockwise);

  if (!_checkValidAction(m_activePiece)) {
    m_activePiece.rotateX(!clockwise);
    return false;
  }

  return true;
}

bool TetrisManager::rotateY(bool clockwise) {
  m_activePiece.rotateY(clockwise);

  if (!_checkValidAction(m_activePiece)) {
    m_activePiece.rotateY(!clockwise);
    return false;
  }

  return true;
}

bool TetrisManager::rotateZ(bool clockwise) {
  m_activePiece.rotateZ(clockwise);

  if (!_checkValidAction(m_activePiece)) {
    m_activePiece.rotateZ(!clockwise);
    return false;
  }

  return true;
}

bool TetrisManager::moveLeft() {
  m_activePiece.moveLeft();

  if (!_checkValidAction(m_activePiece)) {
    m_activePiece.moveRight();
    return false;
  }

  return true;
}

bool TetrisManager::moveRight() {
  m_activePiece.moveRight();

  if (!_checkValidAction(m_activePiece)) {
    m_activePiece.moveLeft();
    return false;
  }

  return true;
}

bool TetrisManager::moveInward() {
  m_activePiece.moveInward();

  if (!_checkValidAction(m_activePiece)) {
    m_activePiece.moveOutward();
    return false;
  }

  return true;
}

bool TetrisManager::moveOutward() {
  m_activePiece.moveOutward();

  if (!_checkValidAction(m_activePiece)) {
    m_activePiece.moveInward();
    return false;
  }

  return true;
}

void TetrisManager::_commit() {
  for (glm::ivec3 cell_position : m_activePiece.getGlobalPositions()) {
    m_space.at(cell_position.x, cell_position.y, cell_position.z).type =
        m_activePiece.getType();
  }
}

bool TetrisManager::_moveDown() {
  m_activePiece.moveDown();

  if (!_checkValidAction(m_activePiece)) {
    m_activePiece.moveUp();
    return false;
  }

  return true;
}

bool TetrisManager::_checkValidAction(const Tetromino &moved_piece) const {
  std::vector<glm::ivec3> active_piece_cell_positions =
      moved_piece.getGlobalPositions();

  for (glm::ivec3 cell_position : active_piece_cell_positions) {
    if (!m_space.checkInBound(cell_position.x, cell_position.y,
                              cell_position.z)) {
      return false;
    }

    if (m_space.at(cell_position.x, cell_position.y, cell_position.z)
            .isOccupied()) {
      return false;
    }
  }

  return true;
}

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

void TetrisManager::_drawCell(glm::vec3 world_pos, glm::vec3 color,
                              const Shader &shader) {
  glm::mat4 model = glm::translate(glm::mat4(1.0f), world_pos);
  shader.setMat4("u_model", model);
  shader.setVec3("u_color", color);

  glDrawArrays(GL_TRIANGLES, 0, 36);
}
