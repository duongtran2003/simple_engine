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
    std::string name;

    Enums::Image::Filter magFilter = Enums::Image::Filter::eLinear;
    Enums::Image::Filter minFilter = Enums::Image::Filter::eLinear;
    Enums::Image::Wrap wrapU = Enums::Image::Wrap::eClampToEdge;
    Enums::Image::Wrap wrapV = Enums::Image::Wrap::eClampToEdge;
    Enums::Image::Wrap wrapW = Enums::Image::Wrap::eClampToEdge;
    uint32_t anisotropyEnable = vk::True;
    uint32_t compareEnable = vk::False;
    vk::CompareOp compareOp = vk::CompareOp::eAlways;
  };

  ImageSampler() = delete;
  ImageSampler(const CreateInfo &createInfo, const RenderContext *context);
  ~ImageSampler();
  void Free();
  bool IsValid() const;
  const std::string &GetName() const;

private:
  const RenderContext *m_pContext;
  bool m_isValid = false;

  std::string m_name;

  Enums::Image::Filter m_magFilter;
  Enums::Image::Filter m_minFilter;
  Enums::Image::Wrap m_wrapU;
  Enums::Image::Wrap m_wrapV;
  Enums::Image::Wrap m_wrapW;
  uint32_t m_anisotropyEnable;
  uint32_t m_compareEnable;
  vk::CompareOp m_compareOp;

  vk::Sampler m_sampler;

  void Create();
};
} // namespace Core
} // namespace SimpleEngine
