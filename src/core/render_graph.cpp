#include "core/render_graph.hpp"
#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace SimpleEngine {
namespace Core {
RenderGraph::RenderGraph(const RenderContext &renderContext)
    : renderContext(renderContext) {};

void RenderGraph::addResource(const std::string &name, vk::Format format,
                              vk::Extent2D extent, vk::ImageUsageFlags usage,
                              vk::ImageLayout initialLayout,
                              vk::ImageLayout finalLayout) {
  Resource resource{.name = name,
                    .format = format,
                    .extent = extent,
                    .usage = usage,
                    .initialLayout = initialLayout,
                    .finalLayout = finalLayout};

  resources[name] = resource;
}

void RenderGraph::addPass(
    const std::string &name, const std::vector<std::string> &inputs,
    const std::vector<std::string> &outputs,
    std::function<void(vk::CommandBuffer &)> executeFunction) {
  Pass pass{.name = name,
            .inputs = inputs,
            .outputs = outputs,
            .executeFunction = executeFunction};

  passes.push_back(pass);
}

void RenderGraph::resolveDependencies(
    std::vector<std::vector<size_t>> &dependencies,
    std::vector<std::vector<size_t>> &dependents) {

  std::unordered_map<std::string, size_t>
      resourceWriters; // The key represents resource name, value is the index
                       // of the pass that output the specific resource

  for (size_t i = 0; i < passes.size(); i++) {
    const auto &pass = passes[i];
    for (const auto &input : pass.inputs) {
      auto it = resourceWriters.find(input);
      if (it != resourceWriters.end()) {
        dependencies[i].push_back(it->second);
        dependents[it->second].push_back(i);
      }
    }

    for (const auto &output : pass.outputs) {
      resourceWriters[output] = i;
    }
  }
}

void RenderGraph::resolveExecutionOrder(
    std::vector<std::vector<size_t>> &dependencies,
    std::vector<std::vector<size_t>> &dependents) {
  // Topology sort
  std::queue<size_t> queue;
  std::vector<bool> visited(passes.size(), false);
  for (size_t i = 0; i < passes.size(); i++) {
    if (dependencies[i].size() == 0) {
      queue.push(i);
    }
  }

  if (queue.empty()) {
    throw std::runtime_error(
        "RenderGraph::compile()::ERROR: Cycle detected. Aborting.");
  }

  while (!queue.empty()) {
    size_t currentNode = queue.front();
    visited[currentNode] = true;
    queue.pop();
    executionOrder.push_back(currentNode);

    for (const auto nextNode : dependents[currentNode]) {
      std::erase_if(dependencies[nextNode],
                    [&](size_t node) { return node == currentNode; });
      if (visited[nextNode] || dependencies[nextNode].size()) {
        continue;
      }

      queue.push(nextNode);
    }
  }

  bool isAllVisited =
      std::none_of(visited.begin(), visited.end(), [&](size_t isNodeVisited) {
        return isNodeVisited == false;
      });
  if (!isAllVisited) {
    throw std::runtime_error(
        "RenderGraph::compile()::ERROR: Cycle detected. Aborting.");
  }
}

void RenderGraph::createSyncObjects(
    std::vector<std::vector<size_t>> &dependencies) {
  for (size_t i = 0; i < passes.size(); i++) {
    for (const auto dependency : dependencies[i]) {
      semaphores.emplace_back(renderContext.device.createSemaphore({}));
      semaphoreSignalWaitPairs.emplace_back(dependency, i);
    }
  }
}

uint32_t RenderGraph::findMemoryType(uint32_t memoryTypeBits,
                                     vk::MemoryPropertyFlags properties) {
  vk::PhysicalDeviceMemoryProperties memoryProperties =
      renderContext.physicalDevice.getMemoryProperties();

  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
    if ((memoryTypeBits & (1 << i)) &&
        (memoryProperties.memoryTypes[i].propertyFlags & properties) ==
            properties) {
      return i;
    }
  }

  throw std::runtime_error(
      "RenderGraph::findMemoryType()::ERROR: Cannot find memory type.");
}

void RenderGraph::allocateResources() {
  for (auto &[name, resource] : resources) {
    vk::ImageCreateInfo imageInfo{
        .imageType = vk::ImageType::e2D,
        .format = resource.format,
        .extent = {resource.extent.width, resource.extent.height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = resource.usage,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined};
    resource.image = renderContext.device.createImage(imageInfo);

    vk::MemoryRequirements memoryRequirement;
    renderContext.device.getImageMemoryRequirements(resource.image,
                                                    &memoryRequirement);
    vk::MemoryAllocateInfo allocateInfo{
        .allocationSize = memoryRequirement.size,
        .memoryTypeIndex =
            findMemoryType(memoryRequirement.memoryTypeBits,
                           vk::MemoryPropertyFlagBits::eDeviceLocal)};
    resource.memory = renderContext.device.allocateMemory(allocateInfo);
    renderContext.device.bindImageMemory(resource.image, resource.memory, 0);

    vk::ImageViewCreateInfo viewInfo{
        .image = resource.image,
        .viewType = vk::ImageViewType::e2D,
        .format = resource.format,
        .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                             .baseMipLevel = 0,
                             .levelCount = 1,
                             .baseArrayLayer = 0,
                             .layerCount = 1}};
    resource.view = renderContext.device.createImageView(viewInfo);
  }
}

