#pragma once

#include "core/render_context.hpp"
#include "core/resource/resource.hpp"
#include "vulkan/vulkan.hpp"
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Core {
class Shader : public Resource {
private:
  vk::ShaderModule shaderModule;
  vk::ShaderStageFlagBits stage;

public:
  Shader(const std::string &id, const RenderContext &renderContext, vk::ShaderStageFlagBits shaderStage);

  bool doLoad() override;
  void doUnload() override;

  vk::ShaderModule getShaderModule() const;
  vk::ShaderStageFlagBits getShaderStage() const;

private:
  bool readShaderFile(const std::string &path, std::vector<char> &buffer);
  void createShaderModule(const std::vector<char> &shaderCode);
};
} // namespace Core
} // namespace SimpleEngine
