#include "core/material.hpp"
#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>

namespace SimpleEngine {
namespace Core {
Material::Material() {}

Material *Material::setAlbedo(TextureBinding binding) {
  albedo = binding;
  return this;
}

const Material::TextureBinding &Material::getAlbedo() const { return albedo; }

Material *Material::registerAlbedo(vk::DescriptorSet &set, uint32_t index,
                                   const RenderContext &context) {
  albedo.index = index;
  vk::DescriptorImageInfo imageInfo{
      .sampler = albedo.handle->getSampler(),
      .imageView = albedo.handle->getImageView(),
      .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal};

  vk::WriteDescriptorSet descriptorWrite{
      .dstSet = set,
      .dstBinding = 0,
      .dstArrayElement = index,
      .descriptorCount = 1,
      .descriptorType = vk::DescriptorType::eCombinedImageSampler,
      .pImageInfo = &imageInfo};

  context.device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

  return this;
}

Material *Material::setNormal(TextureBinding binding) {
  normal = binding;
  return this;
}

const Material::TextureBinding &Material::getNormal() const { return normal; }

Material *Material::registerNormal(vk::DescriptorSet &set, uint32_t index,
                                   const RenderContext &context) {
  normal.index = index;
  vk::DescriptorImageInfo imageInfo{
      .sampler = normal.handle->getSampler(),
      .imageView = normal.handle->getImageView(),
      .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal};

  vk::WriteDescriptorSet descriptorWrite{
      .dstSet = set,
      .dstBinding = 0,
      .dstArrayElement = index,
      .descriptorCount = 1,
      .descriptorType = vk::DescriptorType::eCombinedImageSampler,
      .pImageInfo = &imageInfo};

  context.device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

  return this;
}
} // namespace Core
} // namespace SimpleEngine
