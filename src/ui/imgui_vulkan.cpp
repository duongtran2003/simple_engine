#include "ui/imgui_vulkan.hpp"
#include "core/render_context.hpp"
#include "core/resource/resource_handle.hpp"
#include "core/resource/resource_manager.hpp"
#include "core/resource/shader.hpp"
#include "core/resource/texture.hpp"
#include "helpers/vulkan_helper.hpp"
#include "vulkan/vulkan.hpp"
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <imgui.h>
#include <stdexcept>
#include <utility>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace SimpleEngine {
namespace UI {
ImGuiVulkan::ImGuiVulkan(const Core::RenderContext &context,
                         Core::ResourceManager &resourceManager)
    : context(context), resourceManager(resourceManager) {
  assert(vertexBuffers.size() == 0);
  assert(indexBuffers.size() == 0);

  createVertexBuffers(1);
  createIndexBuffers(1);
}

void ImGuiVulkan::createVertexBuffers(vk::DeviceSize bufferSize) {
  for (size_t i = 0; i < context.inFlightFrame; i++) {
    const auto &[vertexBuffer, vertexBufferMemory] =
        Helper::VulkanHelper::createBuffer(
            bufferSize, vk::BufferUsageFlagBits::eVertexBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible |
                vk::MemoryPropertyFlagBits::eHostCoherent,
            context);

    vertexBuffers.push_back(
        {.buffer = std::move(vertexBuffer),
         .memory = std::move(vertexBufferMemory),
         .mapped = context.device.mapMemory(vertexBufferMemory, 0, 1)});
  }
}

void ImGuiVulkan::createVertexBuffer(vk::DeviceSize bufferSize,
                                     uint32_t frameIndex) {
  const auto &[vertexBuffer, vertexBufferMemory] =
      Helper::VulkanHelper::createBuffer(
          bufferSize, vk::BufferUsageFlagBits::eVertexBuffer,
          vk::MemoryPropertyFlagBits::eHostVisible |
              vk::MemoryPropertyFlagBits::eHostCoherent,
          context);

  vertexBuffers[frameIndex] = ImGuiVkBuffer{
      .buffer = std::move(vertexBuffer),
      .memory = std::move(vertexBufferMemory),
      .mapped = context.device.mapMemory(vertexBufferMemory, 0, 1)};
}

void ImGuiVulkan::clearVertexBuffers() {
  for (size_t i = 0; i < vertexBuffers.size(); i++) {
    context.device.unmapMemory(vertexBuffers[i].memory);
    context.device.destroyBuffer(vertexBuffers[i].buffer);
    context.device.freeMemory(vertexBuffers[i].memory);
  }

  vertexBuffers.clear();
  vertexBuffers.shrink_to_fit();
}

void ImGuiVulkan::clearVertexBuffer(uint32_t frameIndex) {
  assert(frameIndex < vertexBuffers.size());
  context.device.unmapMemory(vertexBuffers[frameIndex].memory);
  context.device.destroyBuffer(vertexBuffers[frameIndex].buffer);
  context.device.freeMemory(vertexBuffers[frameIndex].memory);
}

void ImGuiVulkan::createIndexBuffers(vk::DeviceSize bufferSize) {
  for (size_t i = 0; i < context.inFlightFrame; i++) {
    const auto &[indexBuffer, indexBufferMemory] =
        Helper::VulkanHelper::createBuffer(
            bufferSize, vk::BufferUsageFlagBits::eIndexBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible |
                vk::MemoryPropertyFlagBits::eHostCoherent,
            context);

    indexBuffers.push_back(
        {.buffer = std::move(indexBuffer),
         .memory = std::move(indexBufferMemory),
         .mapped = context.device.mapMemory(indexBufferMemory, 0, 1)});
  }
}

void ImGuiVulkan::createIndexBuffer(vk::DeviceSize bufferSize,
                                    uint32_t frameIndex) {
  const auto &[indexBuffer, indexBufferMemory] =
      Helper::VulkanHelper::createBuffer(
          bufferSize, vk::BufferUsageFlagBits::eIndexBuffer,
          vk::MemoryPropertyFlagBits::eHostVisible |
              vk::MemoryPropertyFlagBits::eHostCoherent,
          context);

  indexBuffers[frameIndex] = ImGuiVkBuffer{
      .buffer = std::move(indexBuffer),
      .memory = std::move(indexBufferMemory),
      .mapped = context.device.mapMemory(indexBufferMemory, 0, 1)};
}

void ImGuiVulkan::clearIndexBuffers() {
  for (size_t i = 0; i < indexBuffers.size(); i++) {
    context.device.unmapMemory(indexBuffers[i].memory);
    context.device.destroyBuffer(indexBuffers[i].buffer);
    context.device.freeMemory(indexBuffers[i].memory);
  }

  indexBuffers.clear();
  indexBuffers.shrink_to_fit();
}

void ImGuiVulkan::clearIndexBuffer(uint32_t frameIndex) {
  assert(frameIndex < indexBuffers.size());
  context.device.unmapMemory(indexBuffers[frameIndex].memory);
  context.device.destroyBuffer(indexBuffers[frameIndex].buffer);
  context.device.freeMemory(indexBuffers[frameIndex].memory);
}

ImGuiVulkan::~ImGuiVulkan() {
  if (context.device) {
    context.device.waitIdle();
    clearVertexBuffers();
    clearIndexBuffers();
  }
}

void ImGuiVulkan::init(float w, float h) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;

