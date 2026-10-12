#pragma once

#include "core/raw_scene_node.hpp"
#include "core/raw_texture.hpp"
#include "core/resource2/raw_model_data.hpp"
#include "enums/texture.hpp"
#include <cstdint>
#include <fastgltf/types.hpp>
#include <filesystem>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace SimpleEngine {
namespace Helper {
class AssetLoader {
public:
  enum class TextureLoadMode { FromBinary, FromUri };

  static void loadGltfSceneFromGltf(
      const std::string &path, const std::string &name,
      std::vector<Core::RawSceneNode> &nodes,
      std::unordered_map<std::string, Core::RawTexture> &texturesMap);

  static void loadImageTexture(const std::string &path,
                               Core::RawTexture &rawTexture);
  static void LoadImage(const std::string &path,
                        std::vector<unsigned char> &pixels, int &width,
                        int &height, int &channels);

  static std::optional<Core::RawModelData>
  LoadGltf(const std::filesystem::path &filePath);

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
  static void processSceneNode(
      const tinygltf::Model &model, int nodeIndex,
      const glm::mat4 &parentTransform, std::vector<Core::RawSceneNode> &nodes,
      std::unordered_map<std::string, Core::RawTexture> &texturesMap,
      const std::string &scenePath);

  static void ParseGltf(const std::filesystem::path &filePath,
                        fastgltf::Asset &asset);
  static void ExtractGltfSamplers(fastgltf::Asset &asset,
                                  std::vector<Core::RawSamplerData> &samplers);
  static void ExtractGltfImages(fastgltf::Asset &asset,
                                std::vector<Core::RawImageData> &images,
                                const std::filesystem::path &filePath);
  static void ExtractGltfTextures(fastgltf::Asset &asset,
                                  std::vector<Core::RawTextureData> &textures);
  static void
  ExtractGltfMaterials(fastgltf::Asset &asset,
                       std::vector<Core::RawMaterialData> &materials);
  static void
  ExtractMeshes(fastgltf::Asset &asset,
                std::vector<std::vector<uint32_t>> &meshToPrimitives,
                Core::RawModelData &modelData, const std::filesystem::path &filePath);
};
} // namespace Helper
} // namespace SimpleEngine
