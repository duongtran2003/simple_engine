#pragma once

#include "core/entity/entity.hpp"
#include "core/input/input.hpp"

namespace SimpleEngine {
namespace Core {

class Camera : public Entity {
public:
  Camera() = delete;
  virtual ~Camera() = default;

  Camera(const Input &input);
  void update(float delta) override;

private:
  const Input &input;

  void handleInput(float delta);
};
} // namespace Core
} // namespace SimpleEngine
