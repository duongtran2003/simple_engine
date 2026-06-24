#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION

#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>

#include "core/resource/mesh.hpp"
#include "helpers/model_loader.hpp"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Helper {
void ModelLoader::loadglTF(const std::string &path,
                           std::vector<Core::Mesh::Vertex> &vertices,
                           std::vector<uint32_t> &indices) {
  tinygltf::Model model;
  tinygltf::TinyGLTF loader;

  std::string error;
  std::string warning;

  bool ret = loader.LoadBinaryFromFile(&model, &error, &warning, path);
  if (!warning.empty()) {
    std::cout << "Helper::ModelLoader::loadglTF::WARNING: " + warning + "\n";
  }

  if (!error.empty()) {
    std::cout << "Helper::ModelLoader::loadglTF::ERROR: " + error + "\n";
  }

  if (!ret) {
    throw std::runtime_error(
        "Helper::ModelLoader::loadglTF::ERROR: Failed to load glTF model.");
  }

  vertices.clear();
  indices.clear();

  for (const auto &mesh : model.meshes) {
    for (const auto &primitive : mesh.primitives) {
      const tinygltf::Accessor &indexAccessor =
          model.accessors[primitive.indices];
      const tinygltf::BufferView &indexBufferView =
          model.bufferViews[indexAccessor.bufferView];
      const tinygltf::Buffer &indexBuffer =
          model.buffers[indexBufferView.buffer];

      const tinygltf::Accessor &positionAccessor =
          model.accessors[primitive.attributes.at("POSITION")];
      const tinygltf::BufferView &positionBufferView =
          model.bufferViews[positionAccessor.bufferView];
      const tinygltf::Buffer &positionBuffer =
          model.buffers[positionBufferView.buffer];

      const tinygltf::Accessor &normalAccessor =
          model.accessors[primitive.attributes.at("NORMAL")];
      const tinygltf::BufferView &normalBufferView =
          model.bufferViews[normalAccessor.bufferView];
      const tinygltf::Buffer &normalBuffer =
          model.buffers[normalBufferView.buffer];

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

      uint32_t baseVertex = vertices.size();

      size_t positionStride = positionAccessor.ByteStride(positionBufferView);
      size_t normalStride = normalAccessor.ByteStride(normalBufferView);
      size_t texStride =
          hasTexCoords ? texCoordAccessor->ByteStride(*texCoordBufferView) : 0;

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

        const float *norm = reinterpret_cast<const float *>(
            &normalBuffer.data[normalBufferView.byteOffset +
                               normalAccessor.byteOffset + i * normalStride]);
        vertex.normal = {norm[0], norm[1], norm[2]};

        vertices.push_back(vertex);
      }

      const unsigned char *indexData =
          &indexBuffer
               .data[indexBufferView.byteOffset + indexAccessor.byteOffset];
      size_t indexCount = indexAccessor.count;
      size_t indexStride = 0;

      if (indexAccessor.componentType ==
          TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
        indexStride = sizeof(uint16_t);
      } else if (indexAccessor.componentType ==
                 TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
        indexStride = sizeof(uint32_t);
      } else if (indexAccessor.componentType ==
                 TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
        indexStride = sizeof(uint8_t);
      } else {
        throw std::runtime_error("Unsupported index component type");
      }

      indices.reserve(indices.size() + indexCount);

      for (size_t i = 0; i < indexCount; i++) {
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
        }

        indices.push_back(baseVertex + index);
      }
    }
  }
}
} // namespace Helper
} // namespace SimpleEngine
