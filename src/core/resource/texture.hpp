#pragma once

#include "core/raw_texture.hpp"
#include "core/render_context.hpp"
#include "core/resource/resource.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <string>

namespace SimpleEngine {
namespace Core {
class Texture : public Resource {
private:
  enum class Source { fromFile, fromRawTexture };
  Source source;

  std::string *path;
  RawTexture rawTexture;

  vk::Image image;
  vk::DeviceMemory memory;
  vk::DeviceSize offset;
  vk::ImageView imageView;
  vk::Sampler sampler;

  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t channels = 0;

  void readFromRawTexture();

public:
  Texture(const std::string &id, const RenderContext &renderContext);
  Texture(const std::string &id, const RenderContext &renderContext,
          RawTexture raw);

  vk::Image getImage() const;
  vk::Sampler getSampler() const;
  vk::ImageView getImageView() const;

protected:
  bool doLoad() override;
  void doUnload() override;
};
} // namespace Core
} // namespace SimpleEngine
