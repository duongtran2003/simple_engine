#include "core/resource/texture.hpp"
#include "core/raw_texture.hpp"
#include "core/render_context.hpp"
#include "core/resource/resource.hpp"
#include "enums/texture.hpp"
#include "helpers/vulkan_helper.hpp"
#include "vulkan/vulkan.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stb_image.h>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace SimpleEngine {
namespace Core {
Texture::Texture(const std::string &id, const RenderContext &renderContext)
    : Resource(id, renderContext) {}
Texture::Texture(const std::string &id, const RenderContext &renderContext,
                 RawTexture raw)
    : Resource(id, renderContext), rawTexture(std::move(raw)) {
  source = Source::FromRawTexture;
}

Texture::Texture(const std::string &id, const RenderContext &renderContext,
                 unsigned char *pixels, uint32_t width, uint32_t height,
                 uint32_t channels)
    : Resource(id, renderContext) {
  unsigned char *buf = new unsigned char[width * height * channels];
  memcpy(buf, pixels, width * height * channels);
  this->pixels = buf;

  this->width = width;
  this->height = height;
  this->channels = channels;
  source = Source::FromPixels;
}

void Texture::generateMipmaps(vk::CommandBuffer &commandBuffer) {
  vk::FormatProperties formatProperties =
      renderContext.physicalDevice.getFormatProperties(format);
  if (!(formatProperties.optimalTilingFeatures &
        vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
    throw std::runtime_error(
        "Texture::generateMipmaps::ERROR: Format is not supported.");
  }

  uint32_t mipHeight = height;
  uint32_t mipWidth = width;
  for (uint32_t i = 1; i < mipLevels; i++) {
    Helper::VulkanHelper::transitionImageLayout(
        commandBuffer, image, 1, i - 1, 1, 0,
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eTransferSrcOptimal, vk::ImageAspectFlagBits::eColor);

    vk::ArrayWrapper1D<vk::Offset3D, 2> srcOffsets, dstOffsets;

    srcOffsets[0] = vk::Offset3D(0, 0, 0);
    srcOffsets[1] = vk::Offset3D(mipWidth, mipHeight, 1);

    dstOffsets[0] = vk::Offset3D(0, 0, 0);
    dstOffsets[1] = vk::Offset3D(mipWidth > 1 ? mipWidth / 2 : 1,
                                 mipHeight > 1 ? mipHeight / 2 : 1, 1);

    vk::ImageSubresourceLayers blitSrcSubResource = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .mipLevel = i - 1,
        .baseArrayLayer = 0,
        .layerCount = 1};

    vk::ImageSubresourceLayers blitDstSubResource = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .mipLevel = i,
        .baseArrayLayer = 0,
        .layerCount = 1};

    vk::ImageBlit blit = {.srcSubresource = blitSrcSubResource,
                          .srcOffsets = srcOffsets,
                          .dstSubresource = blitDstSubResource,
                          .dstOffsets = dstOffsets};

    commandBuffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image,
                            vk::ImageLayout::eTransferDstOptimal, {blit},
                            vk::Filter::eLinear);

    Helper::VulkanHelper::transitionImageLayout(
        commandBuffer, image, 1, i - 1, 1, 0,
        vk::ImageLayout::eTransferSrcOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::ImageAspectFlagBits::eColor);

    if (mipWidth > 1) {
      mipWidth /= 2;
    }

    if (mipHeight > 1) {
      mipHeight /= 2;
    }
  }

  Helper::VulkanHelper::transitionImageLayout(
      commandBuffer, image, 1, mipLevels - 1, 1, 0,
      vk::ImageLayout::eTransferDstOptimal,
      vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor);
}

