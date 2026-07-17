#include "helpers/math.hpp"
#include "core/resource/mesh.hpp"
#include <cstddef>
#include <glm/common.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/trigonometric.hpp>
#include <vector>

namespace SimpleEngine {
namespace Helper {
void Math::calculateTangent(Core::Mesh::Vertex &v1, Core::Mesh::Vertex &v2,
                            Core::Mesh::Vertex &v3,
                            std::vector<glm::vec3> &accumB, size_t i1,
                            size_t i2, size_t i3) {

  glm::vec3 posA = v1.position;
  glm::vec3 posB = v2.position;
  glm::vec3 posC = v3.position;

  glm::vec2 uvA = v1.uv;
  glm::vec2 uvB = v2.uv;
  glm::vec2 uvC = v3.uv;

  glm::vec3 AB = posB - posA;
  glm::vec3 AC = posC - posA;
  glm::vec3 BC = posC - posB;

  float deltaUAB = uvB.x - uvA.x;
  float deltaVAB = uvB.y - uvA.y;

  float deltaUAC = uvC.x - uvA.x;
  float deltaVAC = uvC.y - uvA.y;

  float D = deltaUAB * deltaVAC - deltaVAB * deltaUAC;
  if (glm::abs(D) < 0.000001f) {
    D = 0.000001f;
  }
  glm::vec3 Dx = AB * deltaVAC - deltaVAB * AC;
  glm::vec3 T = glm::normalize(Dx / D);

  glm::vec3 Dy = deltaUAB * AC - AB * deltaUAC;
  glm::vec3 B = glm::normalize(Dy / D);

  glm::vec3 normalizedAB = glm::normalize(AB);
  glm::vec3 normalizedAC = glm::normalize(AC);
  glm::vec3 normalizedBC = glm::normalize(BC);

  float angleA = glm::degrees(glm::acos(
      glm::dot(glm::normalize(normalizedAB), glm::normalize(normalizedAC))));
  float angleB = glm::degrees(glm::acos(
      glm::dot(glm::normalize(-normalizedAB), glm::normalize(normalizedBC))));
  float angleC = glm::degrees(glm::acos(
      glm::dot(glm::normalize(-normalizedAC), glm::normalize(-normalizedBC))));

  v1.tangent += angleA * glm::vec4(T, 0.0f);
  v2.tangent += angleB * glm::vec4(T, 0.0f);
  v3.tangent += angleC * glm::vec4(T, 0.0f);

  accumB[i1] += angleA * B;
  accumB[i2] += angleB * B;
  accumB[i3] += angleC * B;
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
