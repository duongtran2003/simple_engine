#include "core/profiler/profiler.hpp"
#include "core/profiler/profiler_metrics.hpp"
#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>

namespace SimpleEngine {
namespace Core {
Profiler::Profiler(const RenderContext &context) : context(context) {
  vk::PhysicalDeviceProperties props = context.physicalDevice.getProperties();
  metrics.gpuName = std::string(props.deviceName.data());
  isRunning = true;
  worker = std::thread(&Profiler::backgroundWorkerLoop, this);
};

Profiler::~Profiler() {
  isRunning = false;
  if (worker.joinable()) {
    worker.join();
  }
}

ProfilerMetrics Profiler::getSnapshot() {
  std::lock_guard<std::mutex> lock(metricsMutex);
  return metrics;
}

void Profiler::backgroundWorkerLoop() {
  while (isRunning) {
    queryMetrics();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

void Profiler::queryMetrics() {
  std::lock_guard<std::mutex> lock(metricsMutex);
  float deltaTime = context.getDeltaTime();
  if (deltaTime == 0.0f) {
    return;
  }
  metrics.frametime = deltaTime;
  metrics.fps = 1.0f / metrics.frametime;
  metrics.fpsMin = std::min(metrics.fpsMin, metrics.fps);
  metrics.fpsMax = std::max(metrics.fpsMax, metrics.fps);

  try {
    vk::PhysicalDeviceMemoryBudgetPropertiesEXT memoryBudgetInfo{};
    vk::PhysicalDeviceMemoryProperties2 memoryBudgetProps2{
        .pNext = &memoryBudgetInfo};

    context.physicalDevice.getMemoryProperties2(&memoryBudgetProps2);
    uint64_t usageBytes = memoryBudgetInfo.heapUsage[0];
    uint64_t budgetBytes = memoryBudgetInfo.heapBudget[0];

    metrics.totalVram = budgetBytes / (1024.0f * 1024.0f);
    metrics.usedVram = usageBytes / (1024.0f * 1024.0f);
  } catch (...) {
    throw std::runtime_error(
        "Profiler::queryMetrics::ERROR: MEMORY_BUDGET_EXT not enabled");
  }

  if (metrics.historyOffset >= metrics.fpsHistory.size()) {
    metrics.fpsHistory.push_back(metrics.fps);
    metrics.frametimeHistory.push_back(metrics.frametime);
    metrics.vramUsageHistory.push_back(metrics.usedVram);
  } else {
    metrics.fpsHistory[metrics.historyOffset] = metrics.fps;
    metrics.frametimeHistory[metrics.historyOffset] = metrics.frametime;
    metrics.vramUsageHistory[metrics.historyOffset] = metrics.usedVram;
  }
  metrics.historyOffset = (metrics.historyOffset + 1) % METRICS_HISTORY_SIZE;
}
} // namespace Core
} // namespace SimpleEngine
