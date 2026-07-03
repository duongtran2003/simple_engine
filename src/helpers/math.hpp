#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
namespace SimpleEngine {
namespace Helper {
class Math {
public:
  Math() = delete;
  static glm::vec3 calculateTangent(glm::vec3 posA, glm::vec3 posB,
                                    glm::vec3 posC, glm::vec2 uvA,
                                    glm::vec2 uvB, glm::vec2 uvC);

  static void extractTransformation(const glm::mat4 &transform,
                                    glm::vec3 &translation, glm::quat &rotation,
                                    glm::vec3 &scale);
};
} // namespace Helper
} // namespace SimpleEngine
