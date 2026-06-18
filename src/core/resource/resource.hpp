#pragma once

#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <string>
namespace SimpleEngine {
namespace Core {
class Resource {
private:
  std::string resourceId;
  bool loaded = false;

  const RenderContext &renderContext;

public:
  Resource(const std::string &id, const RenderContext &renderContext);
  virtual ~Resource() = default;

  const std::string &getId() const;
  const RenderContext &getRenderContext() const;

  bool isLoaded() const;

  bool load();
  void unload();

  vk::Device getDevice();

protected:
  virtual bool doLoad() = 0;
  virtual void doUnload() = 0;
};
} // namespace Core
} // namespace SimpleEngine
