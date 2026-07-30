#pragma once

#include "core/render_context.hpp"
#include "core/resource/resource.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <string>

namespace SimpleEngine {
namespace Core {
class Cubemap : public Resource {
private:
  enum class Source { FromFile };
  Source source;

  std::string path;

  vk::Image image;
  vk::DeviceMemory memory;
  vk::DeviceSize offset;
  vk::ImageView imageView;
  vk::Sampler sampler;
  vk::Format format;

  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t channels = 0;

  void readFromFile();

public:
  Cubemap(const std::string &id, const RenderContext &renderContext);
  Cubemap(const std::string &id, const RenderContext &renderContext,
          const std::string &path);

  vk::Image getImage() const;
  vk::Sampler getSampler() const;
  vk::ImageView getImageView() const;

protected:
  bool doLoad() override;
  void doUnload() override;
};
} // namespace Core
} // namespace SimpleEngine
