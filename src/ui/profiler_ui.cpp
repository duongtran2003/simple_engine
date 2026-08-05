#include "ui/profiler_ui.hpp"
#include "core/profiler/profiler.hpp"
#include "core/profiler/profiler_metrics.hpp"
#include <imgui.h>
namespace SimpleEngine {
namespace UI {
ProfilerUI::ProfilerUI(Core::Profiler &profiler) : profiler(profiler) {};

void ProfilerUI::render() {
  Core::ProfilerMetrics metricsSnapshot = profiler.getSnapshot();

  ImGui::SetNextWindowPos({0.0f, 0.0f}, ImGuiCond_Always);
  ImGui::SetNextWindowSize({240.0f, 0.0f}, ImGuiCond_Always);
  ImGui::Begin("Profiler", nullptr, ImGuiWindowFlags_NoResize);

  ImGui::Text("GPU: %s", metricsSnapshot.gpuName.c_str());
  ImGui::Text("FPS: %6.2f", metricsSnapshot.fps);
  ImGui::Text("Frametime: %6.4f", metricsSnapshot.frametime);

  ImGui::End();
}
} // namespace UI
} // namespace SimpleEngine
