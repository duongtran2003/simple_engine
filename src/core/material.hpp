#pragma once

#include "core/render_context.hpp"
#include "core/resource/resource_handle.hpp"
#include "core/resource/texture.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <glm/ext/vector_float4.hpp>

namespace SimpleEngine {
namespace Core {
class Material {
public:
  struct PbrProperty {
    glm::vec4 baseColorFactor = glm::vec4(1.0f);
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
  };

  struct TextureBinding {
    uint32_t index;
    ResourceHandle<Texture> handle;
  };

private:
  TextureBinding albedo;
  TextureBinding normal;
  TextureBinding metallicRoughness;

  bool hasAlbedoTexture = false;
  bool hasNormalTexture = false;
  bool hasMetallicRoughness = false;

  PbrProperty pbrProperty;

public:
  Material();

  Material *setAlbedo(TextureBinding binding);
  const TextureBinding &getAlbedo() const;
  Material *registerAlbedo(vk::DescriptorSet &set, uint32_t index,
                           const RenderContext &context);
  Material *setPbrBaseColorFactor(glm::vec4 factor);
  const glm::vec4 getPbrBaseColorFactor() const;

  Material *setNormal(TextureBinding binding);
  const TextureBinding &getNormal() const;
  Material *registerNormal(vk::DescriptorSet &set, uint32_t index,
                           const RenderContext &context);

  bool hasAlbedo() const { return hasAlbedoTexture; }

  bool hasNormal() const { return hasNormalTexture; }
};
} // namespace Core
} // namespace SimpleEngine
