#include "space.hpp"

bool GridCell::isEmpty() const { return type == BlockType::None; }

bool GridCell::isOccupied() const { return type != BlockType::None; }

void GridCell::clear() { type = BlockType::None; }
