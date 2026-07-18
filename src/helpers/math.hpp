#pragma once

#include "core/resource/mesh.hpp"
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>

namespace SimpleEngine {
namespace Helper {
class Math {
public:
  Math() = delete;
  static void calculateTangent(Core::Mesh::Vertex &v1, Core::Mesh::Vertex &v2,
                               Core::Mesh::Vertex &v3);

  static void extractTransformation(const glm::mat4 &transform,
                                    glm::vec3 &translation, glm::quat &rotation,
                                    glm::vec3 &scale);
};
} // namespace Helper
} // namespace SimpleEngine
