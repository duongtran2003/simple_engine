#pragma once

#include "core/render/render_pass.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace SimpleEngine {
namespace Core {
class RenderPassManager {
private:
  std::unordered_map<std::string, std::unique_ptr<RenderPass>> passes;
  std::vector<RenderPass *> sorted;

  bool isDirty = true;

public:
  template <typename T, typename... Args>
  T *addRenderPass(const std::string &name, Args &&...args) {}

  RenderPass* getRenderPass(const std::string& name);
};
} // namespace Core
} // namespace SimpleEngine
