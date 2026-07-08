#pragma once

#include "core/raw_scene_node.hpp"
#include "core/raw_texture.hpp"
#include "core/resource/mesh.hpp"
#include "enums/texture.hpp"
#include <cstdint>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <tiny_gltf.h>
#include <unordered_map>
#include <vector>

namespace SimpleEngine {
namespace Helper {
class AssetLoader {
public:
  enum class TextureLoadMode { fromBinary, fromUri };

  static void loadGltfModelFromBinary(const std::string &path,
                                      const std::string &name,
                                      std::vector<Core::Mesh::Vertex> &vertices,
                                      std::vector<uint32_t> &indices,
                                      std::vector<Core::RawTexture> &textures);

  static void loadGltfSceneFromGltf(const std::string &path,
                                    const std::string &name,
                                    std::vector<Core::RawSceneNode> &nodes);

  static void loadKtxTexture(const std::string &path,
                             Core::RawTexture &rawTexture);

  static void loadImageTexture(const std::string &path,
                               Core::RawTexture &rawTexture);

private:
  static tinygltf::Model loadTinyGltfModelFromBinary(const std::string &path);
  static tinygltf::Model loadTinyGltfModelFromASCII(const std::string &path);
  static void loadTextureFromTinyGltfModel(
      const tinygltf::Model &model, int textureIndex,
      Core::RawTexture &rawTexture, TextureLoadMode loadMode,
      std::optional<std::string_view> scenePath = std::nullopt);

  // Mappers
  static Enums::Texture::Filter mapGltfFilter(int gltfFilter);
  static Enums::Texture::Wrap mapGltfWrap(int gltfWrap);

  static glm::mat4 getNodeTransform(const tinygltf::Node &node);
  static void
  processSceneNode(const tinygltf::Model &model, int nodeIndex,
                   const glm::mat4 &parentTransform,
                   std::vector<Core::RawSceneNode> &nodes,
                   std::unordered_map<std::string, int> &texturesMap,
                   const std::string &scenePath);
};
} // namespace Helper
} // namespace SimpleEngine
