#include "core/render_graph/main_pass.hpp"
#include "core/component/mesh_component.hpp"
#include "core/component/transform_component.hpp"
#include "core/engine.hpp"
#include "core/entity/entity.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/graph_resource.hpp"
#include "core/render_graph/render_pass.hpp"
#include "core/resource/mesh.hpp"
#include "core/resource/resource_manager.hpp"
#include "vulkan/vulkan.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Core {
MainPass::MainPass(const std::string &name, CreateInfo createInfo,
                   const RenderContext &context,
                   ResourceManager &resourceManager)
    : RenderPass(name, context, resourceManager) {
  CreateInfo mainPassCreateInfo{
      .shaders = {.vertShader = "main_pass.vert",
                  .fragShader = "main_pass.frag"},
      .rasterizer = {.enableRasterizerDiscard = vk::False,
                     .polygonMode = vk::PolygonMode::eFill,
                     .cullMode = vk::CullModeFlagBits::eBack,
                     .frontFace = vk::FrontFace::eCounterClockwise},
      .colorBlending = {.enableColorBlending = vk::False,
                        .colorBlendingWriteMask =
                            vk::ColorComponentFlagBits::eR |
                            vk::ColorComponentFlagBits::eG |
                            vk::ColorComponentFlagBits::eB |
                            vk::ColorComponentFlagBits::eA,
                        .enableColorBlendingLogicOp = vk::False,
                        .colorBlendingLogicOp = vk::LogicOp::eCopy},
      .depthStencil = {.enableDepthTest = vk::True,
                       .enableDepthWrite = vk::True,
                       .depthCompareOp = vk::CompareOp::eLess,
                       .enableDepthBoundsTest = vk::False,
                       .enableStencilTest = vk::False},
      .rendering = createInfo.rendering};

  init(mainPassCreateInfo);
}

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

void MainPass::execute(vk::CommandBuffer &commandBuffer,
                       std::vector<Entity *> &renderObjects) {
  bool fromLastLayout = false;
  colorAttachment->transitionLayout(commandBuffer, context.frameIndex,
                                    vk::ImageLayout::eColorAttachmentOptimal,
                                    fromLastLayout);
  depthAttachment->transitionLayout(
      commandBuffer, context.frameIndex,
      vk::ImageLayout::eDepthStencilAttachmentOptimal, fromLastLayout);

  vk::RenderingAttachmentInfoKHR colorAttachment{
      .imageView = this->colorAttachment->getView(context.frameIndex),
      .imageLayout = this->colorAttachment->getLayout(context.frameIndex),
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue =
          vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f})};

  vk::RenderingAttachmentInfoKHR depthAttachment{
      .imageView = this->depthAttachment->getView(context.frameIndex),
      .imageLayout = this->depthAttachment->getLayout(context.frameIndex),
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eDontCare,
      .clearValue = vk::ClearDepthStencilValue(1.0f, 0)};

  vk::RenderingInfoKHR renderingInfo{
      .renderArea = {.offset = {.x = 0, .y = 0},
                     .extent = {.width = this->colorAttachment->getWidth(),
                                .height = this->colorAttachment->getHeight()}},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachment,
      .pDepthAttachment = &depthAttachment};

  commandBuffer.beginRendering(renderingInfo);

  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                             getGraphicsPipeline());
  commandBuffer.setViewport(0, context.viewport);
  commandBuffer.setScissor(0, context.scissor);

  std::array<vk::DescriptorSet, 3> descriptorSets = {
      context.descriptorSets[context.frameIndex],
      context.bindlessDescriptorSets,
      context.ssboDescriptorSets[context.frameIndex]};
  commandBuffer.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, graphicsPipelineLayout, 0,
      descriptorSets.size(), descriptorSets.data(), 0, nullptr);

  Engine::ObjectData *ssboDataArray = static_cast<Engine::ObjectData *>(
      context.getCurrentFrameStorageBufferPtr());

  for (size_t i = 0; i < renderObjects.size(); i++) {
    Entity *e = renderObjects[i];

    auto *mesh = e->getComponent<MeshComponent>();
    auto *transform = e->getComponent<TransformComponent>();

    if (!mesh || !mesh->getMesh()->isLoaded()) {
      continue;
    }

    auto meshResource = mesh->getMesh().get();

    vk::Buffer vertexBuffers[] = {meshResource->getVertexBuffer()};
    vk::DeviceSize offsets[] = {0};

    const auto &mat = mesh->getMaterial();

    ssboDataArray[i].model = transform->getTransformMatrix();
    ssboDataArray[i].normalModel = transform->getNormalTransformMatrix();
    ssboDataArray[i].pbrBaseColorFactor = mat.getPbrBaseColorFactor();
    ssboDataArray[i].pbrMetallicFactor = mat.getPbrMetallicFactor();
    ssboDataArray[i].pbrRoughnessFactor = mat.getPbrRoughnessFactor();

    uint32_t albedoIndex = mat.hasAlbedo() ? mat.getAlbedo().index : 0;
    uint32_t normalIndex = mat.hasNormal() ? mat.getNormal().index : 0;
    uint32_t metallicRoughnessIndex =
        mat.hasMetallicRoughness() ? mat.getMetallicRoughness().index : 0;

    PushConstants pushConstant{.albedoIndex = albedoIndex,
                               .normalIndex = normalIndex,
                               .metallicRoughnessIndex = metallicRoughnessIndex,
                               .uniformIndex = static_cast<uint32_t>(i)};

    commandBuffer.pushConstants(graphicsPipelineLayout,
                                vk::ShaderStageFlagBits::eVertex |
                                    vk::ShaderStageFlagBits::eFragment,
                                0, sizeof(PushConstants), &pushConstant);
    commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
    commandBuffer.bindIndexBuffer(meshResource->getIndexBuffer(), 0,
                                  vk::IndexType::eUint32);

    commandBuffer.drawIndexed(meshResource->getIndexCount(), 1, 0, 0, 0);
  }

  commandBuffer.endRendering();
}
} // namespace Core
} // namespace SimpleEngine
