#include "helpers/math.hpp"
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
namespace SimpleEngine {
namespace Helper {
glm::vec3 Math::calculateTangent(glm::vec3 posA, glm::vec3 posB, glm::vec3 posC,
                                 glm::vec2 uvA, glm::vec2 uvB, glm::vec2 uvC) {
  glm::vec3 edge1 = posB - posA;
  glm::vec3 edge2 = posC - posA;
  glm::vec2 deltaUv1 = uvB - uvA;
  glm::vec2 deltaUv2 = uvC - uvA;

  float f = 1.0f / (deltaUv1.x * deltaUv2.y - deltaUv2.x * deltaUv1.y);

  glm::vec3 tangent;
}
} // namespace Helper
} // namespace SimpleEngine
