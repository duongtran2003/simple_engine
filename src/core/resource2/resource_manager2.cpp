#include "core/resource2/resource_manager2.hpp"
#include "core/image/image_pool.hpp"
#include "core/image/sampler_registry.hpp"
#include "core/render_context.hpp"
#include "core/resource2/texture2.hpp"
#include "core/resource2/texture_handle.hpp"
#include <cstdint>
#include <string>

namespace SimpleEngine {
namespace Core {
ResourceManager2::ResourceManager2(ImagePool *imagePool,
                                   SamplerRegistry *samplerRegistry,
                                   const RenderContext *renderContext) {
  m_pImagePool = imagePool;
  m_pSamplerRegistry = samplerRegistry;
  m_pContext = renderContext;

  InitTextureSlots();
}

void ResourceManager2::InitTextureSlots() {
  m_TextureSlots.resize(MAX_TEXTURES_POOL_SIZE);
  m_TextureFreeList.reserve(MAX_TEXTURES_POOL_SIZE);
  for (int64_t i = MAX_TEXTURES_POOL_SIZE - 1; i >= 1; --i) {
    m_TextureFreeList.push_back(static_cast<uint32_t>(i));
  }
}

ResourceManager2::~ResourceManager2() {
  // TODO: Destructor
}

Texture2 *ResourceManager2::Get(TextureHandle handle) {
  if (handle.Id >= m_TextureSlots.size()) {
    return nullptr;
  }

  const TextureSlot &slot = m_TextureSlots[handle.Id];
  if (slot.Generation != handle.Generation) {
    return nullptr;
  }

  return slot.Texture.get();
}

TextureHandle ResourceManager2::AllocateTexture(const std::string &path) {
  auto it = m_TextureCache.find(path);
  if (it != m_TextureCache.end()) {
    return it->second;
  }

  if (m_TextureFreeList.size() == 0) {
    return TextureHandle{0, 0};
  }

  uint32_t freeSlotIdx = m_TextureFreeList.back();
  m_TextureFreeList.pop_back();

  TextureSlot &freeSlot = m_TextureSlots[freeSlotIdx];
  // freeSlot.Texture = std::make_unique<Texture2>({}, m_pContext);

  TextureHandle handle{.Id = freeSlotIdx, .Generation = freeSlot.Generation};
  return handle;
}
} // namespace Core
} // namespace SimpleEngine
