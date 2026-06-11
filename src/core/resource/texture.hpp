#pragma once

#include "core/resource/resource.hpp"
#include "vulkan/vulkan.hpp"
#include <string>

namespace SimpleEngine {
namespace Core {
class Texture : public Resource {
private:
  vk::Image image;
  vk::DeviceMemory memory;
  vk::DeviceSize offset;
  vk::ImageView imageView;
  vk::Sampler sampler;

  int width = 0;
  int height = 0;
  int channels = 0;

  unsigned char *loadImageData(const std::string &path, int &width, int &height,
                               int &channels);
  void unloadImageData(unsigned char *data);
  void createVulkanImage(unsigned char *data, int &width, int &height,
                         int &channels);

public:
  Texture(const std::string &id);
  ~Texture() override;

protected:
  bool doLoad() override;
  void doUnload() override;
};
} // namespace Core
} // namespace SimpleEngine
