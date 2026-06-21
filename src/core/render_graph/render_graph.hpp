#pragma once

#include "core/render_context.hpp"
#include "core/render_graph/render_pass.hpp"
#include "core/render_graph/graph_resource.hpp"
#include "vulkan/vulkan.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace SimpleEngine {
namespace Core {
class RenderGraph {
private:
  std::unordered_map<std::string, GraphResource *> resources;
  std::unordered_map<std::string, RenderPass *> passes;

  std::vector<std::string> executionOrder;
  std::string outputResource;

  const RenderContext &context;

public:
  RenderGraph() = delete;
  RenderGraph(const RenderContext &context);
  ~RenderGraph();

  void setOutputResource(const std::string &resourceName);
  GraphResource *getOutputResource();
  GraphResource *getResource(const std::string &resourceName);

  void addResource(GraphResource *resource);
  void removeResource(const std::string &resourceName);

  void addPass(RenderPass *pass);
  void removePass(const std::string &passName);

  void compile();
  void sortPasses();
  void execute(vk::CommandBuffer &commandBuffer);
};
} // namespace Core
} // namespace SimpleEngine
