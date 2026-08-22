#include "core/frame_pacer/frame_pacer.hpp"
#include <chrono>
#include <cstdint>
#include <thread>

namespace SimpleEngine {
namespace Core {

FramePacer::FramePacer() : FramePacer(60) {}

FramePacer::FramePacer(int64_t target) { setTargetFPS(target); }

FramePacer *FramePacer::setTargetFPS(int64_t target) {
  targetFPS = target;
  if (!isUncapped()) {
    targetFrametime = Duration(1000.0 / static_cast<double>(targetFPS));
  }
  return this;
}

int64_t FramePacer::getTargetFPS() const { return targetFPS; }

bool FramePacer::isUncapped() const { return targetFPS <= 0; }

void FramePacer::startFrame() { frameStartTime = Clock::now(); }

void FramePacer::endFrame() {
  auto now = Clock::now();
  Duration elapsed = now - frameStartTime;

  if (!isUncapped()) {
    while (elapsed < targetFrametime) {
      Duration remaining = targetFrametime - elapsed;

      if (remaining.count() > 1.5) {
        std::this_thread::sleep_for(std::chrono::microseconds(500));
      }

      elapsed = Clock::now() - frameStartTime;
    }
  }

  lastFrametime = Clock::now() - frameStartTime;
}

double FramePacer::getFrametime() const { return lastFrametime.count(); }

} // namespace Core
} // namespace SimpleEngine
