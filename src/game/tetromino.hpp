#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "game/space.hpp"

class Tetromino {
private:
  BlockType m_type;
  glm::ivec3 m_position;
  std::vector<glm::ivec3> m_offsets;
  glm::vec3 m_color;

public:
  Tetromino(BlockType type, glm::ivec3 startPos);

  // Control methods
  void rotateX(bool clockwise = true);
  void rotateY(bool clockwise = true);
  void rotateZ(bool clockwise = true);
  void moveLeft();
  void moveRight();
  void moveInward();
  void moveOutward();

  std::vector<glm::ivec3> getGlobalPositions() const;
  glm::vec3 getColor();
};

struct TetrominoData {
  BlockType type;
  std::vector<glm::ivec3> offsets;
  glm::vec3 color;
};

class TetrominoFactory {
public:
  static TetrominoData getConfig(BlockType type);
  static glm::vec3 getColor(BlockType type);
};
