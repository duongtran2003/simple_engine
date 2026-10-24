#pragma once

#include "core/buffer/buffer.hpp"
#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <vector>

namespace SimpleEngine {
namespace Core {
class IndexBuffer {
private:
  const RenderContext *m_pContext;

  Buffer m_Buffer;
  uint32_t m_IndexCount = 0;
  // NOTE: Hard-coding index type for now
  vk::IndexType m_IndexType = vk::IndexType::eUint32;

public:
  IndexBuffer(const std::vector<uint32_t> &indices,
              const RenderContext *pRenderContext);
  ~IndexBuffer();

  IndexBuffer() = default;
  IndexBuffer(const IndexBuffer &) = delete;
  IndexBuffer &operator=(const IndexBuffer &) = delete;
  IndexBuffer(IndexBuffer &&) = default;
  IndexBuffer &operator=(IndexBuffer &&) = default;

  vk::Buffer &GetBuffer();
  uint32_t GetIndexCount() const;
  vk::IndexType GetIndexType() const;
};
} // namespace Core
} // namespace SimpleEngine
