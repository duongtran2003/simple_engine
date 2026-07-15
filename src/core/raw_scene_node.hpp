#pragma once

#include "core/material.hpp"
#include "core/resource/mesh.hpp"
#include <cstdint>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Core {

class RawSceneNode {
public:
  enum class TextureIndexer { Albedo = 0, Normal = 1, MetallicRoughness = 2 };
  static constexpr int TEX_INDEXER_COUNT = 3;

  std::string name;

  // Mesh component
  std::vector<Core::Mesh::Vertex> vertices;
  std::vector<uint32_t> indices;
  std::vector<std::string> textureNames;

  Material::PbrProperty pbrProperty;

  // Transform component
  glm::vec3 translation = glm::vec3(0.0f);
  glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  glm::vec3 scale = glm::vec3(1.0f);
};
} // namespace Core
} // namespace SimpleEngine
