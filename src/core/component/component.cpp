#include "core/component/component.hpp"
#include "core/entity/entity.hpp"

namespace SimpleEngine {
namespace Core {
void Component::initialize() {};
void Component::update(float deltaTime) {};
void Component::render() {};

void Component::setOwner(Entity *entity) { owner = entity; }
Entity *Component::getOwner() const { return owner; }
} // namespace Core
} // namespace SimpleEngine
