#pragma once

#include "enums/texture.hpp"
#include <cstdint>
#include <glm/ext/vector_float4.hpp>
#include <string>
#include <vector>
namespace SimpleEngine {
namespace Core {

struct PbrTextureProperty {
  glm::vec4 baseColorFactor;
};

struct RawTexture {
  std::string name;
  bool isValid;

  std::vector<unsigned char> pixels;
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t componentCount = 4;

  PbrTextureProperty pbrProperty;

  Enums::Texture::Filter magFilter = Enums::Texture::Filter::Linear;
  Enums::Texture::Filter minFilter = Enums::Texture::Filter::Linear;

  Enums::Texture::Wrap wrapS = Enums::Texture::Wrap::Repeat;
  Enums::Texture::Wrap wrapT = Enums::Texture::Wrap::Repeat;

  Enums::Texture::ColorSpace colorSpace;
};

} // namespace Core
} // namespace SimpleEngine
