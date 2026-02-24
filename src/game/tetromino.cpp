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

// Control methods
std::generator<glm::ivec3> Tetromino::tryRotateX(bool clockwise) const {
  for (glm::ivec3 offset : m_offsets) {
    int y = offset.y;
    int z = offset.z;

    if (clockwise) {
      offset.y = z;
      offset.z = -y;
    } else {
      offset.y = -z;
      offset.z = y;
    }

    co_yield offset + m_position;
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

std::generator<glm::ivec3> Tetromino::tryRotateY(bool clockwise) const {
  for (glm::ivec3 offset : m_offsets) {
    int x = offset.x;
    int z = offset.z;

    if (clockwise) {
      offset.x = -z;
      offset.z = x;
    } else {
      offset.x = z;
      offset.z = -x;
    }

    co_yield offset + m_position;
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

std::generator<glm::ivec3> Tetromino::tryRotateZ(bool clockwise) const {
  for (glm::ivec3 offset : m_offsets) {
    int x = offset.x;
    int y = offset.y;

    if (clockwise) {
      offset.x = y;
      offset.y = -x;
    } else {
      offset.x = -y;
      offset.y = x;
    }

    co_yield offset + m_position;
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

void Tetromino::setPosition(glm::ivec3 pos) { m_position = pos; }

glm::vec3 Tetromino::getColor() const { return m_color; }
glm::ivec3 Tetromino::getPosition() const { return m_position; }
BlockType Tetromino::getType() const { return m_type; }

const std::vector<glm::ivec3> &Tetromino::getOffsets() const {
  return m_offsets;
}

glm::vec3 TetrominoFactory::getColor(BlockType type) {
  switch (type) {
  case BlockType::Straight:
    return {0.45f, 0.85f, 0.90f}; // Soft Sky Blue (Cyan)
  case BlockType::Square:
    return {0.95f, 0.90f, 0.45f}; // Pale Mustard (Yellow)
  case BlockType::Pyramid:
    return {0.75f, 0.55f, 0.85f}; // Soft Lavender (Purple)
  case BlockType::LeftSnake:
    return {0.40f, 0.60f, 0.95f}; // Cornflower Blue
  case BlockType::RightSnake:
    return {1.00f, 0.70f, 0.45f}; // Peach/Creamsicle (Orange)
  case BlockType::LeftStep:
    return {0.55f, 0.85f, 0.55f}; // Sage Green
  case BlockType::RightStep:
    return {0.90f, 0.50f, 0.55f}; // Dusty Rose (Red)
  case BlockType::Ghost:
    return {0.75f, 0.75f, 0.80f}; // Muted Slate (Gray)
  case BlockType::Corner3D:
    return {0.95f, 0.65f, 0.75f}; // Pink Sherbet
  case BlockType::Pillar3D:
    return {0.60f, 0.95f, 0.85f}; // Mint Crystal
  case BlockType::Cross3D:
    return {1.00f, 0.85f, 0.60f}; // Soft Apricot
  case BlockType::Stair3D:
    return {0.80f, 0.80f, 0.95f}; // Periwinkle
  default:
    return {0.95f, 0.95f, 0.95f}; // Off-White
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
  case BlockType::Corner3D:
    return {type, {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}}, color};
  case BlockType::Pillar3D:
    return {type,
            {{0, 0, 0},
             {1, 0, 0},
             {0, 1, 0},
             {1, 1, 0},
             {0, 0, 1},
             {1, 0, 1},
             {0, 1, 1},
             {1, 1, 1}},
            color};
  case BlockType::Cross3D:
    return {type,
            {{0, 0, 0},
             {1, 0, 0},
             {-1, 0, 0},
             {0, 1, 0},
             {0, -1, 0},
             {0, 0, 1},
             {0, 0, -1}},
            color};
  case BlockType::Stair3D:
    return {type, {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {1, 1, 1}}, color};
  case BlockType::Debug5x5:
    return {type,
            {{-2, -2, 0}, {-1, -2, 0}, {0, -2, 0}, {1, -2, 0}, {2, -2, 0},
             {-2, -1, 0}, {-1, -1, 0}, {0, -1, 0}, {1, -1, 0}, {2, -1, 0},
             {-2, 0, 0},  {-1, 0, 0},  {0, 0, 0},  {1, 0, 0},  {2, 0, 0},
             {-2, 1, 0},  {-1, 1, 0},  {0, 1, 0},  {1, 1, 0},  {2, 1, 0},
             {-2, 2, 0},  {-1, 2, 0},  {0, 2, 0},  {1, 2, 0},  {2, 2, 0}},
            color};
  default:
    return {BlockType::None, {}, color};
  }
}
