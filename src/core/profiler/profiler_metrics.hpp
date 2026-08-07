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
  float fpsMin = 0.0f;
  float fpsMax = 0.0f;
  std::vector<float> fpsHistory = {};

  float frametime = 0.0f;
  std::vector<float> frametimeHistory = {};

  float totalVram = 0.0f;
  float usedVram = 0.0f;
  std::vector<float> vramUsageHistory = {};

  size_t historyOffset = 0;
};
} // namespace Core
} // namespace SimpleEngine
