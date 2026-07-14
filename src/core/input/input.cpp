#include "core/input/input.hpp"
#include "core/render_context.hpp"
#include "enums/input.hpp"
#include "ui/imgui_vulkan.hpp"
#include <GLFW/glfw3.h>
#include <glm/ext/vector_float2.hpp>

namespace SimpleEngine {
namespace Core {
Input::Input(const RenderContext &context) : context(context) {
  glfwSetWindowUserPointer(context.window, this);
  glfwSetKeyCallback(context.window, keyCallback);
  glfwSetCursorPosCallback(context.window, mouseCallback);
  glfwSetMouseButtonCallback(context.window, mouseButtonCallback);
  glfwSetCharCallback(context.window, charCallback);
};

Input *Input::setImGui(UI::ImGuiVulkan *imGui) {
  this->imGui = imGui;
  return this;
}

void Input::keyCallback(GLFWwindow *window, int key, int scancode, int action,
                        int mods) {
  auto *instance = static_cast<Input *>(glfwGetWindowUserPointer(window));
  if (!instance || key < 0 || key >= 512) {
    return;
  }

  if (instance->imGui->getWantKeyCapture()) {
    instance->imGui->handleKey(key, scancode, action, mods);
    return;
  }

  if (action == GLFW_PRESS) {
    instance->keys[key] = true;
  } else if (action == GLFW_RELEASE) {
    instance->keys[key] = false;
  }
}

void Input::mouseButtonCallback(GLFWwindow *window, int button, int action,
                                int mods) {
  auto *instance = static_cast<Input *>(glfwGetWindowUserPointer(window));
  if (!instance || !instance->imGui) {
    return;
  }

  instance->imGui->handleMouseButton(button, action == GLFW_PRESS);
}

void Input::mouseCallback(GLFWwindow *window, double xpos, double ypos) {
  auto *instance = static_cast<Input *>(glfwGetWindowUserPointer(window));
  if (!instance) {
    return;
  }

  instance->handleMouseMove(xpos, ypos);

  if (instance->imGui && !instance->isMouseLocked()) {
    instance->imGui->handleMousePos(xpos, ypos);
  }
}

void Input::charCallback(GLFWwindow *window, unsigned int codepoint) {
  auto *instance = static_cast<Input *>(glfwGetWindowUserPointer(window));
  if (!instance || !instance->imGui) {
    return;
  }

  if (instance->imGui->getWantKeyCapture()) {
    instance->imGui->charPressed(codepoint);
  }
}

void Input::toggleMouseLock() {
  mouseLocked = !mouseLocked;
  if (mouseLocked) {
    glfwSetInputMode(context.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(context.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    mouseFirstEnter = true;
  } else {
    glfwSetInputMode(context.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
}

void Input::update() {
  for (int i = 0; i < 512; i++) {
    bool isPressed = keys[i];
    KeyState lastState = keyStates[i];

    if (isPressed) {
      if (lastState == KeyState::None || lastState == KeyState::JustReleased) {
        keyStates[i] = KeyState::JustPressed;
      } else if (lastState == KeyState::JustPressed) {
        keyStates[i] = KeyState::Held;
      }
    } else {
      if (lastState == KeyState::JustPressed || lastState == KeyState::Held) {
        keyStates[i] = KeyState::JustReleased;
      } else if (lastState == KeyState::JustReleased) {
        keyStates[i] = KeyState::None;
      }
    }
  }
}

void Input::handleMouseMove(double xpos, double ypos) {
  if (mouseLocked) {
    if (mouseFirstEnter) {
      lastMousePos = {xpos, ypos};
      mouseFirstEnter = false;
    }

    mouseDelta =
        mouseDelta + glm::vec2(xpos - lastMousePos.x, ypos - lastMousePos.y);
    lastMousePos = {xpos, ypos};
  }
}

bool Input::isKeyJustPressed(Enums::Input::Key key) const {
  int keyCode = static_cast<int>(key);
  return keyStates[keyCode] == KeyState::JustPressed;
}

bool Input::isKeyHeld(Enums::Input::Key key) const {
  int keyCode = static_cast<int>(key);
  return keyStates[keyCode] == KeyState::Held ||
         keyStates[keyCode] == KeyState::JustPressed;
}

bool Input::isKeyJustReleased(Enums::Input::Key key) const {
  int keyCode = static_cast<int>(key);
  return keyStates[keyCode] == KeyState::JustReleased;
}

bool Input::isMouseLocked() const { return mouseLocked; }

glm::vec2 Input::getMouseDelta() const { return mouseDelta; }

void Input::clearMouseDelta() { mouseDelta = {0.0f, 0.0f}; }
} // namespace Core
} // namespace SimpleEngine