  io.DisplaySize = ImVec2(w, h);
  io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

  uiStyle = ImGui::GetStyle();
  uiStyle.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
  uiStyle.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
  uiStyle.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
  uiStyle.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
  uiStyle.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

  setStyle(0);
}

void ImGuiVulkan::setStyle(uint32_t index) {
  ImGuiStyle &style = ImGui::GetStyle();

  if (index == 0) {
    style = uiStyle;
  } else {
    throw std::runtime_error(
        "ImGuiVulkan::setStyle::ERROR: Style not supported");
  }
}

void ImGuiVulkan::initResources() {
  vk::DescriptorPoolSize poolSize{
      .type = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = 1};

  vk::DescriptorPoolCreateInfo poolCreateInfo{
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets = 2,
      .poolSizeCount = 1,
      .pPoolSizes = &poolSize};

  descriptorPool = context.device.createDescriptorPool(poolCreateInfo);
  vk::DescriptorSetLayoutBinding layoutBinding{
      .binding = 0,
      .descriptorType = vk::DescriptorType::eCombinedImageSampler,
      .descriptorCount = 1,
      .stageFlags = vk::ShaderStageFlagBits::eFragment};

  vk::DescriptorSetLayoutCreateInfo layoutInfo{.bindingCount = 1,
                                               .pBindings = &layoutBinding};

  descriptorSetLayout = context.device.createDescriptorSetLayout(layoutInfo);

  vk::DescriptorSetAllocateInfo descriptorSetAllocInfo{
      .descriptorPool = descriptorPool,
      .descriptorSetCount = 1,
      .pSetLayouts = &descriptorSetLayout};

  descriptorSet =
      context.device.allocateDescriptorSets(descriptorSetAllocInfo)[0];

  ImGuiIO &io = ImGui::GetIO();
  unsigned char *pixels;
  int width, height, channels;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
  channels = 4;

  fontTexture = resourceManager.load<Core::Texture>(
      "im_gui_font", pixels, static_cast<uint32_t>(width),
      static_cast<uint32_t>(height), static_cast<uint32_t>(channels));

  vk::DescriptorImageInfo imageInfo{
      .sampler = fontTexture->getSampler(),
      .imageView = fontTexture->getImageView(),
      .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal};

  vk::WriteDescriptorSet writeDescriptorSet{
      .dstSet = descriptorSet,
      .dstBinding = 0,
      .descriptorCount = 1,
      .descriptorType = vk::DescriptorType::eCombinedImageSampler,
      .pImageInfo = &imageInfo};

  context.device.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);

  vk::PipelineCacheCreateInfo pipelineCacheInfo{};
  pipelineCache = context.device.createPipelineCache(pipelineCacheInfo);

  vk::PushConstantRange pushConstantRange{
      .stageFlags = vk::ShaderStageFlagBits::eVertex,
      .offset = 0,
      .size = sizeof(ImGuiVulkanPushConstant)};

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
      .setLayoutCount = 1,
      .pSetLayouts = &descriptorSetLayout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &pushConstantRange};

  pipelineLayout = context.device.createPipelineLayout(pipelineLayoutInfo);

  Core::ResourceHandle<Core::Shader> vertexShaderHandle =
      resourceManager.load<Core::Shader>(
          "ui_vert", vk::ShaderStageFlagBits::eVertex, "shaders/ui.vert.spv");

  Core::ResourceHandle<Core::Shader> fragmentShaderHandle =
      resourceManager.load<Core::Shader>(
          "ui_frag", vk::ShaderStageFlagBits::eFragment, "shaders/ui.frag.spv");

  vk::PipelineShaderStageCreateInfo vertexShaderStageInfo{
      .stage = vk::ShaderStageFlagBits::eVertex,
      .module = vertexShaderHandle->getShaderModule(),
      .pName = "vertMain"};

  vk::PipelineShaderStageCreateInfo fragmentShaderStageInfo{
      .stage = vk::ShaderStageFlagBits::eFragment,
      .module = fragmentShaderHandle->getShaderModule(),
      .pName = "fragMain"};

  vk::PipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageInfo,
                                                      fragmentShaderStageInfo};

  vk::VertexInputBindingDescription vertexInputBindingDescription{
      .binding = 0,
      .stride = sizeof(ImDrawVert),
      .inputRate = vk::VertexInputRate::eVertex};

  std::array<vk::VertexInputAttributeDescription, 3>
      vertexInputAttributeDescriptions = {};
  vertexInputAttributeDescriptions[0] = {.location = 0,
                                         .binding = 0,
                                         .format = vk::Format::eR32G32Sfloat,
                                         .offset = offsetof(ImDrawVert, pos)};
  vertexInputAttributeDescriptions[1] = {.location = 1,
                                         .binding = 0,
                                         .format = vk::Format::eR32G32Sfloat,
                                         .offset = offsetof(ImDrawVert, uv)};
  vertexInputAttributeDescriptions[2] = {.location = 2,
                                         .binding = 0,
                                         .format = vk::Format::eR8G8B8A8Unorm,
                                         .offset = offsetof(ImDrawVert, col)};

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &vertexInputBindingDescription,
      .vertexAttributeDescriptionCount =
          vertexInputAttributeDescriptions.size(),
      .pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data()};
  vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
      .topology = vk::PrimitiveTopology::eTriangleList};

  std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport,
                                                 vk::DynamicState::eScissor};

  vk::PipelineDynamicStateCreateInfo dynamicState{
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data()};

  vk::PipelineRasterizationStateCreateInfo rasterizer{
      .rasterizerDiscardEnable = vk::False,
      .polygonMode = vk::PolygonMode::eFill,
      .cullMode = vk::CullModeFlagBits::eNone,
      .frontFace = vk::FrontFace::eCounterClockwise,
      .depthBiasEnable = vk::False,
      .depthBiasClamp = vk::False,
      .lineWidth = 1.0f};

  vk::PipelineDepthStencilStateCreateInfo depthStencil{
      .depthTestEnable = vk::False,
      .depthWriteEnable = vk::False,
      .depthCompareOp = vk::CompareOp::eAlways};

  vk::PipelineColorBlendAttachmentState colorBlendAttachment{
      .blendEnable = vk::True,
      .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
      .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
      .colorBlendOp = vk::BlendOp::eAdd,

      .srcAlphaBlendFactor = vk::BlendFactor::eOne,
      .dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
      .alphaBlendOp = vk::BlendOp::eAdd,

      .colorWriteMask =
          vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

  vk::PipelineColorBlendStateCreateInfo colorBlending{
      .logicOpEnable = vk::False,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment};

  vk::PipelineViewportStateCreateInfo viewportState{.viewportCount = 1,
                                                    .pViewports = nullptr,
                                                    .scissorCount = 1,
                                                    .pScissors = nullptr};

  vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo{
      .stageCount = 2,
      .pStages = shaderStages,
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &inputAssembly,
      .pViewportState = &viewportState,
      .pRasterizationState = &rasterizer,
      .pDepthStencilState = &depthStencil,
      .pColorBlendState = &colorBlending,
      .pDynamicState = &dynamicState,
      .layout = pipelineLayout,
      .renderPass = nullptr};

  vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &context.swapChainSurfaceFormat.format};

  vk::StructureChain<vk::GraphicsPipelineCreateInfo,
                     vk::PipelineRenderingCreateInfo>
      pipelineCreateInfoChain = {graphicsPipelineCreateInfo,
                                 pipelineRenderingCreateInfo};

  auto [result, pipelines] = context.device.createGraphicsPipelines(
      nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());

  if (result == vk::Result::eSuccess) {
    vk::Pipeline p = pipelines[0];
    pipeline = p;
  }
}

