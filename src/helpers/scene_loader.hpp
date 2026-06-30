#pragma once

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
class SceneLoader {
public:
  struct RawNode {
    std::string name;
    glm::mat4 transformMat;
    std::vector<Core::Mesh::Vertex> vertices;
    std::vector<uint32_t> indices;
  };

  SceneLoader() = delete;

  static void loadGltfScene(const std::string &path, const std::string &name,
                            Core::SceneObject &sceneObject,
                            Core::ResourceManager &resourceManager);

  static void processNode(const tinygltf::Model &model, int nodeIndex,
                          const glm::mat4 &parentTransformMat,
                          std::vector<RawNode> &nodes);

private:
  static glm::mat4 getNodeTransform(const tinygltf::Node &node);
};
} // namespace Helper
} // namespace SimpleEngine
