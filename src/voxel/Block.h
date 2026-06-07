#pragma once
#include <cstdint>
#include <glm/glm.hpp>

enum class BlockType : uint8_t {
  Air = 0,
  Grass = 1,
  Stone = 2,
};

inline glm::vec3 blockColor(uint8_t id) {
  switch (static_cast<BlockType>(id)) {
  case BlockType::Grass: {
    return {0.0f, 1.0f, 0.0f};
  }
  case BlockType::Stone: {
    return {0.5f, 0.5f, 0.5f};
  }
  default: {
    return {1.0f, 1.0f, 1.0f};
  }
  }
}