void ImGuiVulkan::updateTexture(ImTextureData *tex) {
  if (tex->Status == ImTextureStatus_WantCreate ||
      tex->Status == ImTextureStatus_WantUpdates) {
    int w = tex->Width;
    int h = tex->Height;

    unsigned char *fontData = tex->Pixels;

    if (!fontData) {
      return;
    }

    resourceManager.release<Core::Texture>("im_gui_font");
    fontTexture =
        resourceManager.load<Core::Texture>("im_gui_font", fontData, w, h, 4);

    tex->SetTexID((ImTextureID)(intptr_t)(VkDescriptorSet)descriptorSet);
    tex->SetStatus(ImTextureStatus_OK);
  }
}

bool ImGuiVulkan::newFrame() {
  ImGui::NewFrame();
  ImGui::Begin("Vulkan ImGui");
  ImGui::Text("Hello world");

  if (ImGui::Button("Click me!")) {
    // click handler;
  }
  ImGui::End();
  ImGui::EndFrame();

  ImGui::Render();
  ImDrawData *drawData = ImGui::GetDrawData();
  if (drawData && drawData->CmdListsCount > 0) {
    if (drawData->TotalVtxCount > vertexCount ||
        drawData->TotalIdxCount > indexCount) {
      needUpdateBuffers = true;
      return true;
    }
  }

  return false;
}

