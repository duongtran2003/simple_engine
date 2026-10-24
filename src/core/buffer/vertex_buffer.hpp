#pragma once

#include "core/buffer/buffer.hpp"
#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <vector>

namespace SimpleEngine {
namespace Core {

struct Vertex;

class VertexBuffer {
private:
  const RenderContext *m_pContext;

  Buffer m_Buffer;
  uint32_t m_VertexCount = 0;

public:
  VertexBuffer(const std::vector<Vertex> &vertices,
               const RenderContext *pRenderContext);
  ~VertexBuffer();

  VertexBuffer() = default;
  VertexBuffer(const VertexBuffer &) = delete;
  VertexBuffer &operator=(const VertexBuffer &) = delete;
  VertexBuffer(VertexBuffer &&) = default;
  VertexBuffer &operator=(VertexBuffer &&) = default;

  vk::Buffer &GetBuffer();
  uint32_t GetVertexCount() const;
};
} // namespace Core
} // namespace SimpleEngine
