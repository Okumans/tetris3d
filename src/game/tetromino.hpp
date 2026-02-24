#pragma once

#include <generator>
#include <glm/glm.hpp>
#include <vector>

#include "game/space.hpp"
#include "glm/fwd.hpp"

class Tetromino {
public:
  struct ColumnOffset {
    int x, z, minY;
  };

private:
  BlockType m_type;
  glm::ivec3 m_position;
  glm::vec3 m_color;
  std::vector<glm::ivec3> m_offsets;

public:
  Tetromino(BlockType type, glm::ivec3 startPos);

  // Control methods
  std::generator<glm::ivec3> tryRotateX(bool clockwise = true) const;
  void rotateX(bool clockwise = true);

  std::generator<glm::ivec3> tryRotateY(bool clockwise = true) const;
  void rotateY(bool clockwise = true);

  std::generator<glm::ivec3> tryRotateZ(bool clockwise = true) const;
  void rotateZ(bool clockwise = true);

  std::generator<glm::ivec3> tryMoveRelative(glm::ivec3 direction) const;
  void moveRelative(glm::ivec3 direction);

  void setPosition(glm::ivec3 pos);

  std::generator<glm::ivec3> getGlobalPositions() const;
  const std::vector<glm::ivec3> &getOffsets() const;
  glm::vec3 getColor() const;
  glm::ivec3 getPosition() const;
  BlockType getType() const;
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
