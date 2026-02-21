#include "tetromino.hpp"
#include "game/space.hpp"
#include "glm/fwd.hpp"
#include <vector>

Tetromino::Tetromino(BlockType type, glm::ivec3 startPos)
    : m_type(type), m_position(startPos) {
  TetrominoData config = TetrominoFactory::getConfig(type);
  m_offsets = config.offsets;
  m_color = config.color;
}

// Control methods
void Tetromino::rotateX(bool clockwise) {}
void Tetromino::rotateY(bool clockwise) {}
void Tetromino::rotateZ(bool clockwise) {}

void Tetromino::moveLeft() { m_position.x--; }
void Tetromino::moveRight() { m_position.x++; }
void Tetromino::moveInward() { m_position.z++; }
void Tetromino::moveOutward() { m_position.z--; }
void Tetromino::moveDown() { m_position.y--; }
void Tetromino::moveUp() { m_position.y++; }

std::vector<glm::ivec3> Tetromino::getGlobalPositions() const {
  std::vector<glm::ivec3> globalPositions(m_offsets.size());

  for (size_t i = 0; i < m_offsets.size(); ++i) {
    globalPositions[i] = m_offsets[i] + m_position;
  }

  return globalPositions;
}

glm::vec3 Tetromino::getColor() const { return m_color; }

BlockType Tetromino::getType() const { return m_type; }

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
