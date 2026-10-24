#include "core/buffer/index_buffer.hpp"
#include "core/buffer/buffer.hpp"
#include "core/render_context.hpp"
#include "helpers/vulkan_helper.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace SimpleEngine {
namespace Core {
IndexBuffer::IndexBuffer(const std::vector<uint32_t> &indices,
                         const RenderContext *pRenderContext) {
  m_pContext = pRenderContext;

  vk::DeviceSize bufferSize = indices.size() * sizeof(uint32_t);
  if (bufferSize == 0) {
    throw std::runtime_error("IndexBuffer::ERROR: Size must be non-zero");
  }

  Buffer::CreateInfo stagingBufferInfo{
      .Size = bufferSize,
      .Usage = vk::BufferUsageFlagBits::eTransferSrc,
      .Properties = vk::MemoryPropertyFlagBits::eHostVisible |
                    vk::MemoryPropertyFlagBits::eHostCoherent};
  Buffer stagingBuffer = Buffer(stagingBufferInfo, m_pContext);
  stagingBuffer.Write(indices.data());

  Buffer::CreateInfo indexBufferInfo{
      .Size = bufferSize,
      .Usage = vk::BufferUsageFlagBits::eIndexBuffer |
               vk::BufferUsageFlagBits::eTransferDst,
      .Properties = vk::MemoryPropertyFlagBits::eDeviceLocal};
  m_Buffer = Buffer(indexBufferInfo, m_pContext);

  Helper::VulkanHelper::CopyBuffer(
      stagingBuffer.GetBuffer(), m_Buffer.GetBuffer(), bufferSize, *m_pContext);

  m_IndexCount = static_cast<uint32_t>(indices.size());
}

IndexBuffer::~IndexBuffer() {}
vk::Buffer &IndexBuffer::GetBuffer() { return m_Buffer.GetBuffer(); }
uint32_t IndexBuffer::GetIndexCount() const { return m_IndexCount; }
vk::IndexType IndexBuffer::GetIndexType() const { return m_IndexType; }
} // namespace Core
} // namespace SimpleEngine
