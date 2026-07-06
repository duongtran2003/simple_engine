#pragma once

#include "core/render_context.hpp"
#include "core/resource/resource_handle.hpp"
#include "core/resource/texture.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <glm/ext/vector_float3.hpp>

namespace SimpleEngine {
namespace Core {
class Material {
public:
  struct TextureBinding {
    uint32_t index;
    ResourceHandle<Texture> handle;
  };

private:
  TextureBinding albedo;
  TextureBinding normal;

  bool hasAlbedoTexture = false;
  bool hasNormalTexture = false;

  glm::vec3 baseColorFactor;

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

  bool hasAlbedo() const { return hasAlbedoTexture; }

  bool hasNormal() const { return hasNormalTexture; }
};
} // namespace Core
} // namespace SimpleEngine
