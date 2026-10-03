#pragma once

#include "core/image/image_pool.hpp"
#include "core/image/sampler_registry.hpp"
#include "core/render_context.hpp"
#include "core/resource2/texture2.hpp"
#include "core/resource2/texture_handle.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

constexpr uint32_t MAX_TEXTURES_POOL_SIZE = 65536;

namespace SimpleEngine {
namespace Core {
class ResourceManager2 {
public:
  ResourceManager2() = delete;
  ResourceManager2(ImagePool *imagePool, SamplerRegistry *samplerRegistry,
                   const RenderContext *renderContext);
  ~ResourceManager2();

  Texture2 *Get(TextureHandle handle);
  TextureHandle AllocateTexture(const std::string& path);
  void ReleaseTexture(TextureHandle handle);

private:
  struct TextureSlot {
    std::unique_ptr<Texture2> Texture;
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
};
} // namespace Core
} // namespace SimpleEngine
