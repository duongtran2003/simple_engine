#include "helpers/asset_loader.hpp"
#include "core/raw_scene_node.hpp"
#include "core/raw_texture.hpp"
#include "core/resource/mesh.hpp"
#include "core/resource2/raw_model_data.hpp"
#include "enums/image_enums.hpp"
#include "enums/texture.hpp"
#include "helpers/math.hpp"
#include "vulkan/vulkan.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fastgltf/math.hpp>
#include <fastgltf/util.hpp>
#include <filesystem>
#include <glm/ext/matrix_float3x3.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/matrix.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <tiny_gltf.h>
#include <vector>

namespace SimpleEngine {

// Converter
Enums::Image::Filter GetFilterEnum(fastgltf::Filter filter) {
  switch (filter) {
  case fastgltf::Filter::Nearest:
  case fastgltf::Filter::NearestMipMapNearest:
  case fastgltf::Filter::NearestMipMapLinear:
    return Enums::Image::Filter::eNearest;
  case fastgltf::Filter::Linear:
  case fastgltf::Filter::LinearMipMapNearest:
  case fastgltf::Filter::LinearMipMapLinear:
  default:
    return Enums::Image::Filter::eLinear;
  }
}

Enums::Image::MipmapMode GetMipmapModeEnum(fastgltf::Filter filter) {
  switch (filter) {
  case fastgltf::Filter::NearestMipMapNearest:
  case fastgltf::Filter::LinearMipMapNearest:
    return Enums::Image::MipmapMode::eNearest;
  case fastgltf::Filter::NearestMipMapLinear:
  case fastgltf::Filter::LinearMipMapLinear:
  default:
    return Enums::Image::MipmapMode::eLinear;
  }
}

Enums::Image::Wrap GetWrapEnum(fastgltf::Wrap wrap) {
  switch (wrap) {
  case fastgltf::Wrap::ClampToEdge:
    return Enums::Image::Wrap::eClampToEdge;
  case fastgltf::Wrap::MirroredRepeat:
    return Enums::Image::Wrap::eMirroredRepeat;
  case fastgltf::Wrap::Repeat:
  default:
    return Enums::Image::Wrap::eRepeat;
  }
}

namespace Helper {

void AssetLoader::ParseGltf(const std::filesystem::path &filePath,
                            fastgltf::Asset &asset) {
  if (!std::filesystem::exists(filePath)) {
    throw std::runtime_error(
        "Helper::AssetLoader::LoadGltf::ERROR: File not found at " +
        filePath.string());
  }

  auto data = fastgltf::GltfDataBuffer::FromPath(filePath);
  if (!data) {
    throw std::runtime_error(
        "Helper::AssetLoader::LoadGltf::ERROR: Failed to load data from " +
        filePath.string());
  }

  auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember |
                     fastgltf::Options::AllowDouble |
                     fastgltf::Options::LoadExternalBuffers |
                     fastgltf::Options::LoadExternalImages;

  fastgltf::Parser gltfParser;
  auto parseResult =
      gltfParser.loadGltf(data.get(), filePath.parent_path(), gltfOptions);
  if (parseResult.error() != fastgltf::Error::None) {
    throw std::runtime_error(
        "Helper::AssetLoader::LoadGltf::ERROR: Parsing error at" +
        filePath.string());
  }

  asset = std::move(parseResult.get());
}

void AssetLoader::ExtractGltfSamplers(
    fastgltf::Asset &asset, std::vector<Core::RawSamplerData> &samplers) {
  samplers.reserve(asset.samplers.size());
  for (const auto &sampler : asset.samplers) {
    Core::RawSamplerData rawSampler{};
    if (sampler.magFilter.has_value()) {
      rawSampler.MagFilter = GetFilterEnum(sampler.magFilter.value());
    }
    if (sampler.minFilter.has_value()) {
      rawSampler.MinFilter = GetFilterEnum(sampler.minFilter.value());
      rawSampler.MipmapMode = GetMipmapModeEnum(sampler.minFilter.value());
    }
    rawSampler.WrapU = GetWrapEnum(sampler.wrapS);
    rawSampler.WrapV = GetWrapEnum(sampler.wrapT);
    samplers.push_back(rawSampler);
  }
}

void AssetLoader::ExtractGltfImages(fastgltf::Asset &asset,
                                    std::vector<Core::RawImageData> &images,
                                    const std::filesystem::path &filePath) {
  images.reserve(asset.images.size());
  for (const auto &image : asset.images) {
    Core::RawImageData rawImage{};
    rawImage.Name = image.name;

    // NOTE: I have no idea what this does but fastgltf seems to use it
    fastgltf::visitor visitor{
        [&](const fastgltf::sources::URI &uri) {
          rawImage.FilePath =
              (filePath.parent_path() / uri.uri.fspath()).string();
        },
        [&](const fastgltf::sources::Array &array) {
          rawImage.Bytes.assign(
              reinterpret_cast<const uint8_t *>(array.bytes.data()),
              reinterpret_cast<const uint8_t *>(array.bytes.data()) +
                  array.bytes.size());
        },
        [&](const fastgltf::sources::BufferView &bufferView) {
          const auto &view = asset.bufferViews[bufferView.bufferViewIndex];
          const auto &buffer = asset.buffers[view.bufferIndex];
          fastgltf::visitor visitor{
              [&](const fastgltf::sources::Array &array) {
                const uint8_t *start =
                    reinterpret_cast<const uint8_t *>(array.bytes.data()) +
                    view.byteOffset;
                rawImage.Bytes.assign(start, start + view.byteLength);
              },
              [&](const fastgltf::sources::ByteView &viewData) {
                const uint8_t *start =
                    reinterpret_cast<const uint8_t *>(viewData.bytes.data()) +
                    view.byteOffset;
                rawImage.Bytes.assign(start, start + view.byteLength);
              },
              [&](const auto &) {
                std::cout << "Helper::AssetLoader::ExtractGltfImages::ERROR: "
                             "Unsupported bufferView source backing.\n";
              }};
          std::visit(visitor, buffer.data);
        },
        [&](const auto &) {
          std::cout << "Helper::AssetLoader::ExtractGltfImages::ERROR: "
                       "Unsupported image source type.\n";
        }};
    std::visit(visitor, image.data);
    images.push_back(std::move(rawImage));
  }
}

void AssetLoader::ExtractGltfTextures(
    fastgltf::Asset &asset, std::vector<Core::RawTextureData> &textures) {
  textures.reserve(asset.textures.size());
  for (const auto &texture : asset.textures) {
    Core::RawTextureData rawTexture{};
    rawTexture.SamplerIndex =
        static_cast<int32_t>(texture.samplerIndex.value_or(-1));
    rawTexture.ImageIndex =
        static_cast<int32_t>(texture.imageIndex.value_or(-1));
    textures.push_back(rawTexture);
  }
}

void AssetLoader::ExtractGltfMaterials(
    fastgltf::Asset &asset, std::vector<Core::RawMaterialData> &materials) {
  materials.reserve(asset.materials.size());
  for (const auto &material : asset.materials) {
    Core::RawMaterialData rawMaterial{};
    rawMaterial.Name = material.name;

    const auto &pbr = material.pbrData;
    if (pbr.baseColorTexture.has_value()) {
      rawMaterial.BaseColorTextureIndex =
          static_cast<int32_t>(pbr.baseColorTexture->textureIndex);
    }
    rawMaterial.BaseColorFactor =
        glm::vec4(pbr.baseColorFactor[0], pbr.baseColorFactor[1],
                  pbr.baseColorFactor[2], pbr.baseColorFactor[3]);

    if (pbr.metallicRoughnessTexture.has_value()) {
      rawMaterial.MetallicRoughnessTextureIndex =
          static_cast<int32_t>(pbr.metallicRoughnessTexture->textureIndex);
    }
    rawMaterial.MetallicFactor = pbr.metallicFactor;
    rawMaterial.RoughnessFactor = pbr.roughnessFactor;

    if (material.normalTexture.has_value()) {
      rawMaterial.NormalTextureIndex =
          static_cast<int32_t>(material.normalTexture->textureIndex);
    }
    if (material.occlusionTexture.has_value()) {
      rawMaterial.OcclusionTextureIndex =
          static_cast<int32_t>(material.occlusionTexture->textureIndex);
    }
    if (material.emissiveTexture.has_value()) {
      rawMaterial.EmissiveTextureIndex =
          static_cast<int32_t>(material.emissiveTexture->textureIndex);
    }
    rawMaterial.EmissiveFactor =
        glm::vec3(material.emissiveFactor[0], material.emissiveFactor[1],
                  material.emissiveFactor[2]);

    rawMaterial.AlphaCutoff = material.alphaCutoff;
    rawMaterial.DoubleSided = material.doubleSided;

    materials.push_back(rawMaterial);
  }
}

void AssetLoader::ExtractMeshes(
    fastgltf::Asset &asset,
    std::vector<std::vector<uint32_t>> &meshToPrimitives,
    Core::RawModelData &modelData, const std::filesystem::path &filePath) {
  meshToPrimitives.resize(asset.meshes.size());
  for (size_t meshIdx = 0; meshIdx < asset.meshes.size(); meshIdx++) {
    const auto &mesh = asset.meshes[meshIdx];
    std::string meshName = mesh.name.empty()
                               ? ("mesh_" + std::to_string(meshIdx))
                               : std::string(mesh.name);
    for (size_t primitiveIdx = 0; primitiveIdx < mesh.primitives.size();
         primitiveIdx++) {
      const auto &primitive = mesh.primitives[primitiveIdx];
      Core::RawPrimitiveData rawPrimitive{};
      rawPrimitive.Identifier = filePath.string() + "#" + meshName + "_" +
                                std::to_string(primitiveIdx);
      rawPrimitive.MaterialIndex =
          primitive.materialIndex.has_value()
              ? static_cast<int32_t>(primitive.materialIndex.value())
              : -1;

      auto *positionIt = primitive.findAttribute("POSITION");
      if (positionIt == primitive.attributes.end()) {
        continue;
      }
      const auto &accessor = asset.accessors[positionIt->accessorIndex];
      rawPrimitive.Vertices.resize(accessor.count);
      fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(
          asset, accessor, [&](fastgltf::math::fvec3 position, size_t idx) {
            rawPrimitive.Vertices[idx].Position =
                glm::vec3(position[0], position[1], position[2]);
          });

      auto *normalIt = primitive.findAttribute("NORMAL");
      if (normalIt != primitive.attributes.end()) {
        const auto &normalAccessor = asset.accessors[normalIt->accessorIndex];
        fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(
            asset, normalAccessor,
            [&](fastgltf::math::fvec3 normal, size_t idx) {
              rawPrimitive.Vertices[idx].Normal =
                  glm::vec3(normal[0], normal[1], normal[2]);
            });
      }

      auto *texCoordIt = primitive.findAttribute("TEXCOORD_0");
      if (texCoordIt != primitive.attributes.end()) {
        const auto &uvAccessor = asset.accessors[texCoordIt->accessorIndex];
        fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(
            asset, uvAccessor, [&](fastgltf::math::fvec2 uv, size_t idx) {
              rawPrimitive.Vertices[idx].TexCoords = glm::vec2(uv[0], uv[1]);
            });
      }

      if (primitive.indicesAccessor.has_value()) {
        const auto &indexAccessor = asset.accessors[*primitive.indicesAccessor];
        rawPrimitive.Indices.resize(indexAccessor.count);

        fastgltf::iterateAccessorWithIndex<uint32_t>(
            asset, indexAccessor, [&](uint32_t index, size_t idx) {
              rawPrimitive.Indices[idx] = index;
            });
        rawPrimitive.IndexType = vk::IndexType::eUint32;
      }

      uint32_t globalIndex = static_cast<uint32_t>(modelData.Primitives.size());
      modelData.Primitives.push_back(std::move(rawPrimitive));
      meshToPrimitives[meshIdx].push_back(globalIndex);
    }
  }
}

void AssetLoader::BuildNodesHierarchy(
    fastgltf::Asset &asset, std::vector<Core::RawNode> &nodes,
    std::vector<std::vector<uint32_t>> &meshToPrimitives) {
  nodes.reserve(asset.nodes.size());
  for (size_t nodeIdx = 0; nodeIdx < asset.nodes.size(); nodeIdx++) {
    const auto &node = asset.nodes[nodeIdx];
    Core::RawNode rawNode{};
    rawNode.Name = node.name;
    fastgltf::visitor visitor{
        [&](const fastgltf::TRS &trs) {
          rawNode.Translation = glm::vec3{
              trs.translation[0], trs.translation[1], trs.translation[2]};
          rawNode.Rotation = glm::quat{trs.rotation[3], trs.rotation[0],
                                       trs.rotation[1], trs.rotation[2]};
          rawNode.Scale = glm::vec3{trs.scale[0], trs.scale[1], trs.scale[2]};

          glm::mat4 T = glm::translate(glm::mat4(1.0f), rawNode.Translation);
          glm::mat4 R = glm::mat4_cast(rawNode.Rotation);
          glm::mat4 S = glm::scale(glm::mat4(1.0f), rawNode.Scale);
          rawNode.LocalMatrix = T * R * S;
        },
        [&](const fastgltf::math::fmat4x4 &mat) {
          rawNode.LocalMatrix = glm::make_mat4(mat.data());

          rawNode.Translation = glm::vec3(rawNode.LocalMatrix[3]);

          glm::vec3 scale;
          scale.x = glm::length(glm::vec3(rawNode.LocalMatrix[0]));
          scale.y = glm::length(glm::vec3(rawNode.LocalMatrix[1]));
          scale.z = glm::length(glm::vec3(rawNode.LocalMatrix[2]));
          rawNode.Scale = scale;

          glm::mat3 rotMtx(glm::vec3(rawNode.LocalMatrix[0]) / scale.x,
                           glm::vec3(rawNode.LocalMatrix[1]) / scale.y,
                           glm::vec3(rawNode.LocalMatrix[2]) / scale.z);
          rawNode.Rotation = glm::quat_cast(rotMtx);
        }};
    std::visit(visitor, node.transform);

    if (node.meshIndex.has_value()) {
      rawNode.PrimitiveIndices = meshToPrimitives[node.meshIndex.value()];
    }

    for (size_t childIdx : node.children) {
      rawNode.ChildrenIndices.push_back(static_cast<unsigned int>(childIdx));
    }

    nodes.push_back(rawNode);
  }
}

std::optional<Core::RawModelData>
AssetLoader::LoadGltf(const std::filesystem::path &filePath) {
  fastgltf::Asset gltf;
  ParseGltf(filePath, gltf);
  Core::RawModelData modelData{};
  ExtractGltfSamplers(gltf, modelData.Samplers);
  ExtractGltfImages(gltf, modelData.Images, filePath);
  ExtractGltfTextures(gltf, modelData.Textures);
  ExtractGltfMaterials(gltf, modelData.Materials);
  std::vector<std::vector<uint32_t>> meshToPrimitives;
  ExtractMeshes(gltf, meshToPrimitives, modelData, filePath);
  BuildNodesHierarchy(gltf, modelData.Nodes, meshToPrimitives);

  for (size_t i = 0; i < modelData.Nodes.size(); i++) {
    for (uint32_t childIdx : modelData.Nodes[i].ChildrenIndices) {
      if (childIdx < modelData.Nodes.size()) {
        modelData.Nodes[childIdx].ParentIndex = static_cast<int32_t>(i);
      }
    }
  }

  const auto &scene = gltf.scenes[gltf.defaultScene.value_or(0)];
  for (size_t rootIdx : scene.nodeIndices) {
    modelData.RootNodeIndices.push_back(static_cast<uint32_t>(rootIdx));
  }

  return modelData;
}

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
  if (loadMode == TextureLoadMode::FromBinary) {
    rawTexture.pixels = image.image;
    rawTexture.width = static_cast<uint32_t>(image.width);
    rawTexture.height = static_cast<uint32_t>(image.height);
    rawTexture.componentCount = static_cast<uint32_t>(image.component);
  } else if (loadMode == TextureLoadMode::FromUri) {
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
    std::unordered_map<std::string, Core::RawTexture> &texturesMap,
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
          size_t i1 = rawSceneNode.indices[i];
          size_t i2 = rawSceneNode.indices[i + 1];
          size_t i3 = rawSceneNode.indices[i + 2];

          Helper::Math::calculateTangent(rawSceneNode.vertices[i1],
                                         rawSceneNode.vertices[i2],
                                         rawSceneNode.vertices[i3]);
        }

