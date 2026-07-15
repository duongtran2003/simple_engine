#include "ui/camera_ui.hpp"
#include "core/camera.hpp"
#include <glm/common.hpp>
#include <imgui.h>

namespace SimpleEngine {
namespace UI {
CameraUI::CameraUI(Core::Camera &camera) : camera(camera) {};

void CameraUI::render() {
  ImGui::SetNextWindowPos({0.0f, 0.0f}, ImGuiCond_Always);
  ImGui::SetNextWindowSize({300.0f, 0.0f}, ImGuiCond_Always);
  ImGui::Begin("Camera", nullptr, ImGuiWindowFlags_NoResize);

  float cameraFov = camera.getCamera()->getFov();

  if (ImGui::SliderFloat("FOV", &cameraFov, 1.0f, 150.0f)) {
    camera.getCamera()->setFov(cameraFov);
  }

  if (ImGui::Combo("Aspect ratio", &currentOption, aspectRatios,
                   IM_COUNTOF(aspectRatios))) {
    if (currentOption == 0) {
      camera.getCamera()->setAspectRatio(16.0f / 9.0f);
    } else if (currentOption == 1) {
      camera.getCamera()->setAspectRatio(16.0f / 10.0f);
    } else if (currentOption == 2) {
      camera.getCamera()->setAspectRatio(4.0f / 3.0f);
    } else if (currentOption == 3) {
      camera.getCamera()->setAspectRatio(21.0f / 9.0f);
    }
  }

  ImGui::End();
}
} // namespace UI
} // namespace SimpleEngine
