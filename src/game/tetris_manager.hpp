#pragma once

#include "camera.h"
#include "game/space.hpp"
#include "game/tetromino.hpp"
#include "glad/gl.h"
#include "shader.h"
#include <deque>

class TetrisManager {
public:
  static const size_t SPACE_WIDTH = 10;
  static const size_t SPACE_HEIGHT = 20;
  static const size_t SPACE_DEPTH = 10;

private:
  TetrisSpace<SPACE_WIDTH, SPACE_HEIGHT, SPACE_WIDTH> m_space;
  Tetromino m_activePiece;
  std::deque<Tetromino> m_nextPieces;

  bool swapWithNextPieceQuota = true;

  GLuint m_vao = 0;
  GLuint m_vbo = 0;

public:
  TetrisManager();
  ~TetrisManager();
  void render(const Camera &camera, double delta_time);

  // Control activePiece
  void rotateX(bool clockwise = true);
  void rotateY(bool clockwise = true);
  void rotateZ(bool clockwise = true);
  void moveLeft();
  void moveRight();
  void moveInward();
  void moveOutward();
  void swapWithNextPiece();

private:
  void _onPlace();

  static Tetromino _get_random_piece();
  void _setupBuffers();
  void _drawCell(int x, int y, int z, const Shader &shader);
};
