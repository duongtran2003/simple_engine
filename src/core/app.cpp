#include <array>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include "core/app.hpp"
#include "core/render_context.hpp"
#include "core/render_graph.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <iostream>
#include <vulkan/vulkan_hpp_macros.hpp>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace SimpleEngine {
namespace Core {
App::App() {
  initRenderContext();
  renderGraph = new RenderGraph(renderContext);
}

void App::setupDeferredRenderer(uint32_t w, uint32_t h) {
  std::cout << "Setting up deferred renderer\n";

  renderGraph->addResource("GBuffer_Position", vk::Format::eR16G16B16A16Sfloat,
                           {.width = w, .height = h},
                           vk::ImageUsageFlagBits::eColorAttachment |
                               vk::ImageUsageFlagBits::eInputAttachment,
                           vk::ImageLayout::eUndefined,
                           vk::ImageLayout::eShaderReadOnlyOptimal);

  renderGraph->addResource("GBuffer_Normal", vk::Format::eR16G16B16A16Sfloat,
                           {.width = w, .height = h},
                           vk::ImageUsageFlagBits::eColorAttachment |
                               vk::ImageUsageFlagBits::eInputAttachment,
                           vk::ImageLayout::eUndefined,
                           vk::ImageLayout::eShaderReadOnlyOptimal);

  renderGraph->addResource("GBuffer_Albedo", vk::Format::eR16G16B16A16Sfloat,
                           {.width = w, .height = h},
                           vk::ImageUsageFlagBits::eColorAttachment |
                               vk::ImageUsageFlagBits::eInputAttachment,
                           vk::ImageLayout::eUndefined,
                           vk::ImageLayout::eShaderReadOnlyOptimal);

  renderGraph->addResource("Depth", vk::Format::eD32Sfloat,
                           {.width = w, .height = h},
                           vk::ImageUsageFlagBits::eDepthStencilAttachment |
                               vk::ImageUsageFlagBits::eInputAttachment,
                           vk::ImageLayout::eUndefined,
                           vk::ImageLayout::eDepthStencilAttachmentOptimal);

  renderGraph->addResource(
      "FinalColor", vk::Format::eR8G8B8A8Unorm, {.width = w, .height = h},
      vk::ImageUsageFlagBits::eColorAttachment |
          vk::ImageUsageFlagBits::eTransferSrc,
      vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferSrcOptimal);

  renderGraph->addPass(
      "GeometryPass", {},
      {"GBuffer_Position", "GBuffer_Normal", "GBuffer_Albedo", "Depth"},
      [&](vk::CommandBuffer &commandBuffer, RenderGraph &renderGraph) {
        std::array<vk::RenderingAttachmentInfoKHR, 3> colorAttachments;

        colorAttachments[0]
            .setImageView(renderGraph.getResource("GBuffer_Position")->view)
            .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore);

        colorAttachments[1]
            .setImageView(renderGraph.getResource("GBuffer_Normal")->view)
            .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore);

        colorAttachments[2]
            .setImageView(renderGraph.getResource("GBuffer_Albedo")->view)
            .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore);

        vk::RenderingAttachmentInfoKHR depthAttachment{
            .imageView = renderGraph.getResource("Depth")->view,
            .imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .clearValue = vk::ClearDepthStencilValue(1.0f, 0)};

        vk::RenderingInfoKHR renderingInfo{
            .renderArea = {.offset = {.x = 0, .y = 0},
                           .extent = {.width = w, .height = h}},
            .layerCount = 1,
            .colorAttachmentCount = colorAttachments.size(),
            .pColorAttachments = colorAttachments.data(),
            .pDepthAttachment = &depthAttachment,
        };

        commandBuffer.beginRendering(renderingInfo);
        // Bind stuff and render stuff here
        commandBuffer.endRendering();
      });

  renderGraph->addPass(
      "LightingPass",
      {"GBuffer_Position", "GBuffer_Normal", "GBuffer_Albedo", "Depth"},
      {"FinalColor"},
      [&](vk::CommandBuffer &commandBuffer, RenderGraph &renderGraph) {
        vk::RenderingAttachmentInfoKHR colorAttachment{
            .imageView = renderGraph.getResource("FinalColor")->view,
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)};

        vk::RenderingInfoKHR renderingInfo{
            .renderArea = {.offset = {.x = 0, .y = 0},
                           .extent = {.width = w, .height = h}},
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachment};

        commandBuffer.beginRendering(renderingInfo);
        // Bind stuff and render stuff here
        commandBuffer.endRendering();
      });

  renderGraph->compile();
}

void App::run() {
  std::cout << "App run\n";
  setupDeferredRenderer(800, 600);
}
} // namespace Core
} // namespace SimpleEngine
