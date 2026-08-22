#include "core/render_pass/main_pass.hpp"
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
#include <cstddef>
#include <cstdint>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <glm/trigonometric.hpp>
#include <string>
#include <vector>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/gtc/quaternion.hpp>

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
      .rendering = createInfo.rendering,
      .renderArea = {.offset = {0.0f, 0.0f},
                     .extent = {context.swapChainExtent.width,
                                context.swapChainExtent.height}}};

  init(mainPassCreateInfo);
}

MainPass::~MainPass() {
  // TODO
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
  auto layouts = context.getGlobalDescriptorSetLayouts();

  vk::PushConstantRange pushConstantRange{
      .stageFlags =
          vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
      .offset = 0,
      .size = sizeof(PushConstants)};

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
      .setLayoutCount = static_cast<uint32_t>(layouts.size()),
      .pSetLayouts = layouts.data(),
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &pushConstantRange};

  return context.device.createPipelineLayout(pipelineLayoutInfo);
}

void MainPass::execute(vk::CommandBuffer &commandBuffer,
                       std::vector<Entity *> &renderObjects) {
  std::vector<vk::RenderingAttachmentInfoKHR> renderColorAttachments;
  prepareRenderColorAttachments(renderColorAttachments, commandBuffer);
  auto renderDepthAttachment = prepareRenderDepthAttachment(commandBuffer);
  prepareRenderSampledResources(commandBuffer);

  vk::RenderingInfoKHR renderingInfo;
  prepareRenderingInfo(renderingInfo, renderColorAttachments,
                       renderDepthAttachment);

  commandBuffer.beginRendering(renderingInfo);

  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                             getGraphicsPipeline());
  commandBuffer.setViewport(0, context.viewport);
  commandBuffer.setScissor(0, context.scissor);

  auto descriptorSets = context.getGlobalDescriptorSets();
  commandBuffer.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, graphicsPipelineLayout, 0,
      descriptorSets.size(), descriptorSets.data(), 0, nullptr);

  Engine::ObjectData *ssboDataArray = static_cast<Engine::ObjectData *>(
      context.getCurrentFrameStorageBufferPtr());

  for (size_t i = 0; i < renderObjects.size(); i++) {
    Entity *e = renderObjects[i];

    auto *mesh = e->getComponent<MeshComponent>();
    auto *transform = e->getComponent<TransformComponent>();

    float spinSpeedY = glm::radians(15.0f);
      glm::quat deltaY =
          glm::angleAxis(spinSpeedY * context.getDeltaTime(), glm::vec3(0.0f, 1.0f,
          0.0f));
      
      glm::quat frameRotation = deltaY;
      
      glm::quat currentRotation = transform->getRotation();
      transform->setRotation(frameRotation * currentRotation);

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

    GraphResource *shadowMapResource =
        sampledResources[static_cast<size_t>(SampleSlot::ShadowMap)].resource;
    if (shadowMapResource == nullptr) {
      throw std::runtime_error("MainPass::execute::ERROR: ShadowMap "
                               "sampled resource is required.");
    }
    PushConstants pushConstant{.albedoIndex = albedoIndex,
                               .normalIndex = normalIndex,
                               .metallicRoughnessIndex = metallicRoughnessIndex,
                               .uniformIndex = static_cast<uint32_t>(i),
                               .shadowMapIndex =
                                   shadowMapResource->getBindSlot()};

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
