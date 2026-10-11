#pragma once

namespace SimpleEngine {
namespace Enums {
namespace Image {
enum class Filter { eNearest, eLinear };
enum class Wrap { eRepeat, eClampToEdge, eMirroredRepeat };
enum class ColorSpace { eLinear, eNonLinear };
enum class MipmapMode { eNearest, eLinear };
}; // namespace Image
} // namespace Enums
} // namespace SimpleEngine
