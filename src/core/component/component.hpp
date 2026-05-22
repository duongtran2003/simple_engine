#pragma once

namespace SimpleEngine {
namespace Core {
class Entity;
class Component {
private:
  Entity *owner = nullptr;

public:
  virtual ~Component() = default;

  virtual void initialize();
  virtual void update(float deltaTime);
  virtual void render();

  Entity *getOwner() const;
  void setOwner(Entity *entity);
};
} // namespace Core
} // namespace SimpleEngine
