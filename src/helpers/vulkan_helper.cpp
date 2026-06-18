#include "helpers/vulkan_helper.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <stdexcept>

namespace SimpleEngine {
namespace Helper {
uint32_t VulkanHelper::findMemoryType(uint32_t memoryTypeBits,
                                      vk::MemoryPropertyFlags properties,
                                      const vk::PhysicalDevice &physicalDevice) {
  vk::PhysicalDeviceMemoryProperties memoryProperties =
      physicalDevice.getMemoryProperties();
  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
    if ((memoryTypeBits & (1 << i)) &&
        (memoryProperties.memoryTypes[i].propertyFlags & properties) ==
            properties) {
      return i;
    }
  }

  throw std::runtime_error(
      "VulkanHelper::findMemoryType()::ERROR: Cannot find memory type.");
}
} // namespace Helper
} // namespace SimpleEngine
