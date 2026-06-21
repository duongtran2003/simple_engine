#pragma once

#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <functional>
#include <string>
#include <unordered_set>

namespace SimpleEngine {
namespace Core {
class RenderGraph;

class RenderPass {
private:
  const RenderContext &context;

  std::string name;
  bool active;
  std::unordered_set<std::string> inputs;
  std::unordered_set<std::string> outputs;

  std::function<void(vk::CommandBuffer &commandBuffer)> executeCallback;

public:
  RenderPass() = delete;
  RenderPass(const std::string &name, const RenderContext &context);

  void addInput(const std::string &resourceName);
  void addOutput(const std::string &resourceName);

  void deleteInput(const std::string &resourceName);
  void deleteOutput(const std::string &resourceName);

  RenderPass *setIsActive(bool state);
  RenderPass *setExecuteCallbackFn(
      std::function<void(vk::CommandBuffer &commandBuffer)> fn);

  const std::string &getName() const;

  const std::unordered_set<std::string> &getInputs() const;
  const std::unordered_set<std::string> &getOutputs() const;

  bool getIsActive() const;

  void execute(vk::CommandBuffer &commandBuffer);
};
} // namespace Core
} // namespace SimpleEngine