void RenderGraph::allocateCommandBuffers() {
  vk::CommandBufferAllocateInfo allocateInfo{
      .commandPool = renderContext.commandPool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = static_cast<uint32_t>(executionOrder.size())};
  commandBuffers = renderContext.device.allocateCommandBuffers(allocateInfo);
}

void RenderGraph::compile() {
  std::vector<std::vector<size_t>>
      dependencies; // For each member, it stores the indices of the passes that
                    // it depends on
  std::vector<std::vector<size_t>>
      dependents; // For each member, it stores the indices of the passes that
                  // depends on it
  resolveDependencies(dependencies, dependents);
  resolveExecutionOrder(dependencies, dependents);
  createSyncObjects(dependencies);
  allocateResources();
  allocateCommandBuffers();
}

RenderGraph::Resource *RenderGraph::getResource(const std::string &name) {
  auto it = resources.find(name);
  if (it == resources.end()) {
    return nullptr;
  }

  return &it->second;
}

void RenderGraph::createSemaphores(
    std::vector<vk::Semaphore> &waits, std::vector<vk::Semaphore> &signals,
    std::vector<vk::PipelineStageFlags> &waitStages, size_t passIndex) {
  waits.clear();
  waitStages.clear();

  for (size_t i = 0; i < semaphoreSignalWaitPairs.size(); i++) {
    if (semaphoreSignalWaitPairs[i].second == passIndex) {
      // The pass index i should signal this pass, so add that pass' semaphore
      // to the wait list
      waits.push_back(semaphores[i]);
      waitStages.push_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    }
  }

  signals.clear();
  for (size_t i = 0; i < semaphoreSignalWaitPairs.size(); i++) {
    if (semaphoreSignalWaitPairs[i].first == passIndex) {
      // The pass index i waits for this pass' signal, so add that pass'
      // semaphore to the signal list
      signals.push_back(semaphores[i]);
    }
  }
}

void RenderGraph::transitionInputsLayout(const Pass &pass,
                                         vk::CommandBuffer &commandBuffer) {
  for (const auto &input : pass.inputs) {
    auto &resource = resources[input];
    transitionImageLayout(commandBuffer, resource.image, resource.initialLayout,
                          vk::ImageLayout::eShaderReadOnlyOptimal);
  }
}

void RenderGraph::transitionOutputsLayout(const Pass &pass,
                                          vk::CommandBuffer &commandBuffer) {
  for (const auto &output : pass.outputs) {
    auto &resource = resources[output];
    transitionImageLayout(commandBuffer, resource.image,
                          vk::ImageLayout::eColorAttachmentOptimal,
                          resource.finalLayout);
  }
}

void RenderGraph::transitionImageLayout(vk::CommandBuffer &commandBuffer,
                                        vk::Image image,
                                        vk::ImageLayout srcLayout,
                                        vk::ImageLayout dstLayout) {
  vk::ImageMemoryBarrier barrier{
      .oldLayout = srcLayout,
      .newLayout = dstLayout,
      .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
      .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
      .image = image,
      .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};

  vk::PipelineStageFlags srcStage;
  vk::PipelineStageFlags dstStage;

  if (srcLayout == vk::ImageLayout::eUndefined &&
      dstLayout == vk::ImageLayout::eTransferDstOptimal) {
    barrier.setSrcAccessMask(vk::AccessFlagBits::eNone);
    barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

    srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
    dstStage = vk::PipelineStageFlagBits::eTransfer;
  } else if (srcLayout == vk::ImageLayout::eTransferDstOptimal &&
             dstLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
    barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

    srcStage = vk::PipelineStageFlagBits::eTransfer;
    dstStage = vk::PipelineStageFlagBits::eFragmentShader;
  } else if (dstLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    barrier.setSrcAccessMask(vk::AccessFlagBits::eMemoryWrite);
    barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);
    srcStage = vk::PipelineStageFlagBits::eAllCommands;
    dstStage = vk::PipelineStageFlagBits::eFragmentShader;
  } else if (dstLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    barrier.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
    barrier.setDstAccessMask(vk::AccessFlagBits::eMemoryRead);
    srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dstStage = vk::PipelineStageFlagBits::eAllCommands;
  } else {
    throw std::runtime_error("RenderGraph::transitionImageLayout::ERROR: "
                             "Unsupport layout transition.");
  }

  commandBuffer.pipelineBarrier(srcStage, dstStage,
                                vk::DependencyFlagBits::eByRegion, 0, nullptr,
                                0, nullptr, 1, &barrier);
}

void RenderGraph::execute() {
  std::vector<vk::Semaphore> waitSemaphores;
  std::vector<vk::Semaphore> signalSemaphores;
  std::vector<vk::PipelineStageFlags> waitStages;

  for (const auto passIndex : executionOrder) {
    createSemaphores(waitSemaphores, signalSemaphores, waitStages, passIndex);

    const auto &pass = passes[passIndex];
    auto &commandBuffer = commandBuffers[passIndex];

    commandBuffer.begin({});
    transitionInputsLayout(pass, commandBuffer);
    pass.executeFunction(commandBuffer);
    transitionOutputsLayout(pass, commandBuffer);
    commandBuffer.end();

    vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
        .pWaitSemaphores = waitSemaphores.data(),
        .pWaitDstStageMask = waitStages.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
        .pSignalSemaphores = signalSemaphores.data()};

    renderContext.graphicsQueue.submit(1, &submitInfo, nullptr);
  }
}
} // namespace Core
} // namespace SimpleEngine
