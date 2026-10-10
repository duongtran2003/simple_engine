#include "core/buffer/buffer.hpp"
#include "core/render_context.hpp"
#include "helpers/vulkan_helper.hpp"
#include "vulkan/vulkan.hpp"
#include <cstring>
#include <utility>

namespace SimpleEngine {
namespace Core {
Buffer::Buffer(const CreateInfo &createInfo,
               const RenderContext *renderContext) {
  m_Size = createInfo.Size;
  m_Usage = createInfo.Usage;
  m_Properties = createInfo.Properties;
  m_pContext = renderContext;

  Create();
}

Buffer::Buffer(Buffer &&other) { *this = std::move(other); }
Buffer &Buffer::operator=(Buffer &&other) {
  Free();
  m_pContext = other.m_pContext;
  m_Size = other.m_Size;
  m_Usage = other.m_Usage;
  m_Properties = other.m_Properties;
  m_Buffer = other.m_Buffer;
  m_Memory = other.m_Memory;
  m_Mapped = other.m_Mapped;

  other.m_Buffer = nullptr;
  other.m_Memory = nullptr;
  other.m_Mapped = nullptr;
  other.m_Size = 0;

  return *this;
}

Buffer::~Buffer() { Free(); }
void Buffer::Create() {
  vk::BufferCreateInfo bufferInfo{.size = m_Size,
                                  .usage = m_Usage,
                                  .sharingMode = vk::SharingMode::eExclusive};

  m_Buffer = m_pContext->device.createBuffer(bufferInfo);
  vk::MemoryRequirements memoryRequirements =
      m_pContext->device.getBufferMemoryRequirements(m_Buffer);

  vk::MemoryAllocateInfo allocateInfo{
      .allocationSize = memoryRequirements.size,
      .memoryTypeIndex = Helper::VulkanHelper::findMemoryType(
          memoryRequirements.memoryTypeBits, m_Properties,
          m_pContext->physicalDevice)};

  m_Memory = m_pContext->device.allocateMemory(allocateInfo);
  m_pContext->device.bindBufferMemory(m_Buffer, m_Memory, 0);
}

void Buffer::Free() {
  if (m_Buffer == nullptr) {
    return;
  }

  if (IsMapped()) {
    Unmap();
  }

  m_pContext->device.destroyBuffer(m_Buffer);
  m_pContext->device.freeMemory(m_Memory);
  m_Buffer = nullptr;
  m_Memory = nullptr;
  m_Size = 0;
}

bool Buffer::IsMapped() const { return m_Mapped != nullptr; }
vk::DeviceSize Buffer::GetSize() const { return m_Size; }
vk::Buffer Buffer::GetBuffer() const { return m_Buffer; }
vk::DeviceMemory Buffer::GetMemory() const { return m_Memory; }
void *Buffer::GetMapped() const { return m_Mapped; }

void *Buffer::Map() {
  if (IsMapped()) {
    return m_Mapped;
  }

  m_Mapped = m_pContext->device.mapMemory(m_Memory, 0, m_Size);
  return m_Mapped;
}

void Buffer::Unmap() {
  if (!IsMapped()) {
    return;
  }

  m_pContext->device.unmapMemory(m_Memory);
  m_Mapped = nullptr;
}

void Buffer::Write(const void *data) {
  if (IsMapped()) {
    memcpy(m_Mapped, data, m_Size);
    return;
  }

  Map();
  memcpy(m_Mapped, data, m_Size);
  Unmap();
}
} // namespace Core
} // namespace SimpleEngine
