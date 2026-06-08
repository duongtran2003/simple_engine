#pragma once

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

protected:
  virtual void onInitialize();
  virtual void onDestroy();
  virtual void update(float deltaTime);
  virtual void render();

  friend class Entity;
};
} // namespace Core
} // namespace SimpleEngine
