#include "tetris_manager.hpp"
#include "core/geometry.hpp"
#include "core/shader_manager.hpp"
#include "game/space.hpp"
#include "game/tetromino.hpp"
#include "glm/fwd.hpp"
#include "shader.h"

#include <glad/gl.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <algorithm>
#include <random>
#include <ranges>
#include <utility>

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

  while (m_dropTimer >= current_tick_delay) {
    m_dropTimer -= current_tick_delay;

    if (!_moveDown()) {
      // check if line clear, then commit the operation (move this pice to the
      // tetris space)
      _commit();

      // recalculate height map
      for (int y = 0; y < SPACE_HEIGHT; ++y) {
        for (auto [x, z] : m_activePiece.getGlobalPositions() |
                               std::views::transform([](glm::ivec3 pos) {
                                 return std::make_pair(pos.x, pos.z);
                               })) {
          m_depth_map[x][z] = std::max(
              m_depth_map[x][z], m_space.at(x, y, z).isOccupied() ? y + 1 : 0);
        }
      }

      m_activePiece = _get_random_piece();
    }

    if (m_isSoftDropping)
      break;
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
  shader.setVec4("u_color", glm::vec4(0.5f, 0.5f, 0.5f, 0.7f)); // Grey outline
  m_gridBox.render(camera.GetViewMatrix(), camera.GetProjectionMatrix());

  glBindVertexArray(m_vao);

  // Draw On Grid Piece
  for (int y = 0; y < SPACE_HEIGHT; ++y) {
    for (int x = 0; x < SPACE_WIDTH; ++x) {
      for (int z = 0; z < SPACE_DEPTH; ++z) {
        const GridCell &cell = m_space.at(x, y, z);

        if (cell.isOccupied()) {
          glm::vec3 world_pos = m_space.gridToWorld(x, y, z);
          glm::vec4 color =
              glm::vec4(TetrominoFactory::getColor(cell.type), 1.0f);
          _drawCell(world_pos, color, shader);
        }
      }
    }
  }

  // Draw Active Piece
  glm::vec4 active_piece_color(m_activePiece.getColor(), 1.0f);
  for (glm::ivec3 grid_pos : m_activePiece.getGlobalPositions()) {
    glm::vec3 world_pos =
        m_space.gridToWorld(grid_pos.x, grid_pos.y, grid_pos.z);
    _drawCell(world_pos, active_piece_color, shader);
  }

  // Draw Preview Piece
  glm::vec4 preview_piece_color(m_activePiece.getColor() * 1.7f, 0.2f);
  glm::ivec3 preview_relative_pos = _calculateDropOffset();

  for (glm::ivec3 grid_pos :
       m_activePiece.tryMoveRelative(preview_relative_pos)) {
    glm::vec3 world_pos =
        m_space.gridToWorld(grid_pos.x, grid_pos.y, grid_pos.z);
    _drawCell(world_pos, preview_piece_color, shader);
  }
}

// Control activePiece
bool TetrisManager::rotateX(bool clockwise) {
  m_activePiece.rotateX(clockwise);

  if (!_checkValidPiece(m_activePiece)) {
    m_activePiece.rotateX(!clockwise);
    return false;
  }

  return true;
}

bool TetrisManager::rotateY(bool clockwise) {
  m_activePiece.rotateY(clockwise);

  if (!_checkValidPiece(m_activePiece)) {
    m_activePiece.rotateY(!clockwise);
    return false;
  }

  return true;
}

bool TetrisManager::rotateZ(bool clockwise) {
  m_activePiece.rotateZ(clockwise);

  if (!_checkValidPiece(m_activePiece)) {
    m_activePiece.rotateZ(!clockwise);
    return false;
  }

  return true;
}

