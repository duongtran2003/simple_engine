#include "ui/camera_ui.hpp"
#include "core/camera.hpp"
#include <glm/common.hpp>
#include <imgui.h>

namespace SimpleEngine {
namespace UI {
CameraUI::CameraUI(Core::Camera &camera) : camera(camera) {};

void CameraUI::render() {
  ImGui::SetNextWindowPos({0.0f, 0.0f}, ImGuiCond_Always);
  ImGui::SetNextWindowSize({300.0f, 200.0f}, ImGuiCond_Always);
  ImGui::Begin("Camera", nullptr, ImGuiWindowFlags_NoResize);

  float cameraFov = camera.getCamera()->getFov();

  if (ImGui::SliderFloat("FOV", &cameraFov, 1.0f, 150.0f)) {
    camera.getCamera()->setFov(cameraFov);
  }

  ImGui::End();
}
} // namespace UI
} // namespace SimpleEngine
