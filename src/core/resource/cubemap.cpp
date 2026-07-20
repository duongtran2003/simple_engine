#include "core/resource/cubemap.hpp"
#include "core/render_context.hpp"
#include "core/resource/resource.hpp"
#include "enums/texture.hpp"
#include "helpers/vulkan_helper.hpp"
#include "stb_image.h"
#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Core {
Cubemap::Cubemap(const std::string &id, const RenderContext &renderContext)
    : Resource(id, renderContext) {}
Cubemap::Cubemap(const std::string &id, const RenderContext &renderContext,
                 const std::string &path)
    : Resource(id, renderContext) {
  this->path = path;
  source = Source::fromFile;
}

bool Cubemap::doLoad() {
  bool hasLoaded = false;

  if (source == Source::fromFile) {
    readFromFile();
    hasLoaded = true;
  }

  return hasLoaded;
}

void Cubemap::doUnload() {
  renderContext.device.destroySampler(sampler);
  renderContext.device.destroyImageView(imageView);
  renderContext.device.destroyImage(image);
  renderContext.device.freeMemory(memory);
}

vk::Image Cubemap::getImage() const { return image; }
vk::Sampler Cubemap::getSampler() const { return sampler; }
vk::ImageView Cubemap::getImageView() const { return imageView; }

void Cubemap::readFromFile() {
  std::vector<std::string> faces = {
      "px.hdr", "nx.hdr", "py.hdr", "ny.hdr", "pz.hdr", "nz.hdr",
  };

  int width = 0;
  int height = 0;

  std::filesystem::path firstFacePath = std::filesystem::path(path) / faces[0];
  stbi_info(firstFacePath.c_str(), &width, &height, nullptr);
  vk::DeviceSize faceSize = width * height * 4 * sizeof(float);
  vk::DeviceSize totalSize = faceSize * 6;

  auto [stagingBuffer, stagingMemory] = Helper::VulkanHelper::createBuffer(
      totalSize, vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent,
      renderContext);
  void *stagingBufferMapped =
      renderContext.device.mapMemory(stagingMemory, 0, totalSize);
  for (size_t i = 0; i < 6; i++) {
    int w, h, channels;

    std::filesystem::path facePath = std::filesystem::path(path) / faces[i];
    float *data = stbi_loadf(facePath.c_str(), &w, &h, &channels, 4);
    if (!data) {
      throw std::runtime_error("Failed to load cubemap face: " +
                               facePath.string());
    }

    uint8_t *ptr = static_cast<uint8_t *>(stagingBufferMapped) + (i * faceSize);
    memcpy(ptr, data, faceSize);
    stbi_image_free(data);
  }

  renderContext.device.unmapMemory(stagingMemory);

  this->width = width;
  this->height = height;
  this->channels = 4;
  this->format = vk::Format::eR32G32B32A32Sfloat;

  auto [newImage, newImageMemory, newImageView] =
      Helper::VulkanHelper::createCubemapImage(
          width, 1, format,
          vk::ImageUsageFlagBits::eTransferDst |
              vk::ImageUsageFlagBits::eSampled,
          vk::ImageAspectFlagBits::eColor, renderContext);

  vk::CommandBuffer commandBuffer =
      Helper::VulkanHelper::beginSingleTimeCommands(renderContext);

  Helper::VulkanHelper::transitionImageLayout(
      commandBuffer, newImage, 1, 0, 6, 0, vk::ImageLayout::eUndefined,
      vk::ImageLayout::eTransferDstOptimal, vk::ImageAspectFlagBits::eColor);

  std::vector<vk::BufferImageCopy> bufferCopyRegions;
  bufferCopyRegions.resize(6);

  for (size_t i = 0; i < 6; i++) {
    vk::BufferImageCopy region{
        .bufferOffset = i * faceSize,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                             .mipLevel = 0,
                             .baseArrayLayer = static_cast<uint32_t>(i),
                             .layerCount = 1},
        .imageOffset = {0, 0, 0},
        .imageExtent = {.width = static_cast<uint32_t>(width),
                        .height = static_cast<uint32_t>(height),
                        .depth = 1}};

    bufferCopyRegions[i] = region;
  }

  commandBuffer.copyBufferToImage(
      stagingBuffer, newImage, vk::ImageLayout::eTransferDstOptimal,
      bufferCopyRegions.size(), bufferCopyRegions.data());

  Helper::VulkanHelper::transitionImageLayout(
      commandBuffer, newImage, 1, 0, 6, 0, vk::ImageLayout::eTransferDstOptimal,
      vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor);

  Helper::VulkanHelper::endSingleTimeCommands(commandBuffer, renderContext);

  renderContext.graphicsQueue.waitIdle();
  renderContext.device.destroyBuffer(stagingBuffer);
  renderContext.device.freeMemory(stagingMemory);

  this->image = newImage;
  this->memory = newImageMemory;
  this->imageView = newImageView;

  this->sampler = Helper::VulkanHelper::createImageSampler(
      Enums::Texture::Filter::Linear, Enums::Texture::Filter::Linear,
      Enums::Texture::Wrap::ClampToEdge, Enums::Texture::Wrap::ClampToEdge,
      renderContext);
}
} // namespace Core
} // namespace SimpleEngine
