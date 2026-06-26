#pragma once
#include <cstdint>
#include <glm/glm.hpp>

enum class BlockType : uint8_t { Air = 0, Solid = 1 };

inline glm::vec3 BlockColor(uint8_t id) {
  switch (static_cast<BlockType>(id)) {
  default: {
    return {0.7f, 0.7f, 0.7f};
  }
  }
}

inline bool IsAir(uint8_t id) {
  return id == static_cast<uint8_t>(BlockType::Air);
}

inline bool IsSolid(uint8_t id) {
  return id != static_cast<uint8_t>(BlockType::Air);
}
