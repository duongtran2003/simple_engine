#pragma once

#include "core/raw_texture.hpp"
#include "core/resource/mesh.hpp"

#include <cstdint>
#include <string>
#include <tiny_gltf.h>
#include <vector>

namespace SimpleEngine {
namespace Helper {
class ModelLoader {
public:
  ModelLoader() = delete;

  static void loadglTF(const std::string &path,
                       std::vector<Core::Mesh::Vertex> &vertices,
                       std::vector<uint32_t> &indices,
                       std::vector<Core::RawTexture> &textures);

  static Core::TextureFilter mapGltfFilter(int gltfFilter);
  static Core::TextureWrapMode mapGltfWrap(int gltfWrap);

private:
  static Core::RawTexture loadTexture(tinygltf::Model &model, int textureIndex);
};
} // namespace Helper
} // namespace SimpleEngine
