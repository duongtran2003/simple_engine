#pragma once

#include "vulkan/vulkan.hpp"
#include <string>
namespace SimpleEngine {
namespace Core {
class Resource {
private:
  std::string resourceId;
  bool loaded = false;

  vk::Device device;

public:
  Resource(const std::string &id);
  virtual ~Resource() = default;

  const std::string &getId() const;
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
