#pragma once

#include "core/entity/entity.hpp"
#include <cstddef>
#include <string>

namespace SimpleEngine {
namespace Core {
class SceneObject : public Entity {
private:
  size_t ssboIndex = 0;

public:
  SceneObject(const std::string &name, size_t ssboIndex);
};
} // namespace Core
} // namespace SimpleEngine
