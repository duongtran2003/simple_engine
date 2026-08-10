#include "ui/settings_ui.hpp"
#include "core/frame_pacer/frame_pacer.hpp"
#include "core/render_context.hpp"
#include <glm/common.hpp>
#include <glm/ext/vector_float3.hpp>
#include <imgui.h>
#include <imgui_internal.h>

namespace SimpleEngine {
namespace UI {
SettingsUI::SettingsUI(Core::FramePacer &framePacer,
                       const Core::RenderContext &context)
    : framePacer(framePacer), context(context) {};

void SettingsUI::render() {
  float startY = 10.0f;

  ImGuiWindow *profilerWindow = ImGui::FindWindowByName("Camera");
  if (profilerWindow && profilerWindow->Active) {
    startY = profilerWindow->Pos.y + profilerWindow->Size.y;
  }

  ImGui::SetNextWindowPos({0.0f, startY}, ImGuiCond_Always);
  ImGui::SetNextWindowSize({0.0f, 0.0f}, ImGuiCond_Always);
  ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoResize);

  if (ImGui::Combo("FPS Limiter", &currentOption, fpsLimiter,
                   IM_COUNTOF(fpsLimiter))) {
    switch (currentOption) {
    case 0:
      framePacer.setTargetFPS(60);
      break;
    case 1:
      framePacer.setTargetFPS(72);
      break;
    case 2:
      framePacer.setTargetFPS(90);
      break;
    case 3:
      framePacer.setTargetFPS(120);
      break;
    case 4:
      framePacer.setTargetFPS(144);
      break;
    case 5:
      framePacer.setTargetFPS(0);
      break;
    default:
      framePacer.setTargetFPS(0);
    }
  }

  ImGui::End();
}
} // namespace UI
} // namespace SimpleEngine
