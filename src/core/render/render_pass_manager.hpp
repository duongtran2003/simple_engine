#pragma once

#include "core/render/render_pass.hpp"
#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace SimpleEngine {
namespace Core {
class RenderPassManager {
private:
  std::unordered_map<std::string, std::unique_ptr<RenderPass>> passes;
  std::vector<RenderPass *> sorted;

  bool isDirty = true;
  const RenderContext &context;

public:
  RenderPassManager(const RenderContext &rContext);
  ~RenderPassManager() = default;

  // Need to implement it here
  template <typename T, typename... Args>
  T *addRenderPass(const std::string &name, Args &&...args) {
    static_assert(std::is_base_of<RenderPass, T>::value,
                  "T must derive from RenderPass");

    auto renderPassIt = passes.find(name);
    if (renderPassIt != passes.end()) {
      return dynamic_cast<T *>(renderPassIt->second.get());
    }

    auto pass = std::make_unique<T>(name, context, std::forward<Args>(args)...);
    T *passPtr = pass.get();
    passes[name] = std::move(pass);
    isDirty = true;

    return passPtr;
  }

  RenderPass *getRenderPass(const std::string &name);
  void removeRenderPass(const std::string &name);

  void execute(vk::CommandBuffer &commandBuffer);

private:
  void sortPasses();
};
} // namespace Core
} // namespace SimpleEngine
