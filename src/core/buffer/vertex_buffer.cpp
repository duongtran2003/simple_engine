#include "core/buffer/vertex_buffer.hpp"
#include "core/buffer/buffer.hpp"
#include "core/render_context.hpp"
#include "core/resource2/mesh2.hpp"
#include "helpers/vulkan_helper.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace SimpleEngine {
namespace Core {
VertexBuffer::VertexBuffer(const std::vector<Vertex> &vertices,
                           const RenderContext *pRenderContext) {
  m_pContext = pRenderContext;

  vk::DeviceSize bufferSize = vertices.size() * sizeof(Vertex);
  if (bufferSize == 0) {
    throw std::runtime_error("VertexBuffer::ERROR: Size must be non-zero");
  }

  Buffer::CreateInfo stagingBufferInfo{
      .Size = bufferSize,
      .Usage = vk::BufferUsageFlagBits::eTransferSrc,
      .Properties = vk::MemoryPropertyFlagBits::eHostVisible |
                    vk::MemoryPropertyFlagBits::eHostCoherent};
  Buffer stagingBuffer = Buffer(stagingBufferInfo, m_pContext);
  stagingBuffer.Write(vertices.data());

  Buffer::CreateInfo vertexBufferInfo{
      .Size = bufferSize,
      .Usage = vk::BufferUsageFlagBits::eVertexBuffer |
               vk::BufferUsageFlagBits::eTransferDst,
      .Properties = vk::MemoryPropertyFlagBits::eDeviceLocal};
  m_Buffer = Buffer(vertexBufferInfo, m_pContext);

  Helper::VulkanHelper::CopyBuffer(
      stagingBuffer.GetBuffer(), m_Buffer.GetBuffer(), bufferSize, *m_pContext);

  m_VertexCount = static_cast<uint32_t>(vertices.size());
}

VertexBuffer::~VertexBuffer() {}
vk::Buffer &VertexBuffer::GetBuffer() { return m_Buffer.GetBuffer(); }
uint32_t VertexBuffer::GetVertexCount() const { return m_VertexCount; }
} // namespace Core
} // namespace SimpleEngine
