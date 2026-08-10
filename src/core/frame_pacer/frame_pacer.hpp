#pragma once

#include <chrono>
#include <cstdint>
#include <ratio>

namespace SimpleEngine {
namespace Core {

class FramePacer {
private:
  using Clock = std::chrono::high_resolution_clock;
  using Duration = std::chrono::duration<double, std::milli>;

  int64_t targetFPS{60};
  Clock::time_point frameStartTime;
  Duration targetFrametime{16.666666};
  Duration lastFrametime{16.666666};

  bool isUncapped() const;

public:
  FramePacer();
  explicit FramePacer(int64_t targetFPS);

  void startFrame();
  void endFrame();

  FramePacer *setTargetFPS(int64_t target);
  int64_t getTargetFPS() const;

  double getFrametime() const;
};

} // namespace Core
} // namespace SimpleEngine
