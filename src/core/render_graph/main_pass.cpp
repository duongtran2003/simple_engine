#include "core/render_graph/main_pass.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/render_pass.hpp"
#include "core/resource/mesh.hpp"
#include "core/resource/resource_manager.hpp"
#include "vulkan/vulkan.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace SimpleEngine {
namespace Core {
MainPass::MainPass(const std::string &name, CreateInfo createInfo,
                   const RenderContext &context,
                   ResourceManager &resourceManager)
    : RenderPass(name, createInfo, context, resourceManager) {}

MainPass::~MainPass() {
  // TODO
}

void MainPass::init(const CreateInfo &createInfo) {
  createGraphicsPipeline(createInfo);
}

vk::PipelineInputAssemblyStateCreateInfo MainPass::configInputAssembly() {
  return {.topology = vk::PrimitiveTopology::eTriangleList};
}

vk::PipelineVertexInputStateCreateInfo MainPass::configVertexInput() {
  vertexInputBindingDescription = {.binding = 0,
                                   .stride = sizeof(Mesh::Vertex),
                                   .inputRate = vk::VertexInputRate::eVertex};
  vertexInputAttributeDescriptions = {};
  vertexInputAttributeDescriptions.resize(4);
  vertexInputAttributeDescriptions[0] = {.location = 0,
                                         .binding = 0,
                                         .format = vk::Format::eR32G32B32Sfloat,
                                         .offset =
                                             offsetof(Mesh::Vertex, position)};
  vertexInputAttributeDescriptions[1] = {.location = 1,
                                         .binding = 0,
                                         .format = vk::Format::eR32G32B32Sfloat,
                                         .offset =
                                             offsetof(Mesh::Vertex, normal)};
  vertexInputAttributeDescriptions[2] = {.location = 2,
                                         .binding = 0,
                                         .format = vk::Format::eR32G32Sfloat,
                                         .offset = offsetof(Mesh::Vertex, uv)};
  vertexInputAttributeDescriptions[3] = {
      .location = 3,
      .binding = 0,
      .format = vk::Format::eR32G32B32A32Sfloat,
      .offset = offsetof(Mesh::Vertex, tangent)};

  return {.vertexBindingDescriptionCount = 1,
          .pVertexBindingDescriptions = &vertexInputBindingDescription,
          .vertexAttributeDescriptionCount =
              static_cast<uint32_t>(vertexInputAttributeDescriptions.size()),
          .pVertexAttributeDescriptions =
              vertexInputAttributeDescriptions.data()};
}

vk::PipelineLayout MainPass::createGraphicsPipelineLayout() {

  // This is only the global sets, each pass should have their own local sets as
  // well (for example, one pass can have a specific set to sample input)
  std::array<vk::DescriptorSetLayout, 3> layouts = {
      context.descriptorSetLayout, context.bindlessDescriptorSetLayout,
      context.ssboDescriptorSetLayout};

  vk::PushConstantRange pushConstantRange{
      .stageFlags =
          vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
      .offset = 0,
      .size = sizeof(PushConstants)};

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
      .setLayoutCount = layouts.size(),
      .pSetLayouts = layouts.data(),
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &pushConstantRange};

  return context.device.createPipelineLayout(pipelineLayoutInfo);
}
} // namespace Core
} // namespace SimpleEngine
