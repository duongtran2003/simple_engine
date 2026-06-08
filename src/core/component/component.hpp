#pragma once

#include "core/system/component_type_id_system.hpp"
#include <cstddef>

namespace SimpleEngine {
namespace Core {
class Entity;
class Component {
public:
  enum class State {
    Unintialized,
    Initialzing,
    Active,
    Destroying,
    Destroyed,
  };

private:
  State state = State::Unintialized;
  Entity *owner = nullptr;

public:
  virtual ~Component();

  virtual void initialize();
  virtual void destroy();

  Entity *getOwner() const;
  void setOwner(Entity *entity);

  bool isActive() const;

  template <typename T> static size_t getTypeId() {
    return ComponentTypeIdSystem::getId<T>();
  }

protected:
  virtual void onInitialize();
  virtual void onDestroy();
  virtual void update(float deltaTime);
  virtual void render();

  friend class Entity;
};
} // namespace Core
} // namespace SimpleEngine
