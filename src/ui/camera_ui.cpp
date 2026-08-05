#include "ui/camera_ui.hpp"
#include "core/camera.hpp"
#include <glm/common.hpp>
#include <glm/ext/vector_float3.hpp>
#include <imgui.h>

namespace SimpleEngine {
namespace UI {
CameraUI::CameraUI(Core::Camera &camera) : camera(camera) {};

void CameraUI::render() {
  ImGui::SetNextWindowPos({0.0f, 0.0f}, ImGuiCond_Always);
  ImGui::SetNextWindowSize({0.0f, 0.0f}, ImGuiCond_Always);
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

  glm::vec3 pos = camera.getTransform()->getPosition();
  ImGui::Text("Position: ");

  ImGui::SameLine(90.0f);
  ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "X:");
  ImGui::SameLine(110.0f);
  ImGui::Text("%6.2f", pos.x);

  ImGui::SameLine(180.0f);
  ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Y:");
  ImGui::SameLine(200.0f);
  ImGui::Text("%6.2f", pos.y);

  ImGui::SameLine(270.0f);
  ImGui::TextColored(ImVec4(0.3f, 0.6f, 1.0f, 1.0f), "Z:");
  ImGui::SameLine(290.0f);
  ImGui::Text("%6.2f", pos.z);

  glm::vec3 direction = camera.getForward();
  ImGui::Text("Direction: ");

  ImGui::SameLine(90.0f);
  ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "X:");
  ImGui::SameLine(110.0f);
  ImGui::Text("%6.2f", direction.x);

  ImGui::SameLine(180.0f);
  ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Y:");
  ImGui::SameLine(200.0f);
  ImGui::Text("%6.2f", direction.y);

  ImGui::SameLine(270.0f);
  ImGui::TextColored(ImVec4(0.3f, 0.6f, 1.0f, 1.0f), "Z:");
  ImGui::SameLine(290.0f);
  ImGui::Text("%6.2f", direction.z);

  ImGui::End();
}
} // namespace UI
} // namespace SimpleEngine
