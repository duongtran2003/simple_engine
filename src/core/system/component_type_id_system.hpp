#pragma once

#include <cstddef>

namespace SimpleEngine {
namespace Core {
class ComponentTypeIdSystem {
private:
  static size_t nextTypeId;

public:
  template <typename T> static size_t getId() {
    static size_t typeId = nextTypeId++;
    return typeId;
  }
};
} // namespace Core
} // namespace SimpleEngine
