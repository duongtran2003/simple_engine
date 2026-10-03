#pragma once

#include "core/image/image_sampler.hpp"
#include "core/render_context.hpp"
#include <memory>
#include <string>
#include <unordered_map>

namespace SimpleEngine {
namespace Core {
class SamplerRegistry {
public:
  SamplerRegistry() = delete;
  SamplerRegistry(const RenderContext *context);
  ~SamplerRegistry();
  const ImageSampler *Retrieve(const ImageSampler::CreateInfo &retrieveInfo);

private:
  const RenderContext *m_pContext;
  std::unordered_map<std::string, std::unique_ptr<ImageSampler>> m_Samplers;

  const std::string
  GenerateHashedKey(const ImageSampler::CreateInfo &retrieveInfo) const;
};
} // namespace Core
} // namespace SimpleEngine
