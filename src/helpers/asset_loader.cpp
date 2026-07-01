#include "helpers/asset_loader.hpp"
#include "core/raw_texture.hpp"
#include "core/resource/mesh.hpp"
#include "helpers/math.hpp"
#include "helpers/model_loader.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/geometric.hpp>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Helper {
tinygltf::Model
AssetLoader::loadTinyGltfModelFromBinary(const std::string &path) {
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

  return model;
}

tinygltf::Model
AssetLoader::loadTinyGltfModelFromASCII(const std::string &path) {
  tinygltf::Model model;
  tinygltf::TinyGLTF loader;

  std::string error;
  std::string warning;

  bool ret = loader.LoadASCIIFromFile(&model, &error, &warning, path);
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

  return model;
}

Core::RawTexture
AssetLoader::loadTextureFromTinyGltfModel(tinygltf::Model &model,
                                          int textureIndex) {
  if (textureIndex < 0 || textureIndex >= model.textures.size()) {
    throw std::runtime_error(
        "ModelLoader::loadTexture::ERROR: Out of bound. Texture index: " +
        std::to_string(textureIndex) +
        ". Model textures num: " + std::to_string(model.textures.size()));
  }

  const tinygltf::Texture &texture = model.textures[textureIndex];
  int imageIndex = texture.source;

  if (imageIndex < 0 || imageIndex >= model.images.size()) {
    throw std::runtime_error(
        "ModelLoader::loadTexture::ERROR: Out of bound. Image index: " +
        std::to_string(imageIndex) +
        ". Model images num: " + std::to_string(model.images.size()));
  }

  const tinygltf::Image &image = model.images[imageIndex];
  Core::RawTexture rawTexture{.pixels = image.image,
                              .width = static_cast<uint32_t>(image.width),
                              .height = static_cast<uint32_t>(image.height),
                              .componentCount =
                                  static_cast<uint32_t>(image.component)};

  int samplerIndex = texture.sampler;
  if (samplerIndex < 0 || samplerIndex >= model.samplers.size()) {
    std::cout
        << ("ModelLoader::loadTexture::WARNING: Out of bound. Sampler index: " +
            std::to_string(samplerIndex) +
            ". Model samplers num: " + std::to_string(model.samplers.size()) +
            ". Using default sampler.\n");
  }

  if (samplerIndex >= 0) {
    const tinygltf::Sampler &sampler = model.samplers[samplerIndex];
    rawTexture.magFilter = mapGltfFilter(sampler.magFilter);
    rawTexture.minFilter = mapGltfFilter(sampler.minFilter);
    rawTexture.wrapS = mapGltfWrap(sampler.wrapS);
    rawTexture.wrapT = mapGltfWrap(sampler.wrapT);
  } else {
    rawTexture.magFilter = Core::TextureFilter::Nearest;
    rawTexture.minFilter = Core::TextureFilter::Nearest;
    rawTexture.wrapS = Core::TextureWrapMode::Repeat;
    rawTexture.wrapT = Core::TextureWrapMode::Repeat;
  }

  return rawTexture;
}

Core::TextureFilter AssetLoader::mapGltfFilter(int gltfFilter) {
  if (gltfFilter == 9728 || gltfFilter == 9984 || gltfFilter == 9986) {
    return Core::TextureFilter::Nearest;
  }

  return Core::TextureFilter::Linear;
}

Core::TextureWrapMode AssetLoader::mapGltfWrap(int gltfWrap) {
  if (gltfWrap == 33071) {
    return Core::TextureWrapMode::ClampToEdge;
  }
  if (gltfWrap == 33648) {
    return Core::TextureWrapMode::MirroredRepeat;
  }

  return Core::TextureWrapMode::Repeat;
}

void AssetLoader::loadGltfModelFromBinary(
    const std::string &path, const std::string &name,
    std::vector<Core::Mesh::Vertex> &vertices, std::vector<uint32_t> &indices,
    std::vector<Core::RawTexture> &textures) {
  std::string pathToModel =
      (std::filesystem::path(path) / std::filesystem::path(name)).string();
  tinygltf::Model model = loadTinyGltfModelFromBinary(pathToModel);

  vertices.clear();
  indices.clear();
  textures.clear();
  bool hasTangent = false;

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

      uint32_t baseVertex = vertices.size();

      size_t positionStride = positionAccessor.ByteStride(positionBufferView);
      size_t normalStride = normalAccessor.ByteStride(normalBufferView);
      size_t texStride =
          hasTexCoords ? texCoordAccessor->ByteStride(*texCoordBufferView) : 0;
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

  if (!hasTangent) {
    assert(indices.size() % 3 == 0);
    for (size_t i = 0; i < indices.size() - 2; i += 3) {
      glm::vec3 v1 = vertices[indices[i]].position;
      glm::vec3 v2 = vertices[indices[i + 1]].position;
      glm::vec3 v3 = vertices[indices[i + 2]].position;

      glm::vec2 uv1 = vertices[indices[i]].uv;
      glm::vec2 uv2 = vertices[indices[i + 1]].uv;
      glm::vec2 uv3 = vertices[indices[i + 2]].uv;

      glm::vec4 tangent = glm::vec4(
          Helper::Math::calculateTangent(v1, v2, v3, uv1, uv2, uv3), 1.0f);

      glm::vec3 v1v2 = v2 - v1;
      glm::vec3 v1v3 = v3 - v1;
      float area = glm::length(glm::cross(v1v2, v1v3));

      vertices[indices[i]].tangent =
          vertices[indices[i]].tangent + tangent * area;
      vertices[indices[i + 1]].tangent =
          vertices[indices[i + 1]].tangent + tangent * area;
      vertices[indices[i + 2]].tangent =
          vertices[indices[i + 2]].tangent + tangent * area;
    }

    for (auto &v : vertices) {
      v.tangent = glm::normalize(v.tangent);
    }
  }
}
} // namespace Helper
} // namespace SimpleEngine
