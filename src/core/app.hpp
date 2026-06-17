#pragma once

#include "core/render_context.hpp"
#include "core/render_graph.hpp"
#include <cstdint>

namespace SimpleEngine {
namespace Core {
class App {
private:
  RenderContext renderContext;
  RenderGraph *renderGraph = nullptr;

  void setupDeferredRenderer(uint32_t width, uint32_t height);
  void initRenderContext();

public:
  App();
  void run();
};
} // namespace Core
} // namespace SimpleEngine
