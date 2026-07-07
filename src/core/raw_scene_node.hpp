#pragma once

#include "core/raw_texture.hpp"
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
  enum class TextureIndexer { Albedo = 0, Normal = 1 };
  std::string name;

  // Mesh component
  std::vector<Core::Mesh::Vertex> vertices;
  std::vector<uint32_t> indices;
  std::vector<Core::RawTexture> textures;

  // Transform component
  glm::vec3 translation = glm::vec3(0.0f);
  glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  glm::vec3 scale = glm::vec3(1.0f);
};
} // namespace Core
} // namespace SimpleEngine
