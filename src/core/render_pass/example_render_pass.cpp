#include "core/render_pass/example_render_pass.hpp"
#include "core/render/render_pass.hpp"
#include "core/render/render_target.hpp"
#include "core/render_context.hpp"
#include "helpers/vulkan_helper.hpp"
#include "vulkan/vulkan.hpp"

#include <array>
#include <string>

namespace SimpleEngine {
namespace Core {
ExampleRenderPass::ExampleRenderPass(const std::string &name,
                                     const RenderContext &context)
    : RenderPass(name, context) {
  RenderTarget *renderTarget = new RenderTarget(
      context.swapChainExtent.width, context.swapChainExtent.height, context);

  setRenderTarget(renderTarget);
}

ExampleRenderPass::~ExampleRenderPass() { delete target; }

void ExampleRenderPass::beginPass(vk::CommandBuffer &commandBuffer) {
  Helper::VulkanHelper::transitionImageLayout(
      commandBuffer, target->getColorImage(), target->getColorLayout(),
      vk::ImageLayout::eColorAttachmentOptimal,
      vk::ImageAspectFlagBits::eColor);

  Helper::VulkanHelper::transitionImageLayout(
      commandBuffer, target->getDepthImage(), target->getDepthLayout(),
      vk::ImageLayout::eDepthStencilAttachmentOptimal,
      vk::ImageAspectFlagBits::eDepth);

  vk::RenderingAttachmentInfoKHR colorAttachment{
      .imageView = target->getColorImageView(),
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue =
          vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f})};

  vk::RenderingAttachmentInfoKHR depthAttachment{
      .imageView = target->getDepthImageView(),
      .imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue = vk::ClearDepthStencilValue(1.0f, 0)};

  vk::RenderingInfoKHR renderingInfo{
      .renderArea = {.offset = {.x = 0, .y = 0},
                     .extent = {.width = target->getWidth(),
                                .height = target->getHeight()}},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachment,
      .pDepthAttachment = &depthAttachment};

  commandBuffer.beginRendering(renderingInfo);
}

void ExampleRenderPass::render(vk::CommandBuffer &commandBuffer) {
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                             context.graphicsPipeline);
  commandBuffer.setViewport(0, context.viewport);
  commandBuffer.setScissor(0, context.scissor);
  commandBuffer.draw(3, 1, 0, 0);
}

void ExampleRenderPass::endPass(vk::CommandBuffer &commandBuffer) {
  commandBuffer.endRendering();
}

} // namespace Core
} // namespace SimpleEngine
