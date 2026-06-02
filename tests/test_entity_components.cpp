#include "core/component/transform_component.hpp"
#include "core/entity/entity.hpp"
#include <catch2/catch_test_macros.hpp>
#include <glm/ext/vector_float3.hpp>

TEST_CASE("Entity position and some maths", "[entity]") {
  SimpleEngine::Core::Entity entity("test_entity");
  SimpleEngine::Core::TransformComponent *transformComponent =
      entity.getComponent<SimpleEngine::Core::TransformComponent>();
  if (transformComponent == nullptr) {
    entity.addComponent<SimpleEngine::Core::TransformComponent>();
    transformComponent =
        entity.getComponent<SimpleEngine::Core::TransformComponent>();
  }

  SECTION("Default entity transform component position") {
    glm::vec3 position = transformComponent->getPosition();

    REQUIRE(position.x == 0.0f);
    REQUIRE(position.y == 0.0f);
    REQUIRE(position.z == 0.0f);
  }

  SECTION("Translated entity transform component position") {
    transformComponent->setPosition({1.0f, -2.0f, 5.5f});
    glm::vec3 position = transformComponent->getPosition();

    REQUIRE(position.x == 1.0f);
    REQUIRE(position.y == -2.0f);
    REQUIRE(position.z == 5.5f);
  }
}