void ImGuiVulkan::updateBuffers(uint32_t frameIndex) {
  ImDrawData *drawData = ImGui::GetDrawData();
  if (!drawData || drawData->CmdListsCount == 0) {
    return;
  }

  vk::DeviceSize vertexBufferSize =
      drawData->TotalVtxCount * sizeof(ImDrawVert);
  vk::DeviceSize indexBufferSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);

  if (drawData->TotalVtxCount > vertexCount) {
    clearVertexBuffer(frameIndex);
    createVertexBuffer(vertexBufferSize, frameIndex);
  }

  if (drawData->TotalIdxCount > indexCount) {
    clearIndexBuffer(frameIndex);
    createIndexBuffer(indexBufferSize, frameIndex);
  }

  ImDrawVert *vtxDst =
      static_cast<ImDrawVert *>(vertexBuffers[frameIndex].mapped);

  ImDrawVert *idxDst =
      static_cast<ImDrawVert *>(indexBuffers[frameIndex].mapped);

  for (int i = 0; i < drawData->CmdListsCount; i++) {
    const ImDrawList *cmdList = drawData->CmdLists[i];

    memcpy(vtxDst, cmdList->VtxBuffer.Data,
           cmdList->VtxBuffer.size() * sizeof(ImDrawVert));

    memcpy(idxDst, cmdList->IdxBuffer.Data,
           cmdList->IdxBuffer.size() * sizeof(ImDrawIdx));

    vtxDst += cmdList->VtxBuffer.Size;
    idxDst += cmdList->IdxBuffer.Size;
  }
}

} // namespace UI
} // namespace SimpleEngine