bool TetrisManager::moveRelative(RelativeDir direction, const Camera &camera) {
  glm::vec3 cam_right = camera.GetRight();
  glm::vec3 cam_front = camera.GetFront();

  cam_right.y = 0;
  cam_front.y = 0;
  cam_right = glm::normalize(cam_right);
  cam_front = glm::normalize(cam_front);

  glm::ivec3 gridMove(0);

  switch (direction) {
  case RelativeDir::RIGHT:
    gridMove = _snapToGridAxis(cam_right);
    break;
  case RelativeDir::LEFT:
    gridMove = _snapToGridAxis(-cam_right);
    break;
  case RelativeDir::FORWARD:
    gridMove = _snapToGridAxis(-cam_front);
    break;
  case RelativeDir::BACK:
    gridMove = _snapToGridAxis(cam_front);
    break;
  }

  if (!_checkValidPiecePosition(m_activePiece.tryMoveRelative(gridMove))) {
    return false;
  }

  m_activePiece.moveRelative(gridMove);

  return true;
}

void TetrisManager::hardDrop() {
  glm::ivec3 drop_offset = _calculateDropOffset();
  m_activePiece.moveRelative(drop_offset);
}

void TetrisManager::setSoftDrop(bool is_soft_dropping) {
  m_isSoftDropping = is_soft_dropping;
}

void TetrisManager::_commit() {
  for (glm::ivec3 cell_position : m_activePiece.getGlobalPositions()) {
    m_space.at(cell_position.x, cell_position.y, cell_position.z).type =
        m_activePiece.getType();
  }
}

// Returns relative distance to the dropped position
// can used with Tetromino::moveRelative, or tryMoveRelative
glm::ivec3 TetrisManager::_calculateDropOffset() {

  int max_floor_y =
      std::ranges::max(m_activePiece.getGlobalPositions() |
                       std::views::transform([this](glm::ivec3 pos) {
                         return this->m_depth_map[pos.x][pos.z];
                       }));
  int min_relative_y =
      std::ranges::min(m_activePiece.getOffsets(), {}, &glm::ivec3::y).y;

  glm::ivec3 preview_relative_pos(0);
  bool found_preview_y_pos = false;

  // for (glm::ivec3 grid_pos : m_activePiece.getGlobalPositions()) {
  //   if (found_preview_y_pos) {
  //     break;
  //   }

  for (int y = max_floor_y - min_relative_y; y >= 0; --y) {
    glm::ivec3 relative_pos(0, y - m_activePiece.getPosition().y, 0);

    if (relative_pos.y > 0 ||
        (!_checkValidPiecePosition(
            m_activePiece.tryMoveRelative(relative_pos)))) {
      found_preview_y_pos = true;
      break;
    }

    preview_relative_pos = relative_pos;
  }
  // }

  return preview_relative_pos;
}

bool TetrisManager::_moveDown() {
  const glm::ivec3 direction = {0, -1, 0};

  if (!_checkValidPiecePosition(m_activePiece.tryMoveRelative(direction))) {
    return false;
  }

  m_activePiece.moveRelative(direction);

  return true;
}

bool TetrisManager::_checkValidPiece(const Tetromino &moved_piece) const {
  return _checkValidPiecePosition(moved_piece.getGlobalPositions());
}

bool TetrisManager::_checkValidPiecePosition(
    IVec3Range auto &&positions) const {
  for (glm::ivec3 cell_position : positions) {
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

glm::ivec3 TetrisManager::_snapToGridAxis(glm::vec3 direction) {
  if (std::abs(direction.x) > std::abs(direction.z)) {
    return glm::ivec3(direction.x > 0 ? 1 : -1, 0, 0);
  } else {
    return glm::ivec3(0, 0, direction.z > 0 ? 1 : -1);
  }
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

void TetrisManager::_drawCell(glm::vec3 world_pos, glm::vec4 color,
                              const Shader &shader) {
  glm::mat4 model = glm::translate(glm::mat4(1.0f), world_pos);
  shader.setMat4("u_model", model);
  shader.setVec4("u_color", color);

  glDrawArrays(GL_TRIANGLES, 0, 36);
}
