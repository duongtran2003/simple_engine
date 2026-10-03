#pragma once

#include "core/render_context.hpp"
#include "enums/image_enums.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <string>
#include <vulkan/vulkan.hpp>

namespace SimpleEngine {
namespace Core {
class ImageSampler {
public:
  struct CreateInfo {
    Enums::Image::Filter MagFilter = Enums::Image::Filter::eLinear;
    Enums::Image::Filter MinFilter = Enums::Image::Filter::eLinear;
    Enums::Image::Wrap WrapU = Enums::Image::Wrap::eClampToEdge;
    Enums::Image::Wrap WrapV = Enums::Image::Wrap::eClampToEdge;
    Enums::Image::Wrap WrapW = Enums::Image::Wrap::eClampToEdge;
    uint32_t AnisotropyEnable = vk::True;
    uint32_t CompareEnable = vk::False;
    vk::CompareOp CompareOp = vk::CompareOp::eAlways;
  };

  ImageSampler() = delete;
  ImageSampler(const CreateInfo &createInfo, const std::string &hashedKey,
               const RenderContext *context);
  ~ImageSampler();
  void Free();
  bool IsValid() const;
  const std::string &GetHashedKey() const;

  vk::Sampler GetSampler() const;

private:
  const RenderContext *m_pContext;
  bool m_IsValid = false;

  std::string m_HashedKey;

  Enums::Image::Filter m_MagFilter;
  Enums::Image::Filter m_MinFilter;
  Enums::Image::Wrap m_WrapU;
  Enums::Image::Wrap m_WrapV;
  Enums::Image::Wrap m_WrapW;
  uint32_t m_AnisotropyEnable;
  uint32_t m_CompareEnable;
  vk::CompareOp m_CompareOp;

  vk::Sampler m_Sampler;

  void Create();
};
} // namespace Core
} // namespace SimpleEngine
