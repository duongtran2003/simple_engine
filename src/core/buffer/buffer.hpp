#pragma once

#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
namespace SimpleEngine {
namespace Core {
class Buffer {
public:
  struct CreateInfo {
    vk::DeviceSize Size;
    vk::BufferUsageFlags Usage;
    vk::MemoryPropertyFlags Properties;
  };

  Buffer() = delete;
  Buffer(const Buffer &other) = delete;
  Buffer &operator=(const Buffer &other) = delete;
  Buffer(Buffer &&other);
  Buffer &operator=(Buffer &&other);
  Buffer(const CreateInfo &createInfo, const RenderContext *renderContext);
  ~Buffer();

  void *Map();
  void Unmap();

  bool IsMapped() const;
  vk::DeviceSize GetSize() const;
  vk::Buffer GetBuffer() const;
  vk::DeviceMemory GetMemory() const;
  void *GetMapped() const;

  void Write(void *data);

private:
  const RenderContext *m_pContext;

  vk::DeviceSize m_Size;
  vk::BufferUsageFlags m_Usage;
  vk::MemoryPropertyFlags m_Properties;

  vk::Buffer m_Buffer = nullptr;
  vk::DeviceMemory m_Memory = nullptr;
  void *m_Mapped = nullptr;

  void Create();
  void Free();
};
} // namespace Core
} // namespace SimpleEngine
