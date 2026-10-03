#pragma once

#include <cstdint>

namespace SimpleEngine {
namespace Core {
struct ImageHandle {
  uint32_t Id = 0;
  uint32_t Generation = 0;

  bool IsValid() const { return Id != 0 && Generation != 0; }
};
} // namespace Core
} // namespace SimpleEngine
