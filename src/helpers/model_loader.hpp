#pragma once

#include "core/component/mesh_component.hpp"
#include "core/raw_texture.hpp"
#include "core/resource/mesh.hpp"
#include "core/resource/resource_manager.hpp"

#include <cstdint>
#include <string>
#include <tiny_gltf.h>
#include <vector>

namespace SimpleEngine {
namespace Helper {
class ModelLoader {
public:
  ModelLoader() = delete;

  static void loadGltfMesh(const std::string &path, const std::string &name,
                           Core::MeshComponent &component,
                           Core::ResourceManager &resourceManager);

  static void loadGltfMeshData(const std::string &path,
                               std::vector<Core::Mesh::Vertex> &vertices,
                               std::vector<uint32_t> &indices);

  static void loadGltfMeshTextures(const std::string &path,
                                   std::vector<Core::RawTexture> &textures);

private:
  static Core::RawTexture loadTexture(tinygltf::Model &model, int textureIndex);

  static tinygltf::Model loadGltfModel(const std::string &path);

  static Core::TextureFilter mapGltfFilter(int gltfFilter);
  static Core::TextureWrapMode mapGltfWrap(int gltfWrap);
};
} // namespace Helper
} // namespace SimpleEngine
