#include "core/image/image_pool.hpp"
#include "core/image/image.hpp"
#include "core/image/image_handle.hpp"
#include "core/render_context.hpp"
#include <cstdint>
#include <memory>

namespace SimpleEngine {
namespace Core {
ImagePool::ImagePool(const RenderContext *context) {
  m_pContext = context;
  m_Slots.resize(MAX_IMAGES_POOL_SIZE);
  m_FreeList.reserve(MAX_IMAGES_POOL_SIZE);
  for (int64_t i = MAX_IMAGES_POOL_SIZE - 1; i >= 1; --i) {
    m_FreeList.push_back(static_cast<uint32_t>(i));
  }
}

ImagePool::~ImagePool() {
  for (Slot &slot : m_Slots) {
    if (slot.Image) {
      slot.Image->Free();
      slot.Image.reset();
    }
  }
}

Image *ImagePool::Get(ImageHandle handle) {
  if (handle.Id >= m_Slots.size()) {
    return nullptr;
  }

  const Slot &slot = m_Slots[handle.Id];
  if (slot.Generation != handle.Generation) {
    return nullptr;
  }

  return slot.Image.get();
}

ImageHandle ImagePool::Allocate(const Image::CreateInfo &imageCreateInfo) {
  if (m_FreeList.size() == 0) {
    return ImageHandle{0, 0};
  }

  uint32_t freeSlotIdx = m_FreeList.back();
  m_FreeList.pop_back();

  Slot &freeSlot = m_Slots[freeSlotIdx];
  freeSlot.Image = std::make_unique<Image>(imageCreateInfo, m_pContext);

  ImageHandle handle{.Id = freeSlotIdx, .Generation = freeSlot.Generation};
  return handle;
}

void ImagePool::Free(ImageHandle handle) {
  if (handle.Id >= m_Slots.size()) {
    return;
  }

  Slot &slot = m_Slots[handle.Id];
  if (slot.Generation == handle.Generation && slot.Image != nullptr) {
    slot.Image->Free();
    slot.Image.reset();
    slot.Generation++;
    m_FreeList.push_back(handle.Id);
  }
}
} // namespace Core
} // namespace SimpleEngine
