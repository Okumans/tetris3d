#pragma once

#include "camera.h"
#include "game/space.hpp"
#include "game/tetromino.hpp"
#include "glad/gl.h"
#include "glm/fwd.hpp"
#include "shader.h"
#include "ui/grid_box.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>

template <typename R>
concept IVec3Range = std::ranges::input_range<R> &&
                     std::same_as<std::ranges::range_value_t<R>, glm::ivec3>;

class TetrisManager {
public:
  enum class GameState : uint8_t { FALLING, LOCKING, CLEARING, GAME_OVER };
  enum class RelativeDir : uint8_t { LEFT, RIGHT, FORWARD, BACK };
  enum class RelativeRotation : uint8_t { Y_AXIS, PITCH, ROLL };

  static const size_t SPACE_WIDTH = 10;
  static const size_t SPACE_HEIGHT = 20;
  static const size_t SPACE_DEPTH = 10;

  static const size_t NEXT_PIECES_CAP = 3;
  static constexpr const double MAX_DROP_DELAY = 0.05;
  static constexpr const double MAX_LOCK_DELAY = 0.5;
  static constexpr const double MAX_COLLASPE_DELAY = 0.2;
  static const int MAX_LOCK_RESETS = 15;

private:
  TetrisSpace<SPACE_WIDTH, SPACE_HEIGHT, SPACE_DEPTH> m_space;
  GridBox m_gridBox{SPACE_WIDTH, SPACE_HEIGHT, SPACE_DEPTH};

  const Camera &m_camera;

  // States
  Tetromino m_activePiece;
  GameState m_state = GameState::FALLING;
  std::deque<Tetromino> m_nextPieces;
  bool m_isSoftDropping = false;
  std::optional<Tetromino> m_heldPiece;
  bool m_canHold = true;
  uint8_t m_level = 0;
  uint64_t m_score = 0;

  std::vector<int> m_pendingClearLayers;

  // timers / counters for gamestate tracking
  double m_dropTimer = 0;
  double m_lockTimer = 0.0;
  double m_collapseTimer = 0.0;
  int m_lockMoveResetCount = 0;

  // Properties
  double m_baseDropDelay = 1.0;
  double m_delayDecreaseRate = 0.1;

  std::array<std::array<int, SPACE_WIDTH>, SPACE_DEPTH> m_depth_map;

  GLuint m_vao = 0;
  GLuint m_vbo = 0;

public:
  TetrisManager(const Camera &camera);
  ~TetrisManager();
  void render(double delta_time);
  void update(double delta_time);

  bool rotateRelative(RelativeRotation type, bool clockwise);
  bool moveRelative(RelativeDir direction);

  void hold();
  void hardDrop();

  void setSoftDrop(bool is_soft_dropping);

private:
  void _commit();
  void _performCommitSequence();
  void _finalizeSpawn();
  bool _spawnPiece();

  void _checkLayerClears(std::vector<int> &layers_cleared);
  void _collapseLayers(const std::vector<int> &layers_cleared);

  glm::ivec3 _calculateDropOffset();
  bool _moveDown();
  void _updateDepthMap();

  bool _checkValidPiece(const Tetromino &moved_piece) const;
  bool _checkValidPiecePosition(IVec3Range auto &&positions) const;

  glm::ivec3 _snapToGridAxis(glm::vec3 direction);
  void _applyGlobalRotation(glm::ivec3 axis, bool clockwise);
  std::generator<glm::ivec3> _tryApplyGlobalRotation(glm::ivec3 axis,
                                                     bool clockwise) const;

  static Tetromino _getRandomPiece(glm::ivec3 spawn_grid_pos);
  void _setupBuffers();
  void _renderStaticPiece(BlockType type, glm::vec3 world_pos,
                          const Shader &shader);
  void _drawCell(glm::vec3 world_pos, glm::vec4 color, const Shader &shader);
};
