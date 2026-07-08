#include "helpers/asset_loader.hpp"
#include "core/raw_scene_node.hpp"
#include "core/raw_texture.hpp"
#include "core/resource/mesh.hpp"
#include "enums/texture.hpp"
#include "helpers/math.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>
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
    std::cout << "Helper::AssetLoader::LoadBinaryFromFile::WARNING: " +
                     warning + "\n";
  }

  if (!error.empty()) {
    std::cout << "Helper::AssetLoader::LoadBinaryFromFile::ERROR: " + error +
                     "\n";
  }

  if (!ret) {
    throw std::runtime_error("Helper::AssetLoader::LoadBinaryFromFile::ERROR: "
                             "Failed to load glTF model.");
  }

  return model;
}

tinygltf::Model
AssetLoader::loadTinyGltfModelFromASCII(const std::string &path) {
  tinygltf::Model model;
  tinygltf::TinyGLTF loader;

  loader.SetImageLoader([](tinygltf::Image *, const int, std::string *,
                           std::string *, int, int, const unsigned char *, int,
                           void *) -> bool { return true; },
                        nullptr);

  std::string error;
  std::string warning;

  bool ret = loader.LoadASCIIFromFile(&model, &error, &warning, path);
  if (!warning.empty()) {
    std::cout << "Helper::AssetLoader::LoadASCIIFromFile::WARNING: " + warning +
                     "\n";
  }

  if (!error.empty()) {
    std::cout << "Helper::AssetLoader::LoadASCIIFromFile::ERROR: " + error +
                     "\n";
  }

  if (!ret) {
    throw std::runtime_error("Helper::AssetLoader::LoadASCIIFromFile::ERROR: "
                             "Failed to load glTF model.");
  }

  return model;
}

void AssetLoader::loadTextureFromTinyGltfModel(
    const tinygltf::Model &model, int textureIndex,
    Core::RawTexture &rawTexture, TextureLoadMode loadMode,
    std::optional<std::string_view> scenePath) {
  if (textureIndex < 0 || textureIndex >= model.textures.size()) {
    throw std::runtime_error(
        "AssetLoader::loadTextureFromTinyGltfModel::ERROR: Out of bound. "
        "Texture index: " +
        std::to_string(textureIndex) +
        ". Model textures num: " + std::to_string(model.textures.size()));
  }

  const tinygltf::Texture &texture = model.textures[textureIndex];
  int imageIndex = texture.source;

  if (imageIndex < 0 || imageIndex >= model.images.size()) {
    throw std::runtime_error(
        "AssetLoader::loadTextureFromTinyGltfModel::ERROR: Out of bound. Image "
        "index: " +
        std::to_string(imageIndex) +
        ". Model images num: " + std::to_string(model.images.size()));
  }

  const tinygltf::Image &image = model.images[imageIndex];
  if (loadMode == TextureLoadMode::fromBinary) {
    rawTexture.pixels = image.image;
    rawTexture.width = static_cast<uint32_t>(image.width);
    rawTexture.height = static_cast<uint32_t>(image.height);
    rawTexture.componentCount = static_cast<uint32_t>(image.component);
  } else if (loadMode == TextureLoadMode::fromUri) {
    if (!scenePath.has_value()) {
      throw std::runtime_error(
          "AssetLoader::loadTextureFromTinyGltfModel::ERROR: No scene path is "
          "provided to load texture from uri.");
    }
    std::string texturePath =
        (std::filesystem::path(scenePath.value()) / image.uri).string();
    loadImageTexture(texturePath, rawTexture);
  }

  int samplerIndex = texture.sampler;
  if (samplerIndex < 0 || samplerIndex >= model.samplers.size()) {
    std::cout << ("AssetLoader::loadTextureFromTinyGltfModel::WARNING: Out of "
                  "bound. Sampler index: " +
                  std::to_string(samplerIndex) + ". Model samplers num: " +
                  std::to_string(model.samplers.size()) +
                  ". Using default sampler.\n");
  }

  if (samplerIndex >= 0) {
    const tinygltf::Sampler &sampler = model.samplers[samplerIndex];
    rawTexture.magFilter = mapGltfFilter(sampler.magFilter);
    rawTexture.minFilter = mapGltfFilter(sampler.minFilter);
    rawTexture.wrapS = mapGltfWrap(sampler.wrapS);
    rawTexture.wrapT = mapGltfWrap(sampler.wrapT);
  } else {
    rawTexture.magFilter = Enums::Texture::Filter::Nearest;
    rawTexture.minFilter = Enums::Texture::Filter::Nearest;
    rawTexture.wrapS = Enums::Texture::Wrap::Repeat;
    rawTexture.wrapT = Enums::Texture::Wrap::Repeat;
  }
}

