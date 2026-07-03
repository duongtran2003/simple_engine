#include "helpers/math.hpp"
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>

namespace SimpleEngine {
namespace Helper {
glm::vec3 Math::calculateTangent(glm::vec3 posA, glm::vec3 posB, glm::vec3 posC,
                                 glm::vec2 uvA, glm::vec2 uvB, glm::vec2 uvC) {
  // E1 = vec(AB) => E1 = vec(B) - vec(A) = P1
  glm::vec3 P1 = posB - posA;
  // E2 = vec(BC) => E2 = vec(C) - vec(B) = P2
  glm::vec3 P2 = posC - posB;

  // delta V1 = vB - vA = A
  float A = uvB.x - uvA.x;
  // delta U1 = uB - uA = B
  float B = uvB.y - uvA.y;

  // delta V2 = vC - vB = C
  float C = uvC.x - uvB.x;
  // delta U2 = uC - uB = D
  float D = uvC.y - uvB.y;

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

void Math::extractTransformation(const glm::mat4 &transform,
                                 glm::vec3 &translation, glm::quat &rotation,
                                 glm::vec3 &scale) {
  translation = glm::vec3(transform[3]);
  scale = glm::vec3(glm::length(glm::vec3(transform[0])),
                    glm::length(glm::vec3(transform[1])),
                    glm::length(glm::vec3(transform[2])));
  glm::mat4 rotationMat = transform;
  rotationMat[0] = transform[0] / scale.x;
  rotationMat[1] = transform[1] / scale.y;
  rotationMat[2] = transform[2] / scale.z;
  rotationMat[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

  glm::quat rotationQuat = glm::quat_cast(rotationMat);
  rotation = rotationQuat;
}
} // namespace Helper
} // namespace SimpleEngine
