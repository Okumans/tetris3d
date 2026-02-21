#pragma once

#include <cstddef>
#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

enum class BlockType : uint8_t {
  None = 0,
  Straight,
  LeftSnake,
  RightSnake,
  Square,
  LeftStep,
  Pyramid,
  RightStep,
  Boundary,
  Ghost
};

struct GridCell {
  BlockType type = BlockType::None;

  bool isEmpty() const;
  bool isOccupied() const;
  void clear();
};

template <size_t WIDTH, size_t HEIGHT, size_t DEPTH> class TetrisSpace {
private:
  std::vector<GridCell> m_cells;

public:
  TetrisSpace();

  GridCell &at(int x, int y, int z);
  const GridCell &at(int x, int y, int z) const;
  bool checkInBound(int x, int y, int z) const;

  static glm::vec3 gridToWorld(int x, int y, int z);
};

// TetrisSpace implementation
template <size_t WIDTH, size_t HEIGHT, size_t DEPTH>
TetrisSpace<WIDTH, HEIGHT, DEPTH>::TetrisSpace()
    : m_cells(WIDTH * HEIGHT * DEPTH) {}

template <size_t WIDTH, size_t HEIGHT, size_t DEPTH>
GridCell &TetrisSpace<WIDTH, HEIGHT, DEPTH>::at(int x, int y, int z) {
  return m_cells[x + y * WIDTH + z * WIDTH * HEIGHT];
}

template <size_t WIDTH, size_t HEIGHT, size_t DEPTH>
const GridCell &TetrisSpace<WIDTH, HEIGHT, DEPTH>::at(int x, int y,
                                                      int z) const {
  return m_cells[x + y * WIDTH + z * WIDTH * HEIGHT];
}

template <size_t WIDTH, size_t HEIGHT, size_t DEPTH>
glm::vec3 TetrisSpace<WIDTH, HEIGHT, DEPTH>::gridToWorld(int x, int y, int z) {
  float worldX = (float)x - (float)WIDTH / 2.0f + 0.5f;
  float worldY = (float)y;
  float worldZ = (float)z - (float)DEPTH / 2.0f + 0.5f;

  return glm::vec3(worldX, worldY, worldZ);
}

template <size_t WIDTH, size_t HEIGHT, size_t DEPTH>
bool TetrisSpace<WIDTH, HEIGHT, DEPTH>::checkInBound(int x, int y,
                                                     int z) const {
  return (x >= 0 && x < WIDTH) && (y >= 0 && y < HEIGHT) &&
         (z >= 0 && z < DEPTH);
}
