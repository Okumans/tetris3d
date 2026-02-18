#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>

struct UIHitbox {
  float x, y, w, h;

  bool operator<(const UIHitbox &other) const {
    if (x != other.x)
      return x < other.x;
    return y < other.y;
  }
  bool contains(double px, double py) const {
    return px >= x && px <= x + w && py >= y && py <= y + h;
  }
};

class UIElement {
public:
  std::string name;
  UIHitbox hitbox;
  std::function<void(UIElement *)> onClick;

  GLuint textureID = 0;
  glm::vec3 color = {1.0f, 1.0f, 1.0f};
  bool hasTexture = false;

  UIElement(std::string name, UIHitbox box, glm::vec3 color,
            std::function<void(UIElement *)> cb);
  UIElement(std::string name, UIHitbox box, GLuint tex_id,
            std::function<void(UIElement *)> cb);
};

class UIManager {
private:
  std::map<UIHitbox, std::unique_ptr<UIElement>> m_elements;
  GLuint m_vao = 0;
  GLuint m_vbo = 0;

public:
  UIManager();
  ~UIManager();

  void addElement(std::string name, UIHitbox box, glm::vec3 color,
                  std::function<void(UIElement *)> cb);
  void addElement(std::string name, UIHitbox box, GLuint tex_id,
                  std::function<void(UIElement *)> cb);

  UIElement *getElement(const std::string &name) const;

  bool handleClick(double pos_x, double pos_y);

  void render(int window_width, int window_height);

private:
  void _setup_buffers();
};
