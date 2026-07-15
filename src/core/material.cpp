#include "core/material.hpp"
#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <glm/ext/vector_float4.hpp>

namespace SimpleEngine {
namespace Core {
Material::Material() {}

Material *Material::setAlbedo(TextureBinding binding) {
  albedo = binding;
  hasAlbedoTexture = true;
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
  hasNormalTexture = true;
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

Material *Material::setMetallicRoughness(TextureBinding binding) {
  metallicRoughness = binding;
  hasMetallicRoughnessTexture = true;
  return this;
}

const Material::TextureBinding &Material::getMetallicRoughness() const {
  return metallicRoughness;
}

Material *Material::registerMetallicRoughness(vk::DescriptorSet &set,
                                              uint32_t index,
                                              const RenderContext &context) {
  metallicRoughness.index = index;
  vk::DescriptorImageInfo imageInfo{
      .sampler = metallicRoughness.handle->getSampler(),
      .imageView = metallicRoughness.handle->getImageView(),
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

Material *Material::setPbrBaseColorFactor(glm::vec4 factor) {
  pbrProperty.baseColorFactor = factor;
  return this;
}

const glm::vec4 Material::getPbrBaseColorFactor() const {
  return pbrProperty.baseColorFactor;
}

Material *Material::setPbrMetallicFactor(float factor) {
  pbrProperty.metallicFactor = factor;
  return this;
}

float Material::getPbrMetallicFactor() const {
  return pbrProperty.metallicFactor;
}

Material *Material::setPbrRoughnessFactor(float factor) {
  pbrProperty.roughnessFactor = factor;
  return this;
}

float Material::getPbrRoughnessFactor() const {
  return pbrProperty.roughnessFactor;
}

bool Material::hasAlbedo() const { return hasAlbedoTexture; }

bool Material::hasNormal() const { return hasNormalTexture; }

bool Material::hasMetallicRoughness() const {
  return hasMetallicRoughnessTexture;
}
} // namespace Core
} // namespace SimpleEngine
