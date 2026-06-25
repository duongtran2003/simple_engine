#pragma once

#include <cstdint>
#include <vector>
namespace SimpleEngine {
namespace Core {

enum class TextureFilter { Nearest, Linear };
enum class TextureWrapMode { Repeat, ClampToEdge, MirroredRepeat };

struct RawTexture {
  std::vector<unsigned char> pixels;
  uint32_t width = 0;
  uint32_t height = 0;
  int componentCount = 4;

  TextureFilter magFilter = TextureFilter::Linear;
  TextureFilter minFilter = TextureFilter::Linear;

  TextureWrapMode wrapS = TextureWrapMode::Repeat;
  TextureWrapMode wrapT = TextureWrapMode::Repeat;
};

} // namespace Core
} // namespace SimpleEngine
