#include "core/resource/texture.hpp"
#include "core/resource/resource.hpp"
#include "vulkan/vulkan.hpp"
#include <stb_image.h>
#include <string>
#include <vulkan/vulkan_core.h>

namespace SimpleEngine {
namespace Core {
Texture::Texture(const std::string &id) : Resource(id) {}

Texture::~Texture() {
  // TODO: Proper texture unload
  unload();
}

unsigned char *Texture::loadImageData(const std::string &path, int &_width,
                                      int &_height, int &_channels) {
  unsigned char *data =
      stbi_load(path.c_str(), &_width, &_height, &_channels, STBI_rgb_alpha);

  return data;
}

void Texture::unloadImageData(unsigned char *data) {
  if (data) {
    stbi_image_free(data);
  }
}

bool Texture::doLoad() {
  std::string filePath = "textures/" + getId() + ".ktx";
  unsigned char *data = loadImageData(filePath, width, height, channels);
  if (!data) {
    return false;
  }
  offset = width * height * 4;

  createVulkanImage(data, width, height, channels);

  unloadImageData(data);

  return true;
}

void Texture::doUnload() {
  // TODO: Implement later
}

void Texture::createVulkanImage(unsigned char *data, int &width, int &height,
                                int &channels) {
  // TODO: Implement later
}
} // namespace Core
} // namespace SimpleEngine
