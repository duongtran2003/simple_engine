#include "core/resource/texture.hpp"
#include "core/raw_texture.hpp"
#include "core/render_context.hpp"
#include "core/resource/resource.hpp"
#include "helpers/vulkan_helper.hpp"
#include "vulkan/vulkan.hpp"
#include <cstddef>
#include <cstring>
#include <stb_image.h>
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
  source = Source::fromRawTexture;
}

void Texture::readFromRawTexture() {
  vk::Format textureFormat;
  if (rawTexture.componentCount == 1) {
    textureFormat = rawTexture.colorSpace == ColorSpace::Linear
                        ? vk::Format::eR8Unorm
                        : vk::Format::eR8Srgb;
  } else if (rawTexture.componentCount == 2) {
    textureFormat = rawTexture.colorSpace == ColorSpace::Linear
                        ? vk::Format::eR8G8Unorm
                        : vk::Format::eR8G8Srgb;
  } else if (rawTexture.componentCount == 3) {
    textureFormat = rawTexture.colorSpace == ColorSpace::Linear
                        ? vk::Format::eR8G8B8Unorm
                        : vk::Format::eR8G8B8Srgb;
  } else {
    textureFormat = rawTexture.colorSpace == ColorSpace::Linear
                        ? vk::Format::eR8G8B8A8Unorm
                        : vk::Format::eR8G8B8A8Srgb;
  }
  auto [newImage, newImageMemory, newImageView] =
      Helper::VulkanHelper::createImage(
          rawTexture.width, rawTexture.height, textureFormat,
          vk::ImageUsageFlagBits::eTransferDst |
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
      commandBuffer, newImage, vk::ImageLayout::eUndefined,
      vk::ImageLayout::eTransferDstOptimal, vk::ImageAspectFlagBits::eColor);
  Helper::VulkanHelper::copyBufferToImage(
      commandBuffer, stagingBuffer, newImage, rawTexture.width,
      rawTexture.height, vk::ImageAspectFlagBits::eColor);
  Helper::VulkanHelper::transitionImageLayout(
      commandBuffer, newImage, vk::ImageLayout::eTransferDstOptimal,
      vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor);

  Helper::VulkanHelper::endSingleTimeCommands(commandBuffer, renderContext);

  renderContext.device.destroyBuffer(stagingBuffer);
  renderContext.device.freeMemory(stagingMemory);

  this->image = newImage;
  this->memory = newImageMemory;
  this->imageView = newImageView;
  this->sampler = Helper::VulkanHelper::createImageSampler(
      rawTexture.magFilter, rawTexture.minFilter, rawTexture.wrapS,
      rawTexture.wrapT, renderContext);
  this->width = rawTexture.width;
  this->height = rawTexture.height;
  this->channels = rawTexture.componentCount;
  this->offset = imageSize;

  // Clean up
  rawTexture.pixels.clear();
  rawTexture.pixels.shrink_to_fit();
  rawTexture = {};
}

unsigned char *Texture::loadImageData(const std::string &path, int &_width,
                                      int &_height, int &_channels) {
  unsigned char *data =
      stbi_load(path.c_str(), &_width, &_height, &_channels, STBI_rgb_alpha);

  return data;
}

void Texture::unloadImageData(unsigned char *data) {
  if (data) {
    stbi_image_free(data);
  }
}

bool Texture::doLoad() {
  if (source == Source::fromRawTexture) {
    readFromRawTexture();
    return true;
  }

  return false;
}

void Texture::doUnload() {
  renderContext.device.destroySampler(sampler);
  renderContext.device.destroyImageView(imageView);
  renderContext.device.destroyImage(image);
  renderContext.device.freeMemory(memory);
}

void Texture::createVulkanImage(unsigned char *data, int &width, int &height,
                                int &channels) {
  // TODO: Implement later
}

vk::Image Texture::getImage() const { return image; }
vk::Sampler Texture::getSampler() const { return sampler; }
vk::ImageView Texture::getImageView() const { return imageView; }
} // namespace Core
} // namespace SimpleEngine
