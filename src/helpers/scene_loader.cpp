#include "helpers/scene_loader.hpp"
#include "core/resource/mesh.hpp"
#include "helpers/math.hpp"
#include "helpers/model_loader.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/geometric.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Helper {
void SceneLoader::processNode(const tinygltf::Model &model, int nodeIndex,
                              const glm::mat4 &parentTransformMat,
                              std::vector<RawNode> &nodes) {
  if (nodeIndex < 0 || nodeIndex >= model.nodes.size()) {
    return;
  }
  const auto &node = model.nodes[nodeIndex];
  if (node.mesh >= 0 && node.mesh < model.meshes.size()) {
    const auto &mesh = model.meshes[node.mesh];
    glm::mat4 transformMat = parentTransformMat * getNodeTransform(node);

    RawNode currentNode{.name = node.name.empty() ? mesh.name : node.name,
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
          throw std::runtime_error("SceneLoader::processNode::ERROR: "
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

      bool hasTangent =
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
        if (!hasTangent) {
          assert(currentNode.indices.size() % 3 == 0);
          for (size_t i = 0; i < currentNode.indices.size() - 2; i += 3) {
            glm::vec3 v1 =
                currentNode.vertices[currentNode.indices[i]].position;
            glm::vec3 v2 =
                currentNode.vertices[currentNode.indices[i + 1]].position;
            glm::vec3 v3 =
                currentNode.vertices[currentNode.indices[i + 2]].position;

            glm::vec2 uv1 = currentNode.vertices[currentNode.indices[i]].uv;
            glm::vec2 uv2 = currentNode.vertices[currentNode.indices[i + 1]].uv;
            glm::vec2 uv3 = currentNode.vertices[currentNode.indices[i + 2]].uv;

            glm::vec4 tangent = glm::vec4(
                Helper::Math::calculateTangent(v1, v2, v3, uv1, uv2, uv3),
                1.0f);

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
      }
    }

    nodes.push_back(currentNode);
  }
}
} // namespace Helper
} // namespace SimpleEngine
