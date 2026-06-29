#include "helpers/math.hpp"
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>

namespace SimpleEngine {
namespace Helper {
glm::vec3 Math::calculateTangent(glm::vec3 posA, glm::vec3 posB, glm::vec3 posC,
                                 glm::vec2 uvA, glm::vec2 uvB, glm::vec2 uvC) {
  // E1 = vec(AB) => E1 = vec(B) - vec(A) = P1
  glm::vec3 P1 = posB - posA;
  // E2 = vec(BC) => E2 = vec(C) - vec(B) = P2
  glm::vec3 P2 = posC - posB;

  // delta V1 = vB - vA = A
  float A = uvB.y - uvA.y;
  // delta U1 = uB - uA = B
  float B = uvB.x - uvA.x;

  // delta V2 = vC - vB = C
  float C = uvC.y - uvB.y;
  // delta U2 = uC - uB = D
  float D = uvC.x - uvB.x;

  // det = |A  B|
  //       |C  D|
  float det = A * D - B * C;
  // detX = |P1  B|
  //        |P2  D|
  glm::vec3 detX = P1 * D - B * P2;
  // x represents tangent vector T
  glm::vec3 x = detX / det;

  return glm::normalize(x);
}
} // namespace Helper
} // namespace SimpleEngine
