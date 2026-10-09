#pragma once

#include "core/image/image_pool.hpp"
#include "core/image/sampler_registry.hpp"
#include "core/render_context.hpp"
#include "core/resource2/texture2.hpp"
#include "core/resource2/texture_handle.hpp"
#include "enums/image_enums.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

constexpr uint32_t MAX_TEXTURES_POOL_SIZE = 65536;

namespace SimpleEngine {
namespace Core {
class ResourceManager2 {
public:
  struct TextureAllocateInfo {
    Enums::Image::ColorSpace ColorSpace;
    vk::ImageAspectFlags AspectMask;
    uint32_t MipLevels;
    uint32_t ArrayLayers;
  };

  ResourceManager2() = delete;
  ResourceManager2(ImagePool *imagePool, SamplerRegistry *samplerRegistry,
                   const RenderContext *renderContext);
  ~ResourceManager2();

  const Texture2 *Get(TextureHandle handle);
  TextureHandle AllocateTexture(const std::string &path,
                                const TextureAllocateInfo &allocateInfo);
  // TODO: Implement deferred texture unloading
  void ReleaseTexture(TextureHandle handle);

  // Force free texture regardless RefCount
  void FreeTexture(TextureHandle handle);

private:
  struct TextureSlot {
    Texture2 Texture;
    uint32_t Generation = 1;
    uint32_t RefCount = 0;
  };

  std::vector<TextureSlot> m_TextureSlots;
  std::vector<uint32_t> m_TextureFreeList;
  std::unordered_map<std::string, TextureHandle> m_TextureCache;

  ImagePool *m_pImagePool = nullptr;
  SamplerRegistry *m_pSamplerRegistry = nullptr;
  const RenderContext *m_pContext = nullptr;

  void InitTextureSlots();
  void FreeTextureSlot(uint32_t slotIndex);
};
} // namespace Core
} // namespace SimpleEngine
