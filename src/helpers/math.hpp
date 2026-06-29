#pragma once

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
namespace SimpleEngine {
namespace Helper {
class Math {
public:
  Math() = delete;
  static glm::vec3 calculateTangent(glm::vec3 posA, glm::vec3 posB,
                                    glm::vec3 posC, glm::vec2 uvA,
                                    glm::vec2 uvB, glm::vec2 uvC);
};
} // namespace Helper
} // namespace SimpleEngine
