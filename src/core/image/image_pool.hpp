#pragma once

#include "core/image/image.hpp"
#include "core/image/image_handle.hpp"
#include "core/render_context.hpp"
#include <cstdint>
#include <memory>
#include <vector>

constexpr uint32_t MAX_IMAGES_POOL_SIZE = 65536;

namespace SimpleEngine {
namespace Core {
class ImagePool {
public:
  ImagePool() = delete;
  ImagePool(const RenderContext *context);
  ~ImagePool();

  Image *Get(ImageHandle handle);
  ImageHandle Allocate(const Image::CreateInfo &imageCreateInfo);
  void Free(ImageHandle handle);

private:
  struct Slot {
    std::unique_ptr<Image> Image;
    uint32_t Generation = 1;
  };

  const RenderContext *m_pContext;

  std::vector<Slot> m_Slots;
  std::vector<uint32_t> m_FreeList;
};
} // namespace Core
} // namespace SimpleEngine
