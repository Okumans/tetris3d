#include "tetris_manager.hpp"
#include "camera.h"
#include "core/geometry.hpp"
#include "core/shader_manager.hpp"
#include "game/space.hpp"
#include "game/tetromino.hpp"
#include "shader.h"

#include <glad/gl.h>

#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <algorithm>
#include <limits>
#include <optional>
#include <print>
#include <random>
#include <utility>
#include <vector>

TetrisManager::TetrisManager()
    : m_activePiece(
          Tetromino(_getRandomPieceType(0),
                    {SPACE_WIDTH / 2, SPACE_HEIGHT - 1, SPACE_DEPTH / 2})) {

  _spawnPiece();
  _setupBuffers();
}

TetrisManager::~TetrisManager() {}

void TetrisManager::update(double delta_time) {
  if (m_state == GameState::GAME_OVER)
    return;

  double base_speed =
      std::max(0.7, m_baseDropDelay - (m_level * m_delayDecreaseRate));
  double current_tick_delay =
      m_isSoftDropping ? (base_speed / 10.0) : base_speed;

  // Falling or Locking
  if (m_state == GameState::FALLING || m_state == GameState::LOCKING) {
    m_dropTimer += delta_time;

    while (m_dropTimer >= current_tick_delay) {
      m_dropTimer -= current_tick_delay;

      if (!_moveDown()) {
        // Hit floor, start the grace period
        m_state = GameState::LOCKING;
      } else {
        // If successfully moved down (Falling again)
        if (m_state == GameState::LOCKING)
          m_state = GameState::FALLING;
      }
    }
  }

  // Clearing
  if (m_state == GameState::CLEARING) {
    std::println("clearing {}", m_collapseTimer);

    m_collapseTimer += delta_time;

    if (m_collapseTimer >= MAX_COLLASPE_DELAY) {
      _collapseLayers(m_pendingClearLayers);
      m_pendingClearLayers.clear();

      if (!_spawnPiece()) {
        m_state = GameState::GAME_OVER;

        // TODO: do someething with gameover thing
        std::println("i think you lose my bro");
      } else {
        m_state = GameState::FALLING;
      }
    }

    return;
  }

  // Locking
  if (m_state == GameState::LOCKING) {
    m_lockTimer += delta_time;

    if (m_lockTimer >= MAX_LOCK_DELAY) {
      _performCommitSequence();
    }
  }
}

void TetrisManager::render(double delta_time, const Camera &camera) {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);

  Shader &shader = ShaderManager::getShader(ShaderType::TETROMINO);
  shader.use();

  shader.setVec3("u_viewPos", camera.Position);
  shader.setMat4("u_view", camera.GetViewMatrix());
  shader.setMat4("u_projection", camera.GetProjectionMatrix());

  _renderGrid(shader, camera.GetViewMatrix(), camera.GetProjectionMatrix());
  glBindVertexArray(m_vao);

  // Draw On Grid Piece
  _renderOnGridPiece(shader);

  // Draw Active Piece
  _renderActivePiece(shader);

  // Draw Ghost Piece
  _renderGhostPiece(shader);
}

bool TetrisManager::rotateRelative(RelativeRotation type, bool clockwise,
                                   const Camera &camera) {
  glm::ivec3 rotationAxis(0);

  glm::vec3 cam_right = camera.GetRight();
  glm::vec3 cam_front = camera.GetFront();
  cam_right.y = 0;
  cam_front.y = 0;

  if (type == RelativeRotation::Y_AXIS) {
    rotationAxis = glm::ivec3(0, 1, 0);
  }

  else if (type == RelativeRotation::PITCH) {
    rotationAxis = _snapToGridAxis(cam_right);
  }

  else if (type == RelativeRotation::ROLL) {
    rotationAxis = _snapToGridAxis(cam_front);
  }

  ;

  if (!_checkValidPiecePosition(
          _tryApplyGlobalRotation(rotationAxis, clockwise))) {
    return false;
  }

  _applyGlobalRotation(rotationAxis, clockwise);
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

  // Give the player more time
  if (m_state == GameState::LOCKING) {
    if (m_lockMoveResetCount < MAX_LOCK_RESETS) {
      m_lockTimer = 0.0;
      m_lockMoveResetCount++;
    }
  }

  return true;
}

