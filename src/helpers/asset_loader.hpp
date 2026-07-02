#pragma once

#include "core/raw_texture.hpp"
#include "core/resource/mesh.hpp"
#include <cstdint>
#include <string>
#include <tiny_gltf.h>
#include <vector>

namespace SimpleEngine {
namespace Helper {
class AssetLoader {
public:
  static void loadGltfModelFromBinary(const std::string &path,
                                      const std::string &name,
                                      std::vector<Core::Mesh::Vertex> &vertices,
                                      std::vector<uint32_t> &indices,
                                      std::vector<Core::RawTexture> &textures);

  static void loadKtxTexture(const std::string &path,
                             Core::RawTexture &rawTexture);

private:
  static tinygltf::Model loadTinyGltfModelFromBinary(const std::string &path);
  static tinygltf::Model loadTinyGltfModelFromASCII(const std::string &path);
  static void loadTextureFromTinyGltfModel(tinygltf::Model &model,
                                           int textureIndex,
                                           Core::RawTexture &rawTexture);

  // Mappers
  static Core::TextureFilter mapGltfFilter(int gltfFilter);
  static Core::TextureWrapMode mapGltfWrap(int gltfWrap);
};
} // namespace Helper
} // namespace SimpleEngine
