#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Core {
constexpr size_t METRICS_HISTORY_SIZE = 100;

struct ProfilerMetrics {
  std::string gpuName = "Unknown";
  float fps = 0.0f;
  float frametime = 0.0f;

  std::vector<float> fpsHistory = std::vector<float>(METRICS_HISTORY_SIZE);
  std::vector<float> frametimeHistory =
      std::vector<float>(METRICS_HISTORY_SIZE);

  size_t historyOffset = 0;
};
} // namespace Core
} // namespace SimpleEngine