void TetrisManager::hold() {
  if (!m_canHold)
    return;

  if (!m_heldPiece.has_value()) {
    m_heldPiece = std::move(m_activePiece);
    _spawnPiece();
  } else {
    m_piecesQueue.push_front(std::move(m_heldPiece.value()));
    m_heldPiece = std::move(m_activePiece);
    _spawnPiece();
  }

  m_canHold = false;
}

void TetrisManager::hardDrop() {
  if (m_state == GameState::CLEARING || m_state == GameState::GAME_OVER) {
    return;
  }

  glm::ivec3 drop_offset = _calculateDropOffset();
  m_activePiece.moveRelative(drop_offset);

  _performCommitSequence();
}

void TetrisManager::setSoftDrop(bool is_soft_dropping) {
  m_isSoftDropping = is_soft_dropping;
}

const Tetromino &TetrisManager::getActivePiece() const { return m_activePiece; }

const std::deque<Tetromino> &TetrisManager::getPiecesQueue() const {
  return m_piecesQueue;
}
const std::optional<Tetromino> &TetrisManager::getHold() const {
  return m_heldPiece;
}

void TetrisManager::_commit() {
  for (glm::ivec3 cell_position : m_activePiece.getGlobalPositions()) {
    if (!m_space.checkInBound(cell_position.x, cell_position.y,
                              cell_position.z))
      continue;

    m_space.at(cell_position.x, cell_position.y, cell_position.z).type =
        m_activePiece.getType();

    // Update depth map
    m_depth_map[cell_position.x][cell_position.z] = std::max(
        m_depth_map[cell_position.x][cell_position.z], cell_position.y);
  }
  // Points for landing a piece
  m_score += 10 * (m_level + 1);
}

void TetrisManager::_performCommitSequence() {
  _commit();
  m_lockTimer = 0.0;
  m_lockMoveResetCount = 0;

  _checkLayerClears(m_pendingClearLayers);

  if (!m_pendingClearLayers.empty()) {
    // Scoring for cleared layers
    size_t lines = m_pendingClearLayers.size();
    uint64_t base_points = 0;
    if (lines == 1)
      base_points = 300;
    else if (lines == 2)
      base_points = 800;
    else if (lines == 3)
      base_points = 1500;
    else if (lines >= 4)
      base_points = 2500;

    m_score += base_points * (m_level + 1);
    m_linesCleared += lines;
    m_level = static_cast<uint8_t>(m_linesCleared / 10);

    m_state = GameState::CLEARING;
    m_collapseTimer = 0.0;
  } else {
    _finalizeSpawn();
  }

  m_canHold = true;
}

void TetrisManager::_finalizeSpawn() {
  if (!_spawnPiece()) {
    m_state = GameState::GAME_OVER;
    std::println("i think you lose my bro");
  } else {
    m_state = GameState::FALLING;
  }
}

void TetrisManager::_checkLayerClears(std::vector<int> &layers_cleared) {
  static std::vector<int> unique_y;

  unique_y.clear();
  layers_cleared.clear();

  for (const auto &pos : m_activePiece.getGlobalPositions()) {
    if (std::ranges::find(unique_y, pos.y) == unique_y.end()) {
      unique_y.push_back(pos.y);
    }
  }

  for (int y : unique_y) {
    bool is_plane_full = true;

    for (int x = 0; x < SPACE_WIDTH; ++x) {
      for (int z = 0; z < SPACE_DEPTH; ++z) {
        if (!m_space.checkInBound(x, y, z)) {
          continue;
        }

        if (m_space.at(x, y, z).isEmpty()) {
          is_plane_full = false;
          break;
        }
      }
    }

    if (is_plane_full) {
      layers_cleared.push_back(y);
    }
  }
}

void TetrisManager::_collapseLayers(const std::vector<int> &layers_cleared) {
  if (layers_cleared.empty()) {
    return;
  }

  int write_y = 0;

  for (int read_y = 0; read_y < SPACE_HEIGHT; ++read_y) {
    if (std::ranges::find(layers_cleared, read_y) != layers_cleared.end()) {
      continue;
    }

    if (read_y != write_y) {
      // copy layer y at read_y to write_y
      for (int x = 0; x < SPACE_WIDTH; ++x) {
        for (int z = 0; z < SPACE_DEPTH; ++z) {
          m_space.at(x, write_y, z) = m_space.at(x, read_y, z);
        }
      }
    }

    write_y++;
  }

  // claer layer y at write_y
  for (int y = write_y; y < SPACE_HEIGHT; ++y) {
    for (int x = 0; x < SPACE_WIDTH; ++x) {
      for (int z = 0; z < SPACE_DEPTH; ++z) {
        m_space.at(x, write_y, z).clear();
      }
    }
  }

  _updateDepthMap();
}

