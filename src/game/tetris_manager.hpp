#pragma once

#include "camera.h"
#include "game/space.hpp"
#include "game/tetromino.hpp"
#include "glad/gl.h"
#include "shader.h"
#include "ui/grid_box.hpp"
#include <cstdint>
#include <deque>

class TetrisManager {
public:
  static const size_t SPACE_WIDTH = 10;
  static const size_t SPACE_HEIGHT = 20;
  static const size_t SPACE_DEPTH = 10;

private:
  TetrisSpace<SPACE_WIDTH, SPACE_HEIGHT, SPACE_DEPTH> m_space;
  GridBox m_gridBox{SPACE_WIDTH, SPACE_HEIGHT, SPACE_DEPTH};

  // States
  Tetromino m_activePiece;
  std::deque<Tetromino> m_nextPieces;
  bool m_isSoftDropping = false;
  bool m_swapWithNextPieceQuota = true;
  uint8_t m_level = 0;
  uint64_t m_score = 0;
  double m_dropTimer = 0;

  // Properties
  double m_baseDropDelay = 1.0;
  double m_maxDropDelay = 0.05;
  double m_delayDecreaseRate = 0.1;

  GLuint m_vao = 0;
  GLuint m_vbo = 0;

public:
  TetrisManager();
  ~TetrisManager();
  void render(const Camera &camera, double delta_time);
  void update(double delta_time);

  // Control activePiece
  bool rotateX(bool clockwise = true);
  bool rotateY(bool clockwise = true);
  bool rotateZ(bool clockwise = true);
  bool moveLeft();
  bool moveRight();
  bool moveInward();
  bool moveOutward();
  void swapWithNextPiece();

private:
  void _commit();

  bool _moveDown();
  bool _checkValidAction(const Tetromino &moved_piece) const;

  static Tetromino _get_random_piece();
  void _setupBuffers();
  void _drawCell(glm::vec3 world_pos, glm::vec3 color, const Shader &shader);
};
