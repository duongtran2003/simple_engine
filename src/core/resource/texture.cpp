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
#include <vector>
#include <vulkan/vulkan_core.h>

namespace SimpleEngine {
namespace Core {
Texture::Texture(const std::string &id, const RenderContext &renderContext)
    : Resource(id, renderContext) {}
Texture::Texture(const std::string &id, const RenderContext &renderContext,
                 RawTexture &raw)
    : Resource(id, renderContext) {
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
  auto [newImage, newImageMemory, newImageView] =
      Helper::VulkanHelper::createImage(raw.width, raw.height, textureFormat,
                                        vk::ImageUsageFlagBits::eTransferDst |
                                            vk::ImageUsageFlagBits::eSampled,
                                        vk::ImageAspectFlagBits::eColor,
                                        vk::SampleCountFlagBits::e1,
                                        renderContext);

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
      commandBuffer, newImage, vk::ImageLayout::eUndefined,
      vk::ImageLayout::eTransferDstOptimal, vk::ImageAspectFlagBits::eColor);
  Helper::VulkanHelper::copyBufferToImage(commandBuffer, stagingBuffer,
                                          newImage, raw.width, raw.height,
                                          vk::ImageAspectFlagBits::eColor);
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
      raw.magFilter, raw.minFilter, raw.wrapS, raw.wrapT, renderContext);
  this->width = raw.width;
  this->height = raw.height;
  this->channels = raw.componentCount;
  this->offset = imageSize;
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

bool Texture::doLoad() { return true; }

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
