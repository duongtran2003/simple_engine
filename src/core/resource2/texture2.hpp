#pragma once

#include "core/image/image_handle.hpp"
#include "core/image/image_sampler.hpp"
#include <cstdint>
#include <string>

namespace SimpleEngine {
namespace Core {
class Texture2 {
public:
  Texture2() = default;
  Texture2(const std::string &path, const ImageHandle& imageHandle);
  ~Texture2();

  const std::string &GetPath() const;
  ImageHandle GetImageHandle() const;
  const ImageSampler *GetSampler() const;
  uint32_t GetBindlessIndex() const;

  void SetSampler(ImageSampler *sampler);
  void SetBindlessIndex(uint32_t index);

private:
  std::string m_Path = "";
  ImageHandle m_ImageHandle;
  const ImageSampler *m_Sampler = nullptr;
  uint32_t m_BindlessIndex = 0;
};
} // namespace Core
} // namespace SimpleEngine
