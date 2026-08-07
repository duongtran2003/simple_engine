#include "ui/profiler_ui.hpp"
#include "core/profiler/profiler.hpp"
#include "core/profiler/profiler_metrics.hpp"
#include <cstddef>
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

  float fpsGraphMin = 0.0f;
  float fpsGraphMax = metricsSnapshot.fpsMax + 20.0f;
  ImGui::Text("FPS: %6.2f", metricsSnapshot.fps);
  ImGui::PlotLines("##FPS_GRAPH", metricsSnapshot.fpsHistory.data(),
                   metricsSnapshot.fpsHistory.size(), 0, NULL, fpsGraphMin,
                   fpsGraphMax, ImVec2(0.0f, 80.0f));
  ImGui::Text("Frametime: %6.4f", metricsSnapshot.frametime);

  float vramUsageGraphMin = 0.0f;
  float vramUsageGraphMax = metricsSnapshot.totalVram;
  ImGui::Text("VRAM Usage: %6.2f/%6.2f MB", metricsSnapshot.usedVram,
              metricsSnapshot.totalVram);
  ImGui::PlotLines("##VRAM_USAGE_GRAPH",
                   metricsSnapshot.vramUsageHistory.data(),
                   metricsSnapshot.vramUsageHistory.size(), 0, NULL,
                   vramUsageGraphMin, vramUsageGraphMax, ImVec2(0.0f, 80.0f));

  ImGui::End();
}
} // namespace UI
} // namespace SimpleEngine