Enums::Texture::Filter AssetLoader::mapGltfFilter(int gltfFilter) {
  if (gltfFilter == 9728 || gltfFilter == 9984 || gltfFilter == 9986) {
    return Enums::Texture::Filter::Nearest;
  }

  return Enums::Texture::Filter::Linear;
}

Enums::Texture::Wrap AssetLoader::mapGltfWrap(int gltfWrap) {
  if (gltfWrap == 33071) {
    return Enums::Texture::Wrap::ClampToEdge;
  }
  if (gltfWrap == 33648) {
    return Enums::Texture::Wrap::MirroredRepeat;
  }

  return Enums::Texture::Wrap::Repeat;
}

glm::mat4 AssetLoader::getNodeTransform(const tinygltf::Node &node) {
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
    glm::quat quat = glm::quat(node.rotation[3], node.rotation[0],
                               node.rotation[1], node.rotation[2]);
    transform = transform * glm::mat4_cast(quat);
  }
  if (!node.scale.empty()) {
    transform = glm::scale(
        transform, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
  }

  return transform;
}

void AssetLoader::processSceneNode(
    const tinygltf::Model &model, int nodeIndex,
    const glm::mat4 &parentTransform, std::vector<Core::RawSceneNode> &nodes,
    std::unordered_map<std::string, int> &texturesMap,
    const std::string &scenePath) {
  if (nodeIndex < 0 || nodeIndex >= model.nodes.size()) {
    return;
  }

  const auto &currentNode = model.nodes[nodeIndex];
  glm::mat4 currentTransform = parentTransform * getNodeTransform(currentNode);

  if (currentNode.mesh >= 0 && currentNode.mesh < model.meshes.size()) {
    const auto &mesh = model.meshes[currentNode.mesh];

    uint32_t currentMeshIndex = 0;
    for (const auto &primitive : mesh.primitives) {
      currentMeshIndex += 1;
      Core::RawSceneNode rawSceneNode;
      rawSceneNode.name =
          mesh.name.empty() ? currentNode.name + "_mesh_" : mesh.name;
      rawSceneNode.name += "_" + std::to_string(currentMeshIndex);
      Math::extractTransformation(currentTransform, rawSceneNode.translation,
                                  rawSceneNode.rotation, rawSceneNode.scale);

      // Reading attributes
      const auto &positionAccessor =
          model.accessors[primitive.attributes.at("POSITION")];
      const auto &positionBufferView =
          model.bufferViews[positionAccessor.bufferView];
      const auto &positionBuffer = model.buffers[positionBufferView.buffer];

      size_t positionStride = positionAccessor.ByteStride(positionBufferView);

      const auto &normalAccessor =
          model.accessors[primitive.attributes.at("NORMAL")];
      const auto &normalBufferView =
          model.bufferViews[normalAccessor.bufferView];
      const auto &normalBuffer = model.buffers[normalBufferView.buffer];

      size_t normalStride = normalAccessor.ByteStride(normalBufferView);

      bool hasUv =
          primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end();
      bool hasTangent =
          primitive.attributes.find("TANGENT") != primitive.attributes.end();

      const tinygltf::Accessor *uvAccessor = nullptr;
      const tinygltf::BufferView *uvBufferView = nullptr;
      const tinygltf::Buffer *uvBuffer = nullptr;
      size_t uvStride = 0;
      if (hasUv) {
        uvAccessor = &model.accessors[primitive.attributes.at("TEXCOORD_0")];
        uvBufferView = &model.bufferViews[uvAccessor->bufferView];
        uvBuffer = &model.buffers[uvBufferView->buffer];
        uvStride = uvAccessor->ByteStride(*uvBufferView);
      }

      const tinygltf::Accessor *tangentAccessor = nullptr;
      const tinygltf::BufferView *tangentBufferView = nullptr;
      const tinygltf::Buffer *tangentBuffer = nullptr;
      size_t tangentStride = 0;
      if (hasTangent) {
        tangentAccessor = &model.accessors[primitive.attributes.at("TANGENT")];
        tangentBufferView = &model.bufferViews[tangentAccessor->bufferView];
        tangentBuffer = &model.buffers[tangentBufferView->buffer];
        tangentStride = tangentAccessor->ByteStride(*tangentBufferView);
      }

      for (size_t i = 0; i < positionAccessor.count; i++) {
        Core::Mesh::Vertex v;

        const float *position = reinterpret_cast<const float *>(
            &positionBuffer
                 .data[positionBufferView.byteOffset +
                       positionAccessor.byteOffset + i * positionStride]);
        v.position = {position[0], position[1], position[2]};

        const float *normal = reinterpret_cast<const float *>(
            &normalBuffer.data[normalBufferView.byteOffset +
                               normalAccessor.byteOffset + i * normalStride]);
        v.normal = {normal[0], normal[1], normal[2]};

        if (hasUv) {
          const float *uv = reinterpret_cast<const float *>(
              &uvBuffer->data[uvBufferView->byteOffset +
                              uvAccessor->byteOffset + i * uvStride]);
          v.uv = {uv[0], uv[1]};
        }

        if (hasTangent) {
          const float *tangent = reinterpret_cast<const float *>(
              &tangentBuffer
                   ->data[tangentBufferView->byteOffset +
                          tangentAccessor->byteOffset + i * tangentStride]);
          v.tangent = {tangent[0], tangent[1], tangent[2], tangent[3]};
        }

        rawSceneNode.vertices.push_back(v);
      }

      // Reading indices
      const auto &indexAccessor = model.accessors[primitive.indices];
      const auto &indexBufferView = model.bufferViews[indexAccessor.bufferView];
      const auto &indexBuffer = model.buffers[indexBufferView.buffer];

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
        }

        rawSceneNode.indices.push_back(index);
      }

      if (!hasTangent) {
        assert(rawSceneNode.indices.size() % 3 == 0);
        for (auto &v : rawSceneNode.vertices) {
          v.tangent = glm::vec4(0.0f);
        }

        for (size_t i = 0; i < rawSceneNode.indices.size() - 2; i += 3) {
          glm::vec3 v1 =
              rawSceneNode.vertices[rawSceneNode.indices[i]].position;
          glm::vec3 v2 =
              rawSceneNode.vertices[rawSceneNode.indices[i + 1]].position;
          glm::vec3 v3 =
              rawSceneNode.vertices[rawSceneNode.indices[i + 2]].position;

          glm::vec2 uv1 = rawSceneNode.vertices[rawSceneNode.indices[i]].uv;
          glm::vec2 uv2 = rawSceneNode.vertices[rawSceneNode.indices[i + 1]].uv;
          glm::vec2 uv3 = rawSceneNode.vertices[rawSceneNode.indices[i + 2]].uv;

          glm::vec4 tangent = glm::vec4(
              Helper::Math::calculateTangent(v1, v2, v3, uv1, uv2, uv3), 0.0f);

          glm::vec3 v1v2 = v2 - v1;
          glm::vec3 v1v3 = v3 - v1;
          float degreeAngle = glm::degrees(
              glm::acos(glm::dot(glm::normalize(v1v2), glm::normalize(v1v3))));

          rawSceneNode.vertices[rawSceneNode.indices[i]].tangent =
              rawSceneNode.vertices[rawSceneNode.indices[i]].tangent +
              tangent * degreeAngle;
          rawSceneNode.vertices[rawSceneNode.indices[i + 1]].tangent =
              rawSceneNode.vertices[rawSceneNode.indices[i + 1]].tangent +
              tangent * degreeAngle;
          rawSceneNode.vertices[rawSceneNode.indices[i + 2]].tangent =
              rawSceneNode.vertices[rawSceneNode.indices[i + 2]].tangent +
              tangent * degreeAngle;
        }

        for (auto &v : rawSceneNode.vertices) {
          if (glm::length(glm::vec3(v.tangent)) > 0.0f) {
            v.tangent = glm::vec4(glm::vec3(glm::normalize(v.tangent)), 1.0f);
          } else {
            v.tangent = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
          }
        }
      }

      if (primitive.material >= 0) {
        rawSceneNode.textures.resize(2);
        const auto &material = model.materials[primitive.material];

        int albedoIndex = material.pbrMetallicRoughness.baseColorTexture.index;
        const std::vector<double> &colorFactor =
            material.pbrMetallicRoughness.baseColorFactor;
        int normalIndex = material.normalTexture.index;

        // Has either texture or base color valur to fallback to
        if (albedoIndex >= 0 || !colorFactor.empty()) {
          Core::RawTexture albedo = {};
          albedo.name = material.name + "_" +
                        std::to_string(primitive.material) + "_abledo";

          if (albedoIndex >= 0) {
            auto it = texturesMap.find(albedo.name);
            if (it == texturesMap.end()) {
              loadTextureFromTinyGltfModel(model, albedoIndex, albedo,
                                           TextureLoadMode::fromUri, scenePath);
              texturesMap[albedo.name] = 1;
            } else {
              albedo.hasLoadedImage = true;
            }
          }

          albedo.colorSpace = Enums::Texture::ColorSpace::NonLinear;

          if (!colorFactor.empty()) {
            albedo.pbrProperty.baseColorFactor = {
                colorFactor[0], colorFactor[1], colorFactor[2], colorFactor[3]};
          } else {
            albedo.pbrProperty.baseColorFactor = glm::vec4(1.0f);
          }
          albedo.isValid = true;

          size_t albedoIdx =
              static_cast<size_t>(Core::RawSceneNode::TextureIndexer::Albedo);
          rawSceneNode.textures[albedoIdx] = albedo;
        }

        if (normalIndex >= 0) {
          Core::RawTexture normal = {};
          normal.name = material.name + "_" +
                        std::to_string(primitive.material) + "_normal";
          auto it = texturesMap.find(normal.name);
          if (it == texturesMap.end()) {
            loadTextureFromTinyGltfModel(model, normalIndex, normal,
                                         TextureLoadMode::fromUri, scenePath);
            // Flipping the green channel of the normal map
            if (!normal.pixels.empty() && normal.componentCount >= 3) {
              size_t totalPixels = normal.width * normal.height;
              for (size_t i = 0; i < totalPixels; i++) {
                size_t offset = i * normal.componentCount;
                normal.pixels[offset + 1] = 255 - normal.pixels[offset + 1];
              }
            }

            texturesMap[normal.name] = 1;
          } else {
            normal.hasLoadedImage = true;
          }
          normal.colorSpace = Enums::Texture::ColorSpace::Linear;
          normal.isValid = true;

          size_t normalIdx =
              static_cast<size_t>(Core::RawSceneNode::TextureIndexer::Normal);
          rawSceneNode.textures[normalIdx] = normal;
        }
      }

      nodes.push_back(rawSceneNode);
    }
  }

  for (int childIndex : currentNode.children) {
    processSceneNode(model, childIndex, currentTransform, nodes, texturesMap,
                     scenePath);
  }
}

