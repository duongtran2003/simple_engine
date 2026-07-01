#include "helpers/resource_loader.hpp"
#include "core/raw_texture.hpp"
#include "core/resource/mesh.hpp"
#include "core/resource/resource_manager.hpp"
#include "core/scene/scene_object.hpp"
#include "helpers/math.hpp"
#include "helpers/model_loader.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Helper {
glm::mat4 ResourceLoader::getGltfNodeTransform(const tinygltf::Node &node) {
  if (!node.matrix.empty()) {
    return glm::make_mat4(node.matrix.data());
  }

  glm::mat4 transform = glm::mat4(1.0f);
  if (!node.translation.empty()) {
    transform = glm::translate(transform, glm::vec3(node.translation[0],
                                                    node.translation[1],
                                                    node.translation[2]));
  }

  if (!node.rotation.empty()) {
    glm::quat q = glm::quat(node.rotation[3], node.rotation[0],
                            node.rotation[1], node.rotation[2]);
    transform = transform * glm::mat4_cast(q);
  }

  if (!node.scale.empty()) {
    transform = glm::scale(
        transform, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
  }

  return transform;
}

void ResourceLoader::loadKtxTexture(const std::string &path,
                                    Core::RawTexture &rawTexture) {}

void ResourceLoader::loadGltfScene(const std::string &basePath,
                                   const std::string &name,
                                   Core::SceneObject &sceneObject,
                                   Core::ResourceManager &resourceManager) {
  tinygltf::Model model;
  tinygltf::TinyGLTF loader;

  std::string error;
  std::string warning;

  std::filesystem::path pathToSceneEntry =
      std::filesystem::path(basePath) / name;

  bool ret = loader.LoadASCIIFromFile(&model, &error, &warning,
                                      pathToSceneEntry.string());
  if (!warning.empty()) {
    std::cout << "Helper::SceneLoader::loadGltfScene::WARNING: " + warning +
                     "\n";
  }

  if (!error.empty()) {
    std::cout << "Helper::SceneLoader::loadGltfScene::ERROR: " + error + "\n";
  }

  if (!ret) {
    throw std::runtime_error("Helper::SceneLoader::loadGltfScene::ERROR: "
                             "Failed to load glTF model.");
  }

  std::vector<SceneNode> sceneNodes;
  const tinygltf::Scene &scene =
      model.scenes[model.defaultScene >= 0 ? model.defaultScene : 0];

  for (int nodeIndex : scene.nodes) {
    processGltfNode(model, nodeIndex, glm::mat4(1.0f), sceneNodes, basePath);
  }
}

void ResourceLoader::processGltfNode(const tinygltf::Model &model,
                                     int nodeIndex,
                                     const glm::mat4 &parentTransformMat,
                                     std::vector<SceneNode> &nodes,
                                     const std::string &basePath) {
  if (nodeIndex < 0 || nodeIndex >= model.nodes.size()) {
    return;
  }
  const auto &node = model.nodes[nodeIndex];
  if (node.mesh >= 0 && node.mesh < model.meshes.size()) {
    const auto &mesh = model.meshes[node.mesh];
    glm::mat4 transformMat = parentTransformMat * getGltfNodeTransform(node);

    bool hasTangent = false;
    SceneNode currentNode{.name = node.name.empty() ? mesh.name : node.name,
                          .transformMat = transformMat,
                          .vertices = {},
                          .indices = {}};

    for (const auto &primitive : mesh.primitives) {
      const tinygltf::Accessor &indexAccessor =
          model.accessors[primitive.indices];
      const tinygltf::BufferView &indexBufferView =
          model.bufferViews[indexAccessor.bufferView];
      const tinygltf::Buffer &indexBuffer =
          model.buffers[indexBufferView.buffer];
      const unsigned char *indexData =
          &indexBuffer
               .data[indexBufferView.byteOffset + indexAccessor.byteOffset];
      size_t indexStride = indexAccessor.ByteStride(indexBufferView);
      for (size_t i = 0; i < indexAccessor.count; i++) {
        uint32_t index = 0;
        if (indexAccessor.componentType ==
            TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
          index =
              *reinterpret_cast<const uint16_t *>(indexData + i * indexStride);
        } else if (indexAccessor.componentType ==
                   TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
          index =
              *reinterpret_cast<const uint32_t *>(indexData + i * indexStride);
        } else if (indexAccessor.componentType ==
                   TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
          index =
              *reinterpret_cast<const uint8_t *>(indexData + i * indexStride);
        } else {
          throw std::runtime_error("SceneLoader::processGltfNode::ERROR: "
                                   "Unsupported index component type");
        }
        currentNode.indices.push_back(index);
      }

      const tinygltf::Accessor &positionAccessor =
          model.accessors[primitive.attributes.at("POSITION")];
      const tinygltf::BufferView &positionBufferView =
          model.bufferViews[positionAccessor.bufferView];
      const tinygltf::Buffer &positionBuffer =
          model.buffers[positionBufferView.buffer];
      size_t positionStride = positionAccessor.ByteStride(positionBufferView);

      const tinygltf::Accessor &normalAccessor =
          model.accessors[primitive.attributes.at("NORMAL")];
      const tinygltf::BufferView &normalBufferView =
          model.bufferViews[normalAccessor.bufferView];
      const tinygltf::Buffer &normalBuffer =
          model.buffers[normalBufferView.buffer];
      size_t normalStride = normalAccessor.ByteStride(normalBufferView);

      bool hasTexCoords =
          primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end();
      const tinygltf::Accessor *texCoordAccessor = nullptr;
      const tinygltf::BufferView *texCoordBufferView = nullptr;
      const tinygltf::Buffer *texCoordBuffer = nullptr;

      if (hasTexCoords) {
        texCoordAccessor =
            &model.accessors[primitive.attributes.at("TEXCOORD_0")];
        texCoordBufferView = &model.bufferViews[texCoordAccessor->bufferView];
        texCoordBuffer = &model.buffers[texCoordBufferView->buffer];
      }
      size_t texStride =
          hasTexCoords ? texCoordAccessor->ByteStride(*texCoordBufferView) : 0;

      hasTangent =
          primitive.attributes.find("TANGENT") != primitive.attributes.end();
      const tinygltf::Accessor *tangentAccessor = nullptr;
      const tinygltf::BufferView *tangentBufferView = nullptr;
      const tinygltf::Buffer *tangentBuffer = nullptr;

      if (hasTangent) {
        tangentAccessor = &model.accessors[primitive.attributes.at("TANGENT")];
        tangentBufferView = &model.bufferViews[tangentAccessor->bufferView];
        tangentBuffer = &model.buffers[tangentBufferView->buffer];
      }
      size_t tangentStride =
          hasTangent ? tangentAccessor->ByteStride(*tangentBufferView) : 0;

      for (size_t i = 0; i < positionAccessor.count; i++) {
        Core::Mesh::Vertex vertex{};
        const float *pos = reinterpret_cast<const float *>(
            &positionBuffer
                 .data[positionBufferView.byteOffset +
                       positionAccessor.byteOffset + i * positionStride]);
        vertex.position = {pos[0], pos[1], pos[2]};

        if (hasTexCoords) {
          const float *texCoord = reinterpret_cast<const float *>(
              &texCoordBuffer
                   ->data[texCoordBufferView->byteOffset +
                          texCoordAccessor->byteOffset + i * texStride]);
          vertex.uv = {texCoord[0], texCoord[1]};
        } else {
          vertex.uv = {0.0f, 0.0f};
        }

        if (hasTangent) {
          const float *tangent = reinterpret_cast<const float *>(
              &tangentBuffer
                   ->data[tangentBufferView->byteOffset +
                          tangentAccessor->byteOffset + i * tangentStride]);
          vertex.tangent = {tangent[0], tangent[1], tangent[2], tangent[3]};
        }

        const float *norm = reinterpret_cast<const float *>(
            &normalBuffer.data[normalBufferView.byteOffset +
                               normalAccessor.byteOffset + i * normalStride]);
        vertex.normal = {norm[0], norm[1], norm[2]};

        currentNode.vertices.push_back(vertex);
      }

      if (primitive.material >= 0) {
        const auto &material = model.materials[primitive.material];

        int albedoIndex = material.pbrMetallicRoughness.baseColorTexture.index;
        currentNode.hasAlbedo = false;
        if (albedoIndex >= 0 && albedoIndex < model.textures.size()) {
          currentNode.hasAlbedo = true;

          const tinygltf::Texture &texture = model.textures[albedoIndex];
          int imageIndex = texture.source;
          if (imageIndex >= 0 && imageIndex < model.images.size()) {
            const tinygltf::Image image = model.images[imageIndex];
            std::string textureUri = image.uri;
            std::filesystem::path texturePath =
                std::filesystem::path(basePath) /
                std::filesystem::path(textureUri);

            Core::RawTexture rawTexture;
            loadKtxTexture(texturePath.string(), rawTexture);
          }
        }

        int normalIndex = material.normalTexture.index;
        currentNode.hasNormal = false;
        if (normalIndex >= 0 && normalIndex < model.textures.size()) {
          currentNode.hasNormal = true;

          const tinygltf::Texture &texture = model.textures[normalIndex];
          int imageIndex = texture.source;
          if (imageIndex >= 0 && imageIndex < model.images.size()) {
            const tinygltf::Image image = model.images[imageIndex];
            std::string textureUri = image.uri;
            std::filesystem::path texturePath =
                std::filesystem::path(basePath) /
                std::filesystem::path(textureUri);

            Core::RawTexture rawTexture;
            loadKtxTexture(texturePath.string(), rawTexture);
          }
        }
      }
    }

    if (!hasTangent) {
      assert(currentNode.indices.size() % 3 == 0);
      for (size_t i = 0; i < currentNode.indices.size() - 2; i += 3) {
        glm::vec3 v1 = currentNode.vertices[currentNode.indices[i]].position;
        glm::vec3 v2 =
            currentNode.vertices[currentNode.indices[i + 1]].position;
        glm::vec3 v3 =
            currentNode.vertices[currentNode.indices[i + 2]].position;

        glm::vec2 uv1 = currentNode.vertices[currentNode.indices[i]].uv;
        glm::vec2 uv2 = currentNode.vertices[currentNode.indices[i + 1]].uv;
        glm::vec2 uv3 = currentNode.vertices[currentNode.indices[i + 2]].uv;

        glm::vec4 tangent = glm::vec4(
            Helper::Math::calculateTangent(v1, v2, v3, uv1, uv2, uv3), 1.0f);

        glm::vec3 v1v2 = v2 - v1;
        glm::vec3 v1v3 = v3 - v1;
        float area = glm::length(glm::cross(v1v2, v1v3));

        currentNode.vertices[currentNode.indices[i]].tangent =
            currentNode.vertices[currentNode.indices[i]].tangent +
            tangent * area;
        currentNode.vertices[currentNode.indices[i + 1]].tangent =
            currentNode.vertices[currentNode.indices[i + 1]].tangent +
            tangent * area;
        currentNode.vertices[currentNode.indices[i + 2]].tangent =
            currentNode.vertices[currentNode.indices[i + 2]].tangent +
            tangent * area;
      }

      for (auto &v : currentNode.vertices) {
        v.tangent = glm::normalize(v.tangent);
      }
    }
    nodes.push_back(currentNode);
    for (int childIndex : node.children) {
      processGltfNode(model, childIndex, currentNode.transformMat, nodes,
                      basePath);
    }
  }
}
} // namespace Helper
} // namespace SimpleEngine
