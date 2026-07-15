#pragma once

#include "core/camera.hpp"
namespace SimpleEngine {
namespace UI {
class CameraUI {
private:
  Core::Camera &camera;

  const char *aspectRatios[4] = {"16:9", "16:10", "4:3", "21:9"};
  int currentOption = 0;

public:
  CameraUI() = delete;
  CameraUI(Core::Camera &camera);

  void render();
};
} // namespace UI
} // namespace SimpleEngine