void AssetLoader::loadKtxTexture(const std::string &path,
                                 Core::RawTexture &rawTexture) {}
void AssetLoader::loadImageTexture(const std::string &path,
                                   Core::RawTexture &rawTexture) {
  int w, h, channels;
  unsigned char *pixels =
      stbi_load(path.c_str(), &w, &h, &channels, STBI_rgb_alpha);

  if (!pixels) {
    throw std::runtime_error(
        "AssetLoader::LoadImageTexture::ERROR: Failed to load texture at " +
        path);
  }

  size_t imageSize = w * h * 4;
  rawTexture.pixels = std::vector<unsigned char>(pixels, pixels + imageSize);
  rawTexture.width = w;
  rawTexture.height = h;
  rawTexture.componentCount = 4;
  stbi_image_free(pixels);
}

void AssetLoader::loadGltfSceneFromGltf(
    const std::string &path, const std::string &name,
    std::vector<Core::RawSceneNode> &nodes) {
  std::string pathToFile =
      (std::filesystem::path(path) / std::filesystem::path(name)).string();
  tinygltf::Model model = loadTinyGltfModelFromASCII(pathToFile + ".gltf");
  tinygltf::Scene scene = model.scenes[model.defaultScene];

  std::unordered_map<std::string, int> texturesMap;
  for (int nodeIndex : scene.nodes) {
    processSceneNode(model, nodeIndex, glm::mat4(1.0f), nodes, texturesMap,
                     path);
  }
}
} // namespace Helper
} // namespace SimpleEngine