        for (size_t i = 0; i < rawSceneNode.vertices.size(); i++) {
          auto &v = rawSceneNode.vertices[i];
          glm::vec3 N = glm::normalize(v.normal);
          glm::vec3 T = glm::normalize(glm::vec3(v.tangent));

          if (glm::length(T) > 0.0f) {
            glm::vec3 B = glm::normalize(glm::cross(T, N));
            B = glm::normalize(B);

            float w = 1.0f;
            glm::mat3 TBN = glm::mat3(T, B, N);
            if (glm::determinant(TBN) < 0.0f) {
              w = -1.0f;
            }
            v.tangent = glm::vec4(T, w);
          } else {
            v.tangent = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
          }
        }
      }

      if (primitive.material >= 0) {
        rawSceneNode.textureNames.resize(Core::RawSceneNode::TEX_INDEXER_COUNT);
        const auto &material = model.materials[primitive.material];

        int albedoIndex = material.pbrMetallicRoughness.baseColorTexture.index;

        const std::vector<double> &colorFactor =
            material.pbrMetallicRoughness.baseColorFactor;
        if (colorFactor.size()) {
          rawSceneNode.pbrProperty.baseColorFactor = {
              colorFactor[0], colorFactor[1], colorFactor[2], colorFactor[3]};
        }

        const float metallicFactor =
            static_cast<float>(material.pbrMetallicRoughness.metallicFactor);
        rawSceneNode.pbrProperty.metallicFactor = metallicFactor;

        const float roughnessFactor =
            static_cast<float>(material.pbrMetallicRoughness.roughnessFactor);
        rawSceneNode.pbrProperty.roughnessFactor = roughnessFactor;

        int normalIndex = material.normalTexture.index;
        int metallicRoughnessIndex =
            material.pbrMetallicRoughness.metallicRoughnessTexture.index;

        if (albedoIndex >= 0) {
          const std::string name = material.name + "_" +
                                   std::to_string(primitive.material) +
                                   "_abledo";
          auto it = texturesMap.find(name);
          if (it == texturesMap.end()) {
            Core::RawTexture albedo{.name = name,
                                    .useMipmap = true,
                                    .colorSpace =
                                        Enums::Texture::ColorSpace::NonLinear};
            loadTextureFromTinyGltfModel(model, albedoIndex, albedo,
                                         TextureLoadMode::FromUri, scenePath);
            texturesMap[albedo.name] = albedo;
          }

          size_t albedoIdx =
              static_cast<size_t>(Core::RawSceneNode::TextureIndexer::Albedo);
          rawSceneNode.textureNames[albedoIdx] = name;
        }

        if (normalIndex >= 0) {
          const std::string name = material.name + "_" +
                                   std::to_string(primitive.material) +
                                   "_normal";
          auto it = texturesMap.find(name);
          if (it == texturesMap.end()) {
            Core::RawTexture normal{.name = name,
                                    .useMipmap = true,
                                    .colorSpace =
                                        Enums::Texture::ColorSpace::Linear};
            loadTextureFromTinyGltfModel(model, normalIndex, normal,
                                         TextureLoadMode::FromUri, scenePath);
            texturesMap[normal.name] = normal;
          }

          size_t normalIdx =
              static_cast<size_t>(Core::RawSceneNode::TextureIndexer::Normal);
          rawSceneNode.textureNames[normalIdx] = name;
        }

        if (metallicRoughnessIndex >= 0) {
          const std::string name = material.name + "_" +
                                   std::to_string(primitive.material) +
                                   "_metallicRoughness";

          auto it = texturesMap.find(name);
          if (it == texturesMap.end()) {
            Core::RawTexture metallicRoughness{
                .name = name,
                .useMipmap = true,
                .colorSpace = Enums::Texture::ColorSpace::Linear};
            loadTextureFromTinyGltfModel(model, metallicRoughnessIndex,
                                         metallicRoughness,
                                         TextureLoadMode::FromUri, scenePath);
            texturesMap[metallicRoughness.name] = metallicRoughness;
          }

          size_t metallicRoughnessIdx = static_cast<size_t>(
              Core::RawSceneNode::TextureIndexer::MetallicRoughness);
          rawSceneNode.textureNames[metallicRoughnessIdx] = name;
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

void AssetLoader::LoadImage(const std::string &path,
                            std::vector<unsigned char> &pixels, int &width,
                            int &height, int &channels) {
  unsigned char *pRaw =
      stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
  if (!pRaw) {
    stbi_image_free(pRaw);
    throw std::runtime_error(
        "AssetLoader::LoadImage::ERROR: Failed to load image at " + path);
  }

  size_t size = static_cast<size_t>(width * height * channels);
  pixels = std::vector<unsigned char>(pRaw, pRaw + size);
  stbi_image_free(pRaw);
}

void AssetLoader::loadGltfSceneFromGltf(
    const std::string &path, const std::string &name,
    std::vector<Core::RawSceneNode> &nodes,
    std::unordered_map<std::string, Core::RawTexture> &texturesMap) {
  std::string pathToFile =
      (std::filesystem::path(path) / std::filesystem::path(name)).string();
  tinygltf::Model model = loadTinyGltfModelFromASCII(pathToFile + ".gltf");
  tinygltf::Scene scene = model.scenes[model.defaultScene];

  for (int nodeIndex : scene.nodes) {
    processSceneNode(model, nodeIndex, glm::mat4(1.0f), nodes, texturesMap,
                     path);
  }
}
} // namespace Helper
} // namespace SimpleEngine
