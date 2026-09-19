#pragma once

#include <cstdint>

namespace SimpleEngine {
namespace Core {
struct ImageHandle {
  uint32_t id = 0;
  uint32_t generation = 0;

  bool IsValid() const { return id != 0 && generation != 0; }
};
} // namespace Core
} // namespace SimpleEngine