void Texture::readFromRawTexture() {
  if (rawTexture.componentCount == 1) {
    format = rawTexture.colorSpace == Enums::Texture::ColorSpace::Linear
                 ? vk::Format::eR8Unorm
                 : vk::Format::eR8Srgb;
  } else if (rawTexture.componentCount == 2) {
    format = rawTexture.colorSpace == Enums::Texture::ColorSpace::Linear
                 ? vk::Format::eR8G8Unorm
                 : vk::Format::eR8G8Srgb;
  } else if (rawTexture.componentCount == 3) {
    format = rawTexture.colorSpace == Enums::Texture::ColorSpace::Linear
                 ? vk::Format::eR8G8B8Unorm
                 : vk::Format::eR8G8B8Srgb;
  } else {
    format = rawTexture.colorSpace == Enums::Texture::ColorSpace::Linear
                 ? vk::Format::eR8G8B8A8Unorm
                 : vk::Format::eR8G8B8A8Srgb;
  }

  width = rawTexture.width;
  height = rawTexture.height;
  channels = rawTexture.componentCount;
  mipLevels = rawTexture.useMipmap
                  ? static_cast<uint32_t>(
                        std::floor(std::log2(std::max(width, height))) + 1)
                  : 1;

  auto [newImage, newImageMemory, newImageView] =
      Helper::VulkanHelper::createImage(
          rawTexture.width, rawTexture.height, mipLevels, 1, format,
          vk::ImageUsageFlagBits::eTransferDst |
              vk::ImageUsageFlagBits::eTransferSrc |
              vk::ImageUsageFlagBits::eSampled,
          vk::ImageAspectFlagBits::eColor, vk::SampleCountFlagBits::e1,
          renderContext);

  vk::DeviceSize imageSize = rawTexture.pixels.size();
  auto [stagingBuffer, stagingMemory] = Helper::VulkanHelper::createBuffer(
      imageSize, vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent,
      renderContext);

  void *data = renderContext.device.mapMemory(stagingMemory, 0, imageSize);
  memcpy(data, rawTexture.pixels.data(), static_cast<size_t>(imageSize));
  renderContext.device.unmapMemory(stagingMemory);

  vk::CommandBuffer commandBuffer =
      Helper::VulkanHelper::beginSingleTimeCommands(renderContext);

  Helper::VulkanHelper::transitionImageLayout(
      commandBuffer, newImage, mipLevels, 0, 1, 0, vk::ImageLayout::eUndefined,
      vk::ImageLayout::eTransferDstOptimal, vk::ImageAspectFlagBits::eColor);
  Helper::VulkanHelper::copyBufferToImage(
      commandBuffer, stagingBuffer, newImage, rawTexture.width,
      rawTexture.height, vk::ImageAspectFlagBits::eColor);

  image = newImage;
  memory = newImageMemory;
  imageView = newImageView;
  offset = imageSize;
  sampler = Helper::VulkanHelper::createImageSampler(
      rawTexture.magFilter, rawTexture.minFilter, rawTexture.wrapS,
      rawTexture.wrapT, renderContext);

  if (rawTexture.useMipmap) {
    generateMipmaps(commandBuffer);
  } else {
    Helper::VulkanHelper::transitionImageLayout(
        commandBuffer, newImage, mipLevels, 0, 1, 0,
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::ImageAspectFlagBits::eColor);
  }

  Helper::VulkanHelper::endSingleTimeCommands(commandBuffer, renderContext);

  renderContext.graphicsQueue.waitIdle();
  renderContext.device.destroyBuffer(stagingBuffer);
  renderContext.device.freeMemory(stagingMemory);

  // Clean up
  rawTexture.pixels.clear();
  rawTexture.pixels.shrink_to_fit();
  rawTexture = {};
}

void Texture::readFromPixels() {
  mipLevels = 1;

  auto [newImage, newImageMemory, newImageView] =
      Helper::VulkanHelper::createImage(
          width, height, mipLevels, 1, vk::Format::eR8G8B8A8Unorm,
          vk::ImageUsageFlagBits::eTransferDst |
              vk::ImageUsageFlagBits::eSampled,
          vk::ImageAspectFlagBits::eColor, vk::SampleCountFlagBits::e1,
          renderContext);

  vk::DeviceSize imageSize = width * height * channels;
  auto [stagingBuffer, stagingMemory] = Helper::VulkanHelper::createBuffer(
      imageSize, vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent,
      renderContext);

  void *data = renderContext.device.mapMemory(stagingMemory, 0, imageSize);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  renderContext.device.unmapMemory(stagingMemory);

  vk::CommandBuffer commandBuffer =
      Helper::VulkanHelper::beginSingleTimeCommands(renderContext);

  Helper::VulkanHelper::transitionImageLayout(
      commandBuffer, newImage, mipLevels, 0, 1, 0, vk::ImageLayout::eUndefined,
      vk::ImageLayout::eTransferDstOptimal, vk::ImageAspectFlagBits::eColor);
  Helper::VulkanHelper::copyBufferToImage(commandBuffer, stagingBuffer,
                                          newImage, width, height,
                                          vk::ImageAspectFlagBits::eColor);

  image = newImage;
  memory = newImageMemory;
  imageView = newImageView;
  offset = imageSize;
  sampler = Helper::VulkanHelper::createImageSampler(
      Enums::Texture::Filter::Linear, Enums::Texture::Filter::Linear,
      Enums::Texture::Wrap::ClampToEdge, Enums::Texture::Wrap::ClampToEdge,
      renderContext);

  Helper::VulkanHelper::transitionImageLayout(
      commandBuffer, newImage, mipLevels, 0, 1, 0,
      vk::ImageLayout::eTransferDstOptimal,
      vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor);

  Helper::VulkanHelper::endSingleTimeCommands(commandBuffer, renderContext);

  renderContext.graphicsQueue.waitIdle();
  renderContext.device.destroyBuffer(stagingBuffer);
  renderContext.device.freeMemory(stagingMemory);

  free(pixels);
}

bool Texture::doLoad() {
  bool hasLoaded = false;

  if (source == Source::FromRawTexture) {
    readFromRawTexture();
    hasLoaded = true;
  } else if (source == Source::FromPixels) {
    readFromPixels();
    hasLoaded = true;
  }

  return hasLoaded;
}

void Texture::doUnload() {
  renderContext.device.destroySampler(sampler);
  renderContext.device.destroyImageView(imageView);
  renderContext.device.destroyImage(image);
  renderContext.device.freeMemory(memory);
}

vk::Image Texture::getImage() const { return image; }
vk::Sampler Texture::getSampler() const { return sampler; }
vk::ImageView Texture::getImageView() const { return imageView; }
} // namespace Core
} // namespace SimpleEngine