bool TetrisManager::_spawnPiece() {
  glm::ivec3 startPos = {SPACE_WIDTH / 2, SPACE_HEIGHT - 1, SPACE_DEPTH / 2};

  while (m_piecesQueue.size() < PIECES_QUEUE_CAP) {
    m_piecesQueue.emplace_back(_getRandomPieceType(m_level), startPos);
  }

  m_activePiece = m_piecesQueue.front();
  m_piecesQueue.pop_front();

  while (!_checkValidPiece(m_activePiece)) {
    glm::ivec3 currentPos = m_activePiece.getPosition();
    currentPos.y -= 1;
    m_activePiece.setPosition(currentPos);

    if (currentPos.y >= SPACE_HEIGHT + 4) {
      return false;
    }
  }

  return true;
}

void TetrisManager::_updateDepthMap() {
  for (int x = 0; x < SPACE_WIDTH; ++x) {
    for (int z = 0; z < SPACE_DEPTH; ++z) {
      m_depth_map[x][z] = 0;

      for (int y = SPACE_HEIGHT - 1; y >= 0; --y) {
        if (m_space.at(x, y, z).isOccupied()) {
          m_depth_map[x][z] = y;
          break;
        }
      }
    }
  }
}

// Returns relative distance to the dropped position
// can used with Tetromino::moveRelative, or tryMoveRelative
glm::ivec3 TetrisManager::_calculateDropOffset() {
  int max_floor_y = std::numeric_limits<int>::lowest();
  int min_relative_y = std::numeric_limits<int>::max();

  for (glm::ivec3 pos : m_activePiece.getGlobalPositions()) {
    max_floor_y = std::max(max_floor_y, m_depth_map[pos.x][pos.z]);
    min_relative_y = std::min(min_relative_y, pos.y);
  }

  int start_y = max_floor_y - min_relative_y;
  int current_y = m_activePiece.getPosition().y;

  for (int y = start_y; y <= current_y; ++y) {
    glm::ivec3 relative_pos(0, y, 0);

    if (_checkValidPiecePosition(m_activePiece.tryMoveRelative(relative_pos))) {
      return relative_pos;
    }
  }

  return glm::ivec3(0);
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

glm::ivec3 TetrisManager::_snapToGridAxis(glm::vec3 dir) {
  if (glm::length(dir) < 0.1f)
    return glm::ivec3(0);
  dir = glm::normalize(dir);

  float absX = std::abs(dir.x);
  float absY = std::abs(dir.y);
  float absZ = std::abs(dir.z);

  if (absX > absY && absX > absZ)
    return glm::ivec3(dir.x > 0 ? 1 : -1, 0, 0);
  if (absY > absX && absY > absZ)
    return glm::ivec3(0, dir.y > 0 ? 1 : -1, 0);
  return glm::ivec3(0, 0, dir.z > 0 ? 1 : -1);
}

std::generator<glm::ivec3>
TetrisManager::_tryApplyGlobalRotation(glm::ivec3 axis, bool clockwise) const {
  if (std::abs(axis.x) > 0)
    return m_activePiece.tryRotateX(axis.x > 0 ? clockwise : !clockwise);
  if (std::abs(axis.y) > 0)
    return m_activePiece.tryRotateY(axis.y > 0 ? clockwise : !clockwise);

  return m_activePiece.tryRotateZ(axis.z > 0 ? clockwise : !clockwise);
}

void TetrisManager::_applyGlobalRotation(glm::ivec3 axis, bool clockwise) {
  if (std::abs(axis.x) > 0)
    m_activePiece.rotateX(axis.x > 0 ? clockwise : !clockwise);
  else if (std::abs(axis.y) > 0)
    m_activePiece.rotateY(axis.y > 0 ? clockwise : !clockwise);
  else if (std::abs(axis.z) > 0)
    m_activePiece.rotateZ(axis.z > 0 ? clockwise : !clockwise);
}

BlockType TetrisManager::_getRandomPieceType(uint8_t level) {
  static std::mt19937 gen(std::random_device{}());

  std::vector<BlockType> pool;
  // Levels 0-2: Classic pieces
  pool.insert(pool.end(),
              {BlockType::Straight, BlockType::LeftSnake, BlockType::RightSnake,
               BlockType::Square, BlockType::LeftStep, BlockType::Pyramid,
               BlockType::RightStep});

  if (level >= 3) {
    // Levels 3-5: More advanced pieces
    pool.insert(pool.end(),
                {BlockType::Corner3D, BlockType::Pillar3D, BlockType::Stair3D});
  }

  if (level >= 6) {
    // Levels 6+: Very hard cross piece
    pool.push_back(BlockType::Cross3D);
  }

  std::uniform_int_distribution<size_t> dist(0, pool.size() - 1);
  return pool[dist(gen)];
}

void TetrisManager::_renderGrid(const Shader &shader, const glm::mat4 &view,
                                const glm::mat4 &proj) {
  shader.setMat4("u_model", glm::mat4(1.0f));
  shader.setVec4("u_color", glm::vec4(0.5f, 0.5f, 0.5f, 0.7f)); // Grey outline
  m_gridBox.render(view, proj);
}

void TetrisManager::_renderOnGridPiece(const Shader &shader) {
  for (int y = 0; y < SPACE_HEIGHT; ++y) {
    bool is_clearing = std::ranges::find(m_pendingClearLayers, y) !=
                       m_pendingClearLayers.end();

    for (int x = 0; x < SPACE_WIDTH; ++x) {
      for (int z = 0; z < SPACE_DEPTH; ++z) {
        const GridCell &cell = m_space.at(x, y, z);

        if (cell.isOccupied()) {
          glm::vec3 world_pos = m_space.gridToWorld(x, y, z);

          glm::vec4 color;

          if (is_clearing) {
            color = glm::vec4(TetrominoFactory::getColor(cell.type), 0.7);
          } else {
            color = glm::vec4(TetrominoFactory::getColor(cell.type), 1.0f);
          }

          _drawCell(world_pos, color, shader);
        }
      }
    }
  }
}

void TetrisManager::_renderActivePiece(const Shader &shader) {
  glm::vec4 active_piece_color(m_activePiece.getColor(), 1.0f);

  for (glm::ivec3 grid_pos : m_activePiece.getGlobalPositions()) {
    glm::vec3 world_pos =
        m_space.gridToWorld(grid_pos.x, grid_pos.y, grid_pos.z);
    _drawCell(world_pos, active_piece_color, shader);
  }
}

void TetrisManager::_renderGhostPiece(const Shader &shader) {
  glm::vec4 ghost_piece_color(m_activePiece.getColor(), 1.0f);
  glm::ivec3 ghost_relative_pos = _calculateDropOffset();

  for (glm::ivec3 grid_pos :
       m_activePiece.tryMoveRelative(ghost_relative_pos)) {
    glm::vec3 world_pos =
        m_space.gridToWorld(grid_pos.x, grid_pos.y, grid_pos.z);
    _drawCell(world_pos, ghost_piece_color, shader, true);
  }
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

// void TetrisManager::_renderStaticPiece(BlockType type, glm::vec3 world_pos,
//                                        const Shader &shader) {
//   switch (type) {
//   case BlockType::Ghost:
//   case BlockType::None:
//     return;
//   }
//
//   TetrominoData data = TetrominoFactory::getConfig(type);
//   glm::vec4 color = glm::vec4(data.color, 1.0f);
//
//   float time = (float)glfwGetTime();
//   glm::mat4 rotation =
//       glm::rotate(glm::mat4(1.0f), time, glm::vec3(0.2f, 1.0f, 0.0f));
//
//   for (const auto &offset : data.offsets) {
//     glm::mat4 model = glm::mat4(1.0f);
//     model = glm::translate(model, world_pos);
//     model = model * rotation;
//     model = glm::translate(model, glm::vec3(offset));
//
//     shader.setMat4("u_model", model);
//     shader.setBool("u_isGhost", false);
//     shader.setVec4("u_color", color);
//     shader.setFloat("u_time", time);
//     glDrawArrays(GL_TRIANGLES, 0, 36);
//   }
// }

void TetrisManager::_drawCell(glm::vec3 world_pos, glm::vec4 color,
                              const Shader &shader, bool is_ghost_piece) {

  glm::mat4 model = glm::translate(glm::mat4(1.0f), world_pos);
  shader.setMat4("u_model", model);
  shader.setVec4("u_color", color);
  shader.setFloat("u_time", glfwGetTime());
  shader.setFloat("u_isGhost", is_ghost_piece);

  glDrawArrays(GL_TRIANGLES, 0, 36);
}
