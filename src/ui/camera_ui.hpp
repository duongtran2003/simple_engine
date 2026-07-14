#pragma once

#include "core/camera.hpp"
namespace SimpleEngine {
namespace UI {
class CameraUI {
private:
  Core::Camera &camera;

public:
  CameraUI() = delete;
  CameraUI(Core::Camera &camera);

  void render();
};
} // namespace UI
} // namespace SimpleEngine
