#include "ui/imgui_vulkan.hpp"
#include "core/render_context.hpp"
#include "enums/texture.hpp"
#include "helpers/vulkan_helper.hpp"
#include "vulkan/vulkan.hpp"
#include <cstddef>
#include <cstdint>
#include <imgui.h>
#include <stdexcept>
#include <utility>

namespace SimpleEngine {
namespace UI {
ImGuiVulkan::ImGuiVulkan(const Core::RenderContext &context)
    : context(context) {
  for (size_t i = 0; i < context.inFlightFrame; i++) {
    const auto &[vertexBuffer, vertexBufferMemory] =
        Helper::VulkanHelper::createBuffer(
            1, vk::BufferUsageFlagBits::eVertexBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible |
                vk::MemoryPropertyFlagBits::eHostCoherent,
            context);

    vertexBuffers.push_back(
        {.buffer = std::move(vertexBuffer),
         .memory = std::move(vertexBufferMemory),
         .mapped = context.device.mapMemory(vertexBufferMemory, 0, 1)});

    const auto &[indexBuffer, indexBufferMemory] =
        Helper::VulkanHelper::createBuffer(
            1, vk::BufferUsageFlagBits::eIndexBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible |
                vk::MemoryPropertyFlagBits::eHostCoherent,
            context);

    indexBuffers.push_back(
        {.buffer = std::move(indexBuffer),
         .memory = std::move(indexBufferMemory),
         .mapped = context.device.mapMemory(indexBufferMemory, 0, 1)});
  }
}

ImGuiVulkan::~ImGuiVulkan() {
  if (context.device) {
    context.device.waitIdle();
    for (size_t i = 0; i < context.inFlightFrame; i++) {
      context.device.unmapMemory(vertexBuffers[i].memory);
      context.device.destroyBuffer(vertexBuffers[i].buffer);
      context.device.freeMemory(vertexBuffers[i].memory);

      context.device.unmapMemory(indexBuffers[i].memory);
      context.device.destroyBuffer(indexBuffers[i].buffer);
      context.device.freeMemory(indexBuffers[i].memory);
    }
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
  sampler = Helper::VulkanHelper::createImageSampler(
      Enums::Texture::Filter::Linear, Enums::Texture::Filter::Linear,
      Enums::Texture::Wrap::ClampToEdge, Enums::Texture::Wrap::ClampToEdge,
      context);

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
}

} // namespace UI
} // namespace SimpleEngine
