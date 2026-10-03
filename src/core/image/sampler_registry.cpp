#include "core/image/sampler_registry.hpp"
#include "core/image/image_sampler.hpp"
#include "core/render_context.hpp"
#include "enums/image_enums.hpp"
#include "vulkan/vulkan.hpp"
#include <memory>
#include <string>
#include <utility>

namespace SimpleEngine {
namespace Core {
SamplerRegistry::SamplerRegistry(const RenderContext *context) {
  m_pContext = context;
}
SamplerRegistry::~SamplerRegistry() {
  // TODO: Destructor
}

const ImageSampler *
SamplerRegistry::Retrieve(const ImageSampler::CreateInfo &retrieveInfo) {
  const std::string hashed = GenerateHashedKey(retrieveInfo);
  auto it = m_Samplers.find(hashed);
  if (it != m_Samplers.end()) {
    return it->second.get();
  }

  auto sampler =
      std::make_unique<ImageSampler>(retrieveInfo, hashed, m_pContext);
  m_Samplers[hashed] = std::move(sampler);

  const ImageSampler *res = sampler.get();
  return res;
}

const std::string SamplerRegistry::GenerateHashedKey(
    const ImageSampler::CreateInfo &retrieveInfo) const {
  std::string hashed = "";
  if (retrieveInfo.MagFilter == Enums::Image::Filter::eLinear) {
    hashed += "MAG_LINEAR_";
  } else {
    hashed += "MAG_NEAREST_";
  }
  if (retrieveInfo.MinFilter == Enums::Image::Filter::eLinear) {
    hashed += "MIN_LINEAR_";
  } else {
    hashed += "MIN_NEAREST_";
  }

  if (retrieveInfo.WrapU == Enums::Image::Wrap::eMirroredRepeat) {
    hashed += "U_MIRROREDREPEAT_";
  } else if (retrieveInfo.WrapU == Enums::Image::Wrap::eRepeat) {
    hashed += "U_REPEAT_";
  } else {
    hashed += "U_CLAMP_";
  }
  if (retrieveInfo.WrapV == Enums::Image::Wrap::eMirroredRepeat) {
    hashed += "V_MIRROREDREPEAT_";
  } else if (retrieveInfo.WrapV == Enums::Image::Wrap::eRepeat) {
    hashed += "V_REPEAT_";
  } else {
    hashed += "V_CLAMP_";
  }
  if (retrieveInfo.WrapW == Enums::Image::Wrap::eMirroredRepeat) {
    hashed += "W_MIRROREDREPEAT_";
  } else if (retrieveInfo.WrapW == Enums::Image::Wrap::eRepeat) {
    hashed += "W_REPEAT_";
  } else {
    hashed += "W_CLAMP_";
  }

  if (retrieveInfo.AnisotropyEnable) {
    hashed += "ANISOTROPY_";
  } else {
    hashed += "NOANISOTROPY_";
  }

  if (retrieveInfo.CompareEnable) {
    hashed += "COMPARE_";
  } else {
    hashed += "NOCOMPARE_";
  }

  if (retrieveInfo.CompareOp == vk::CompareOp::eAlways) {
    hashed += "COMPAREALWAYS_";
  } else if (retrieveInfo.CompareOp == vk::CompareOp::eEqual) {
    hashed += "COMPAREEQUAL_";
  } else if (retrieveInfo.CompareOp == vk::CompareOp::eGreaterOrEqual) {
    hashed += "COMPAREGE_";
  } else if (retrieveInfo.CompareOp == vk::CompareOp::eLessOrEqual) {
    hashed += "COMPARELE_";
  } else if (retrieveInfo.CompareOp == vk::CompareOp::eLess) {
    hashed += "COMPARELESS_";
  } else if (retrieveInfo.CompareOp == vk::CompareOp::eNever) {
    hashed += "COMPARENEVER_";
  } else if (retrieveInfo.CompareOp == vk::CompareOp::eNotEqual) {
    hashed += "COMPARENOTEQUAL_";
  } else if (retrieveInfo.CompareOp == vk::CompareOp::eGreater) {
    hashed += "COMPAREGREATER_";
  }

  return hashed;
}
} // namespace Core
} // namespace SimpleEngine
