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
  bool hasMetallicRoughnessTexture = false;

  PbrProperty pbrProperty;

public:
  Material();

  Material *setAlbedo(TextureBinding binding);
  const TextureBinding &getAlbedo() const;
  Material *registerAlbedo(vk::DescriptorSet &set, uint32_t index,
                           const RenderContext &context);

  Material *setNormal(TextureBinding binding);
  const TextureBinding &getNormal() const;
  Material *registerNormal(vk::DescriptorSet &set, uint32_t index,
                           const RenderContext &context);

  Material *setMetallicRoughness(TextureBinding binding);
  const TextureBinding &getMetallicRoughness() const;
  Material *registerMetallicRoughness(vk::DescriptorSet &set, uint32_t index,
                                      const RenderContext &context);

  Material *setPbrBaseColorFactor(glm::vec4 factor);
  const glm::vec4 getPbrBaseColorFactor() const;

  Material *setPbrMetallicFactor(float factor);
  float getPbrMetallicFactor() const;

  Material *setPbrRoughnessFactor(float factor);
  float getPbrRoughnessFactor() const;

  bool hasAlbedo() const;

  bool hasNormal() const;

  bool hasMetallicRoughness() const;
};
} // namespace Core
} // namespace SimpleEngine
