#include "core/profiler/profiler.hpp"
#include "core/profiler/profiler_metrics.hpp"
#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <chrono>
#include <mutex>
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
  metrics.frametime = context.getDeltaTime();
  metrics.fps = 1.0f / metrics.frametime;
  metrics.fpsHistory[metrics.historyOffset] = metrics.fps;
  metrics.frametimeHistory[metrics.historyOffset] = metrics.frametime;
  metrics.historyOffset = (metrics.historyOffset + 1) % METRICS_HISTORY_SIZE;
}
} // namespace Core
} // namespace SimpleEngine
