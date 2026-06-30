#include "core/resource/mesh.hpp"
#include "core/render_context.hpp"
#include "core/resource/resource.hpp"
#include "helpers/model_loader.hpp"
#include "helpers/vulkan_helper.hpp"
#include "vulkan/vulkan.hpp"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace SimpleEngine {
namespace Core {
Mesh::Mesh(const std::string &id, const RenderContext &renderContext,
           const std::string &modelPath)
    : Resource(id, renderContext), modelPath(modelPath) {}

Mesh::~Mesh() { unload(); }

bool Mesh::loadMeshData(const std::string &path,
                        std::vector<Mesh::Vertex> &vertices,
                        std::vector<uint32_t> &indices) {
  Helper::ModelLoader::loadGltfMeshData(path, vertices, indices);
  return true;
};

void Mesh::createVertexBuffer(std::vector<Mesh::Vertex> &vertices) {
  vk::Device device = renderContext.device;
  vk::DeviceSize bufferSize = sizeof(Mesh::Vertex) * vertices.size();

  const auto &[stagingBuffer, stagingBufferMemory] =
      Helper::VulkanHelper::createBuffer(
          bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
          vk::MemoryPropertyFlagBits::eHostVisible |
              vk::MemoryPropertyFlagBits::eHostCoherent,
          renderContext);

  void *dataStaging = device.mapMemory(stagingBufferMemory, 0, bufferSize);
  memcpy(dataStaging, vertices.data(), bufferSize);
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

void Mesh::createIndexBuffer(std::vector<uint32_t> &indices) {
  vk::Device device = renderContext.device;
  vk::DeviceSize bufferSize = sizeof(uint32_t) * indices.size();

  const auto &[stagingBuffer, stagingBufferMemory] =
      Helper::VulkanHelper::createBuffer(
          bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
          vk::MemoryPropertyFlagBits::eHostVisible |
              vk::MemoryPropertyFlagBits::eHostCoherent,
          renderContext);

  void *dataStaging = device.mapMemory(stagingBufferMemory, 0, bufferSize);
  memcpy(dataStaging, indices.data(), bufferSize);
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
  std::vector<Vertex> _vertices;
  std::vector<uint32_t> _indices;

  if (!loadMeshData(modelPath, _vertices, _indices)) {
    return false;
  }

  createVertexBuffer(_vertices);
  createIndexBuffer(_indices);

  vertexCount = static_cast<uint32_t>(_vertices.size());
  indexCount = static_cast<uint32_t>(_indices.size());

  return true;
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
