#include "core/image/image_sampler.hpp"
#include "core/render_context.hpp"
#include "enums/image_enums.hpp"
#include "vulkan/vulkan.hpp"
#include <algorithm>

namespace SimpleEngine {
namespace Core {
ImageSampler::ImageSampler(const CreateInfo &createInfo,
                           const RenderContext *context) {
  m_magFilter = createInfo.magFilter;
  m_minFilter = createInfo.minFilter;

  m_wrapU = createInfo.wrapU;
  m_wrapV = createInfo.wrapV;
  m_wrapW = createInfo.wrapW;

  m_anisotropyEnable = createInfo.anisotropyEnable;
  m_compareEnable = createInfo.compareEnable;
  m_compareOp = createInfo.compareOp;

  m_pContext = context;

  Create();
}

ImageSampler::~ImageSampler() {};
void ImageSampler::Free() {
  if (!m_isValid) {
    return;
  }

  m_pContext->device.destroySampler(m_sampler);
  m_isValid = false;
}

bool ImageSampler::IsValid() { return m_isValid; }

void ImageSampler::Create() {
  vk::Filter vkMagFilter, vkMinFilter;
  if (m_magFilter == Enums::Image::Filter::eLinear) {
    vkMagFilter = vk::Filter::eLinear;
  } else {
    vkMagFilter = vk::Filter::eNearest;
  }
  if (m_minFilter == Enums::Image::Filter::eLinear) {
    vkMinFilter = vk::Filter::eLinear;
  } else {
    vkMinFilter = vk::Filter::eNearest;
  }

  vk::SamplerAddressMode vkUWrap, vkVWrap, vkWWrap;
  if (m_wrapU == Enums::Image::Wrap::eMirroredRepeat) {
    vkUWrap = vk::SamplerAddressMode::eMirroredRepeat;
  } else if (m_wrapU == Enums::Image::Wrap::eRepeat) {
    vkUWrap = vk::SamplerAddressMode::eRepeat;
  } else {
    vkUWrap = vk::SamplerAddressMode::eClampToEdge;
  }
  if (m_wrapV == Enums::Image::Wrap::eMirroredRepeat) {
    vkVWrap = vk::SamplerAddressMode::eMirroredRepeat;
  } else if (m_wrapV == Enums::Image::Wrap::eRepeat) {
    vkVWrap = vk::SamplerAddressMode::eRepeat;
  } else {
    vkVWrap = vk::SamplerAddressMode::eClampToEdge;
  }
  if (m_wrapW == Enums::Image::Wrap::eMirroredRepeat) {
    vkWWrap = vk::SamplerAddressMode::eMirroredRepeat;
  } else if (m_wrapW == Enums::Image::Wrap::eRepeat) {
    vkWWrap = vk::SamplerAddressMode::eRepeat;
  } else {
    vkWWrap = vk::SamplerAddressMode::eClampToEdge;
  }

  vk::PhysicalDeviceProperties deviceProperties =
      m_pContext->physicalDevice.getProperties();
  float maxAnisotropy =
      std::min(16.0f, deviceProperties.limits.maxSamplerAnisotropy);

  vk::SamplerCreateInfo info{.magFilter = vkMagFilter,
                             .minFilter = vkMinFilter,
                             .addressModeU = vkUWrap,
                             .addressModeV = vkVWrap,
                             .addressModeW = vkWWrap,
                             .mipLodBias = -0.5f,
                             .anisotropyEnable = m_anisotropyEnable,
                             .maxAnisotropy = maxAnisotropy,
                             .compareEnable = m_compareEnable,
                             .compareOp = m_compareOp,
                             .minLod = 0.0f,
                             .maxLod = vk::LodClampNone,
                             .borderColor = vk::BorderColor::eIntOpaqueBlack,
                             .unnormalizedCoordinates = vk::False};

  m_sampler = m_pContext->device.createSampler(info);
  m_isValid = true;
}
} // namespace Core
} // namespace SimpleEngine
