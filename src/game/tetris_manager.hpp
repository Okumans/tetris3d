#pragma once

#include "camera.h"
#include "game/space.hpp"
#include "game/tetris_ui_renderer.hpp"
#include "game/tetromino.hpp"
#include "glad/gl.h"
#include "glm/fwd.hpp"
#include "shader.h"
#include "ui/grid_box.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <generator>
#include <optional>
#include <vector>

template <typename R>
concept IVec3Range = std::ranges::input_range<R> &&
                     std::same_as<std::ranges::range_value_t<R>, glm::ivec3>;

class TetrisManager {
public:
  // --- Public Variables & Constants ---
  enum class GameState : uint8_t { FALLING, LOCKING, CLEARING, GAME_OVER };
  enum class RelativeDir : uint8_t { LEFT, RIGHT, FORWARD, BACK };
  enum class RelativeRotation : uint8_t { Y_AXIS, PITCH, ROLL };

  static const size_t SPACE_WIDTH = 10;
  static const size_t SPACE_HEIGHT = 20;
  static const size_t SPACE_DEPTH = 10;

  static const size_t PIECES_QUEUE_CAP = 3;
  static constexpr double MAX_DROP_DELAY = 0.05;
  static constexpr double MAX_LOCK_DELAY = 0.5;
  static constexpr double MAX_COLLASPE_DELAY = 0.2;
  static const int MAX_LOCK_RESETS = 15;

private:
  // --- State & Core Systems ---
  TetrisSpace<SPACE_WIDTH, SPACE_HEIGHT, SPACE_DEPTH> m_space;
  GridBox m_gridBox{SPACE_WIDTH, SPACE_HEIGHT, SPACE_DEPTH};
  Tetromino m_activePiece;
  std::deque<Tetromino> m_piecesQueue;
  std::optional<Tetromino> m_heldPiece;

  GLuint m_vao = 0;
  GLuint m_vbo = 0;

  GameState m_state = GameState::FALLING;
  bool m_isSoftDropping = false;
  bool m_canHold = true;
  uint8_t m_level = 0;
  uint64_t m_score = 0;
  uint64_t m_linesCleared = 0;

  std::vector<int> m_pendingClearLayers;
  std::array<std::array<int, SPACE_WIDTH>, SPACE_DEPTH> m_depth_map;

  double m_dropTimer = 0.0;
  double m_lockTimer = 0.0;
  double m_collapseTimer = 0.0;
  int m_lockMoveResetCount = 0;
  double m_baseDropDelay = 2.0;
  double m_delayDecreaseRate = 0.13;

public:
  // --- Lifecycle & Main Loop ---
  TetrisManager();
  ~TetrisManager();

  void update(double delta_time);
  void render(double delta_time, const Camera &camera);

  // --- Input Actions ---
  bool moveRelative(RelativeDir direction, const Camera &camera);
  bool rotateRelative(RelativeRotation type, bool clockwise,
                      const Camera &camera);
  void hardDrop();
  void hold();
  void setSoftDrop(bool is_soft_dropping);

  // --- State Accessors ---
  const Tetromino &getActivePiece() const;
  const std::deque<Tetromino> &getPiecesQueue() const;
  const std::optional<Tetromino> &getHold() const;
  GameState getState() const { return m_state; }
  uint64_t getScore() const { return m_score; }
  uint8_t getLevel() const { return m_level; }
  uint64_t getVAO() const { return m_vao; }

private:
  // --- Logic & Progression ---
  bool _spawnPiece();
  void _finalizeSpawn();
  void _commit();
  void _performCommitSequence();
  void _checkLayerClears(std::vector<int> &layers_cleared);
  void _collapseLayers(const std::vector<int> &layers_cleared);
  static BlockType _getRandomPieceType(uint8_t level);

  // --- Movement & Collision ---
  bool _moveDown();
  glm::ivec3 _calculateDropOffset();
  void _updateDepthMap();
  bool _checkValidPiece(const Tetromino &moved_piece) const;
  bool _checkValidPiecePosition(IVec3Range auto &&positions) const;

  // --- Math & Rotation Helpers ---
  glm::ivec3 _snapToGridAxis(glm::vec3 direction);
  void _applyGlobalRotation(glm::ivec3 axis, bool clockwise);
  std::generator<glm::ivec3> _tryApplyGlobalRotation(glm::ivec3 axis,
                                                     bool clockwise) const;

  // --- Internal Rendering ---
  void _setupBuffers();
  void _renderGrid(const Shader &shader, const glm::mat4 &view,
                   const glm::mat4 &proj);
  void _renderOnGridPiece(const Shader &shader);
  void _renderActivePiece(const Shader &shader);
  void _renderGhostPiece(const Shader &shader);
  void _drawCell(glm::vec3 world_pos, glm::vec4 color, const Shader &shader,
                 bool is_ghost = false);
};
