#pragma once

#include "core/resource2/mesh2.hpp"
#include "enums/image_enums.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Core {
struct RawSamplerData {
  Enums::Image::Filter MagFilter = Enums::Image::Filter::eLinear;
  Enums::Image::Filter MinFilter = Enums::Image::Filter::eLinear;

  Enums::Image::MipmapMode MipmapMode = Enums::Image::MipmapMode::eLinear;

  Enums::Image::Wrap WrapU = Enums::Image::Wrap::eRepeat;
  Enums::Image::Wrap WrapV = Enums::Image::Wrap::eRepeat;
  Enums::Image::Wrap WrapW = Enums::Image::Wrap::eRepeat;
};

struct RawImageData {
  std::string Name;
  std::string FilePath;
  std::vector<uint8_t> Bytes;
};

struct RawTextureData {
  int32_t ImageIndex = -1;
  int32_t SamplerIndex = -1;
};

struct RawMaterialData {
  std::string Name;

  int32_t BaseColorTextureIndex = -1;
  int32_t MetallicRoughnessTextureIndex = -1;
  int32_t NormalTextureIndex = -1;
  int32_t OcclusionTextureIndex = -1;
  int32_t EmissiveTextureIndex = -1;

  glm::vec4 BaseColorFactor{1.0f};
  float MetallicFactor = 1.0f;
  float RoughnessFactor = 1.0f;
  glm::vec3 EmissiveFactor{0.0f};

  float AlphaCutoff = 0.5f;
  bool DoubleSided = false;
};

struct RawPrimitiveData {
  std::string Identifier;
  std::vector<Vertex> Vertices;
  std::vector<uint32_t> Indices;
  vk::IndexType IndexType = vk::IndexType::eUint32;
  int32_t MaterialIndex = -1;
};

struct RawNode {
  std::string Name;

  glm::vec3 Translation{0.0f};
  glm::quat Rotation{1.0f, 0.0f, 0.0f, 0.0f};
  glm::vec3 Scale{1.0f};
  glm::mat4 LocalMatrix{1.0f};

  std::vector<uint32_t> PrimitiveIndices;
  int32_t ParentIndex = -1;
  std::vector<uint32_t> ChildrenIndices;
};

struct RawModelData {
  std::vector<RawSamplerData> Samplers;
  std::vector<RawImageData> Images;
  std::vector<RawTextureData> Textures;
  std::vector<RawMaterialData> Materials;
  std::vector<RawPrimitiveData> Primitives;

  std::vector<RawNode> Nodes;
  std::vector<uint32_t> RootNodeIndices;
};
} // namespace Core
} // namespace SimpleEngine
