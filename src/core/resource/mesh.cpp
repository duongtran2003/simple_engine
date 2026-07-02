#include "core/resource/mesh.hpp"
#include "core/render_context.hpp"
#include "core/resource/resource.hpp"
#include "helpers/vulkan_helper.hpp"
#include "vulkan/vulkan.hpp"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

namespace SimpleEngine {
namespace Core {
Mesh::Mesh(const std::string &id, const RenderContext &renderContext,
           const std::string &path)
    : Resource(id, renderContext), path(path) {
  source = Source::fromFile;
}

Mesh::Mesh(const std::string &id, const RenderContext &renderContext,
           std::vector<Vertex> vertices, std::vector<uint32_t> indices)
    : Resource(id, renderContext), vertices(std::move(vertices)),
      indices(std::move(indices)) {
  source = Source::fromMemory;
}

void Mesh::createVertexBuffer(std::vector<Mesh::Vertex> &v) {
  vk::Device device = renderContext.device;
  vk::DeviceSize bufferSize = sizeof(Mesh::Vertex) * v.size();

  const auto &[stagingBuffer, stagingBufferMemory] =
      Helper::VulkanHelper::createBuffer(
          bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
          vk::MemoryPropertyFlagBits::eHostVisible |
              vk::MemoryPropertyFlagBits::eHostCoherent,
          renderContext);

  void *dataStaging = device.mapMemory(stagingBufferMemory, 0, bufferSize);
  memcpy(dataStaging, v.data(), bufferSize);
  device.unmapMemory(stagingBufferMemory);

  const auto &[buffer, memory] = Helper::VulkanHelper::createBuffer(
      bufferSize,
      vk::BufferUsageFlagBits::eVertexBuffer |
          vk::BufferUsageFlagBits::eTransferDst,
      vk::MemoryPropertyFlagBits::eDeviceLocal, renderContext);

  vertexBuffer = std::move(buffer);
  vertexBufferMemory = std::move(memory);

  Helper::VulkanHelper::copyBuffer(stagingBuffer, vertexBuffer, bufferSize,
                                   renderContext);
  device.destroyBuffer(stagingBuffer);
  device.freeMemory(stagingBufferMemory);
}

void Mesh::createIndexBuffer(std::vector<uint32_t> &i) {
  vk::Device device = renderContext.device;
  vk::DeviceSize bufferSize = sizeof(uint32_t) * i.size();

  const auto &[stagingBuffer, stagingBufferMemory] =
      Helper::VulkanHelper::createBuffer(
          bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
          vk::MemoryPropertyFlagBits::eHostVisible |
              vk::MemoryPropertyFlagBits::eHostCoherent,
          renderContext);

  void *dataStaging = device.mapMemory(stagingBufferMemory, 0, bufferSize);
  memcpy(dataStaging, i.data(), bufferSize);
  device.unmapMemory(stagingBufferMemory);

  const auto &[buffer, memory] = Helper::VulkanHelper::createBuffer(
      bufferSize,
      vk::BufferUsageFlagBits::eIndexBuffer |
          vk::BufferUsageFlagBits::eTransferDst,
      vk::MemoryPropertyFlagBits::eDeviceLocal, renderContext);

  indexBuffer = std::move(buffer);
  indexBufferMemory = std::move(memory);

  Helper::VulkanHelper::copyBuffer(stagingBuffer, indexBuffer, bufferSize,
                                   renderContext);
  device.destroyBuffer(stagingBuffer);
  device.freeMemory(stagingBufferMemory);
}

vk::Buffer Mesh::getVertexBuffer() const { return vertexBuffer; }

vk::Buffer Mesh::getIndexBuffer() const { return indexBuffer; }

uint32_t Mesh::getVertexCount() const { return vertexCount; }

uint32_t Mesh::getIndexCount() const { return indexCount; }

bool Mesh::doLoad() {
  if (source == Source::fromMemory) {
    createVertexBuffer(vertices);
    createIndexBuffer(indices);

    vertexCount = static_cast<uint32_t>(vertices.size());
    indexCount = static_cast<uint32_t>(indices.size());

    // Clean up
    vertices.clear();
    vertices.shrink_to_fit();

    indices.clear();
    indices.shrink_to_fit();

    return true;
  }

  return false;
}

void Mesh::doUnload() {
  if (isLoaded()) {
    vk::Device device = getRenderContext().device;

    device.destroyBuffer(indexBuffer);
    device.freeMemory(indexBufferMemory);

    device.destroyBuffer(vertexBuffer);
    device.freeMemory(vertexBufferMemory);
  }
}
} // namespace Core
} // namespace SimpleEngine
