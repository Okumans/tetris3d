#include "tetromino.hpp"
#include "game/space.hpp"
#include "glm/fwd.hpp"
#include <generator>
#include <vector>

Tetromino::Tetromino(BlockType type, glm::ivec3 startPos)
    : m_type(type), m_position(startPos) {
  TetrominoData config = TetrominoFactory::getConfig(type);
  m_offsets = config.offsets;
  m_color = config.color;
}

// Control methods
void Tetromino::rotateX(bool clockwise) {
  for (auto &offset : m_offsets) {
    int y = offset.y;
    int z = offset.z;
    if (clockwise) {
      offset.y = z;
      offset.z = -y;
    } else {
      offset.y = -z;
      offset.z = y;
    }
  }
}

void Tetromino::rotateY(bool clockwise) {
  for (auto &offset : m_offsets) {
    int x = offset.x;
    int z = offset.z;
    if (clockwise) {
      offset.x = -z;
      offset.z = x;
    } else {
      offset.x = z;
      offset.z = -x;
    }
  }
}

void Tetromino::rotateZ(bool clockwise) {
  for (auto &offset : m_offsets) {
    int x = offset.x;
    int y = offset.y;
    if (clockwise) {
      offset.x = y;
      offset.y = -x;
    } else {
      offset.x = -y;
      offset.y = x;
    }
  }
}

void Tetromino::moveRelative(glm::ivec3 direction) { m_position += direction; }

std::generator<glm::ivec3>
Tetromino::tryMoveRelative(glm::ivec3 direction) const {
  for (const auto &off : m_offsets) {
    co_yield off + m_position + direction;
  }
}

std::generator<glm::ivec3> Tetromino::getGlobalPositions() const {
  for (const auto &off : m_offsets) {
    co_yield off + m_position;
  }
}

glm::vec3 Tetromino::getColor() const { return m_color; }
glm::ivec3 Tetromino::getPosition() const { return m_position; }
BlockType Tetromino::getType() const { return m_type; }

const std::vector<glm::ivec3> &Tetromino::getOffsets() const {
  return m_offsets;
}

glm::vec3 TetrominoFactory::getColor(BlockType type) {
  switch (type) {
  case BlockType::Straight:
    return {0.0f, 1.0f, 1.0f}; // Cyan
  case BlockType::Square:
    return {1.0f, 1.0f, 0.0f}; // Yellow
  case BlockType::Pyramid:
    return {0.5f, 0.0f, 0.5f}; // Purple
  case BlockType::LeftSnake:
    return {0.0f, 0.0f, 1.0f}; // Blue
  case BlockType::RightSnake:
    return {1.0f, 0.5f, 0.0f}; // Orange
  case BlockType::LeftStep:
    return {0.0f, 1.0f, 0.0f}; // Green
  case BlockType::RightStep:
    return {1.0f, 0.0f, 0.0f}; // Red
  case BlockType::Ghost:
    return {0.5f, 0.5f, 0.5f}; // Gray
  default:
    return {1.0f, 1.0f, 1.0f}; // White
  }
}

TetrominoData TetrominoFactory::getConfig(BlockType type) {
  glm::vec3 color = getColor(type);

  switch (type) {
  case BlockType::Straight:
    return {type, {{0, 0, 0}, {-1, 0, 0}, {1, 0, 0}, {2, 0, 0}}, color};
  case BlockType::Square:
    return {type, {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0}}, color};
  case BlockType::Pyramid:
    return {type, {{0, 0, 0}, {-1, 0, 0}, {1, 0, 0}, {0, 1, 0}}, color};
  case BlockType::LeftSnake:
    return {type, {{0, 0, 0}, {-1, 0, 0}, {1, 0, 0}, {-1, 1, 0}}, color};
  case BlockType::RightSnake:
    return {type, {{0, 0, 0}, {-1, 0, 0}, {1, 0, 0}, {1, 1, 0}}, color};
  case BlockType::LeftStep:
    return {type, {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {-1, 1, 0}}, color};
  case BlockType::RightStep:
    return {type, {{0, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {1, 1, 0}}, color};
  default:
    return {BlockType::None, {}, color};
  }
}
