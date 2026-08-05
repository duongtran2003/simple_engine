#pragma once

#include "core/profiler/profiler_metrics.hpp"
#include "core/render_context.hpp"
#include <atomic>
#include <mutex>
#include <thread>
namespace SimpleEngine {
namespace Core {
class Profiler {
public:
  Profiler() = delete;
  Profiler(const RenderContext &context);
  ~Profiler();

  ProfilerMetrics getSnapshot();

private:
  const RenderContext &context;
  ProfilerMetrics metrics;
  std::mutex metricsMutex;
  std::thread worker;
  std::atomic<bool> isRunning;

  void backgroundWorkerLoop();
  void queryMetrics();
};
} // namespace Core
} // namespace SimpleEngine
