#pragma once

#include "core/render_context.hpp"
#include "core/resource/resource_handle.hpp"
#include "core/resource/texture.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>

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

public:
  Material();

  Material *setAlbedo(TextureBinding binding);
  const TextureBinding &getAlbedo() const;
  Material *registerAlbedo(vk::DescriptorSet &set, uint32_t index,
                           const RenderContext &context);
};
} // namespace Core
} // namespace SimpleEngine
