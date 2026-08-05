#pragma once

#include "core/profiler/profiler.hpp"
namespace SimpleEngine {
namespace UI {
class ProfilerUI {
private:
  Core::Profiler &profiler;

public:
  ProfilerUI() = delete;
  ProfilerUI(Core::Profiler &profiler);

  void render();
};
} // namespace UI
} // namespace SimpleEngine
