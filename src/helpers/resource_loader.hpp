#pragma once

#include "core/raw_texture.hpp"
#include "core/resource/mesh.hpp"
#include "core/resource/resource_manager.hpp"
#include "core/scene/scene_object.hpp"
#include "helpers/model_loader.hpp"
#include <cstdint>
#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Helper {
class ResourceLoader {
public:
  struct SceneNode {
    std::string name;
    glm::mat4 transformMat;
    std::vector<Core::Mesh::Vertex> vertices;
    std::vector<uint32_t> indices;

    bool hasAlbedo;
    Core::RawTexture rawAlbedoTexture;

    bool hasNormal;
    Core::RawTexture rawNormalTexture;
  };

  ResourceLoader() = delete;

  static void loadGltfScene(const std::string &basePath,
                            const std::string &name,
                            Core::SceneObject &sceneObject,
                            Core::ResourceManager &resourceManager);

  static void loadKtxTexture(const std::string &path,
                             Core::RawTexture &rawTexture);

private:
  static glm::mat4 getGltfNodeTransform(const tinygltf::Node &node);

  static void processGltfNode(const tinygltf::Model &model, int nodeIndex,
                              const glm::mat4 &parentTransformMat,
                              std::vector<SceneNode> &nodes,
                              const std::string &basePath);
};
} // namespace Helper
} // namespace SimpleEngine
