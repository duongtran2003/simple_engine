#include "core/image/image_sampler.hpp"
#include "core/render_context.hpp"
#include "enums/image_enums.hpp"
#include "vulkan/vulkan.hpp"
#include <algorithm>
#include <string>

namespace SimpleEngine {
namespace Core {
ImageSampler::ImageSampler(const CreateInfo &createInfo,
                           const RenderContext *context) {
  m_MagFilter = createInfo.MagFilter;
  m_MinFilter = createInfo.MinFilter;

  m_WrapU = createInfo.WrapU;
  m_WrapV = createInfo.WrapV;
  m_WrapW = createInfo.WrapW;

  m_AnisotropyEnable = createInfo.AnisotropyEnable;
  m_CompareEnable = createInfo.CompareEnable;
  m_CompareOp = createInfo.CompareOp;

  m_Name = createInfo.Name;

  m_pContext = context;

  Create();
  GenerateHashedKey();
}

ImageSampler::~ImageSampler() {};
void ImageSampler::Free() {
  if (!m_IsValid) {
    return;
  }

  m_pContext->device.destroySampler(m_Sampler);
  m_IsValid = false;
}

bool ImageSampler::IsValid() const { return m_IsValid; }

void ImageSampler::Create() {
  vk::Filter vkMagFilter, vkMinFilter;
  if (m_MagFilter == Enums::Image::Filter::eLinear) {
    vkMagFilter = vk::Filter::eLinear;
  } else {
    vkMagFilter = vk::Filter::eNearest;
  }
  if (m_MinFilter == Enums::Image::Filter::eLinear) {
    vkMinFilter = vk::Filter::eLinear;
  } else {
    vkMinFilter = vk::Filter::eNearest;
  }

  vk::SamplerAddressMode vkUWrap, vkVWrap, vkWWrap;
  if (m_WrapU == Enums::Image::Wrap::eMirroredRepeat) {
    vkUWrap = vk::SamplerAddressMode::eMirroredRepeat;
  } else if (m_WrapU == Enums::Image::Wrap::eRepeat) {
    vkUWrap = vk::SamplerAddressMode::eRepeat;
  } else {
    vkUWrap = vk::SamplerAddressMode::eClampToEdge;
  }
  if (m_WrapV == Enums::Image::Wrap::eMirroredRepeat) {
    vkVWrap = vk::SamplerAddressMode::eMirroredRepeat;
  } else if (m_WrapV == Enums::Image::Wrap::eRepeat) {
    vkVWrap = vk::SamplerAddressMode::eRepeat;
  } else {
    vkVWrap = vk::SamplerAddressMode::eClampToEdge;
  }
  if (m_WrapW == Enums::Image::Wrap::eMirroredRepeat) {
    vkWWrap = vk::SamplerAddressMode::eMirroredRepeat;
  } else if (m_WrapW == Enums::Image::Wrap::eRepeat) {
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
                             .anisotropyEnable = m_AnisotropyEnable,
                             .maxAnisotropy = maxAnisotropy,
                             .compareEnable = m_CompareEnable,
                             .compareOp = m_CompareOp,
                             .minLod = 0.0f,
                             .maxLod = vk::LodClampNone,
                             .borderColor = vk::BorderColor::eIntOpaqueBlack,
                             .unnormalizedCoordinates = vk::False};

  m_Sampler = m_pContext->device.createSampler(info);
  m_IsValid = true;
}

const std::string &ImageSampler::GetName() const { return m_Name; }
} // namespace Core
} // namespace SimpleEngine
