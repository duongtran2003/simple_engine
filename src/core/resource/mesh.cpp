#include "core/resource/mesh.hpp"
#include "core/raw_texture.hpp"
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
                        std::vector<uint32_t> &indices,
                        std::vector<RawTexture> &textures) {
  Helper::ModelLoader::loadglTF(path, vertices, indices, textures);
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

void Mesh::createMeshTextures(std::vector<RawTexture> &textures) {
  assert(meshTextures.empty());
  for (const auto &raw : textures) {
    vk::Format textureFormat;
    if (raw.componentCount == 1) {
      textureFormat = vk::Format::eR8Srgb;
    } else if (raw.componentCount == 2) {
      textureFormat = vk::Format::eR8G8Srgb;
    } else if (raw.componentCount == 3) {
      textureFormat = vk::Format::eR8G8B8Srgb;
    } else {
      textureFormat = vk::Format::eR8G8B8A8Srgb;
    }
    auto [image, imageMemory, imageView] = Helper::VulkanHelper::createImage(
        raw.width, raw.height, textureFormat,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::ImageAspectFlagBits::eColor, renderContext);

    vk::DeviceSize imageSize = raw.pixels.size();
    auto [stagingBuffer, stagingMemory] = Helper::VulkanHelper::createBuffer(
        imageSize, vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent,
        renderContext);

    void *data = renderContext.device.mapMemory(stagingMemory, 0, imageSize);
    memcpy(data, raw.pixels.data(), static_cast<size_t>(imageSize));
    renderContext.device.unmapMemory(stagingMemory);

    vk::CommandBuffer commandBuffer =
        Helper::VulkanHelper::beginSingleTimeCommands(renderContext);

    Helper::VulkanHelper::transitionImageLayout(
        commandBuffer, image, vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal, vk::ImageAspectFlagBits::eColor);
    Helper::VulkanHelper::copyBufferToImage(commandBuffer, stagingBuffer, image,
                                            raw.width, raw.height,
                                            vk::ImageAspectFlagBits::eColor);
    Helper::VulkanHelper::transitionImageLayout(
        commandBuffer, image, vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::ImageAspectFlagBits::eColor);

    Helper::VulkanHelper::endSingleTimeCommands(commandBuffer, renderContext);

    renderContext.device.destroyBuffer(stagingBuffer);
    renderContext.device.freeMemory(stagingMemory);

    MeshTexture texture{
        .image = image,
        .memory = imageMemory,
        .view = imageView,
        .sampler = Helper::VulkanHelper::createImageSampler(
            raw.magFilter, raw.minFilter, raw.wrapS, raw.wrapT, renderContext)};

    meshTextures.push_back(texture);
  }
}

void Mesh::allocateTextureDescriptorSet(vk::DescriptorSetLayout layout) {
  if (meshTextures.empty()) {
    return;
  }

  vk::DescriptorSetAllocateInfo allocateInfo{.descriptorPool =
                                                 renderContext.descriptorPool,
                                             .descriptorSetCount = 1,
                                             .pSetLayouts = &layout};

  auto allocatedSets =
      renderContext.device.allocateDescriptorSets(allocateInfo);
  textureDescriptorSet = allocatedSets[0];

  vk::DescriptorImageInfo imageInfo{
      .sampler = meshTextures[0].sampler,
      .imageView = meshTextures[0].view,
      .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal};

  vk::WriteDescriptorSet descriptorWrite{
      .dstSet = textureDescriptorSet,
      .dstBinding = 0,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = vk::DescriptorType::eCombinedImageSampler,
      .pImageInfo = &imageInfo};

  renderContext.device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
}

vk::DescriptorSet Mesh::getTextureDescriptorSet() const {
  return textureDescriptorSet;
}

vk::Buffer Mesh::getVertexBuffer() const { return vertexBuffer; }

vk::Buffer Mesh::getIndexBuffer() const { return indexBuffer; }

uint32_t Mesh::getVertexCount() const { return vertexCount; }

uint32_t Mesh::getIndexCount() const { return indexCount; }

bool Mesh::doLoad() {
  std::vector<Vertex> _vertices;
  std::vector<uint32_t> _indices;
  std::vector<RawTexture> _textures;

  if (!loadMeshData(modelPath, _vertices, _indices, _textures)) {
    return false;
  }

  std::cout << "Loaded textures: " << _textures.size() << "\n";

  createVertexBuffer(_vertices);
  createIndexBuffer(_indices);
  createMeshTextures(_textures);

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

    for (auto &texture : meshTextures) {
      device.destroyImageView(texture.view);
      device.destroyImage(texture.image);
      device.freeMemory(texture.memory);
      device.destroySampler(texture.sampler);
    }
  }
}
} // namespace Core
} // namespace SimpleEngine
