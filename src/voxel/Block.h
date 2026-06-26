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

inline glm::vec3 GetColorByLOD(int lod) {
  switch (lod) {
  case 0:
    return {0.1, 0.4, 0.5};
  case 1:
    return {0.1, 0.3, 0.6};
  case 2:
    return {0.2, 0.2, 0.7};
  case 3:
    return {0.4, 0.1, 0.7};
  case 4:
    return {0.7, 0.2, 0.5};
  case 5:
    return {0.3, 0.8, 0.4};
  default:
    return {0.9, 0.1, 0.3};
  }
}
