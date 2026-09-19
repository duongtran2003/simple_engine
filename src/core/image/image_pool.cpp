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
  m_slots.resize(MAX_IMAGES_POOL_SIZE);
  m_freeList.reserve(MAX_IMAGES_POOL_SIZE);
  for (int64_t i = MAX_IMAGES_POOL_SIZE - 1; i >= 1; --i) {
    m_freeList.push_back(static_cast<uint32_t>(i));
  }
}

ImagePool::~ImagePool() {
  for (Slot &slot : m_slots) {
    if (slot.image) {
      slot.image->Free();
      slot.image.reset();
    }
  }
}

Image *ImagePool::Get(ImageHandle handle) {
  if (handle.id >= m_slots.size()) {
    return nullptr;
  }

  const Slot &slot = m_slots[handle.id];
  if (slot.generation != handle.generation) {
    return nullptr;
  }

  return slot.image.get();
}

ImageHandle ImagePool::Allocate(const Image::CreateInfo &imageCreateInfo) {
  if (m_freeList.size() == 0) {
    return ImageHandle{0, 0};
  }

  uint32_t freeSlotIdx = m_freeList.back();
  m_freeList.pop_back();

  Slot &freeSlot = m_slots[freeSlotIdx];
  freeSlot.image = std::make_unique<Image>(imageCreateInfo, m_pContext);

  ImageHandle handle{.id = freeSlotIdx, .generation = freeSlot.generation};
  return handle;
}

void ImagePool::Free(ImageHandle handle) {
  if (handle.id >= m_slots.size()) {
    return;
  }

  Slot &slot = m_slots[handle.id];
  if (slot.generation == handle.generation && slot.image != nullptr) {
    slot.image->Free();
    slot.image.reset();
    slot.generation++;
    m_freeList.push_back(handle.id);
  }
}
} // namespace Core
} // namespace SimpleEngine
