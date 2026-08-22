#pragma once

#include "core/frame_pacer/frame_pacer.hpp"
#include "core/render_context.hpp"
namespace SimpleEngine {
namespace UI {
class SettingsUI {
private:
  Core::FramePacer &framePacer;
  const Core::RenderContext &context;

  const char *fpsLimiter[6] = {"60", "72", "90", "120", "144", "Uncapped"};
  int currentOption = 0;

public:
  SettingsUI() = delete;
  SettingsUI(Core::FramePacer &framePacer, const Core::RenderContext &context);

  void render();
};
} // namespace UI
} // namespace SimpleEngine
