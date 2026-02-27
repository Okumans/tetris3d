#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

struct UIHitbox {
  float x, y, w, h;

  bool contains(double px, double py) const {
    return px >= x && px <= x + w && py >= y && py <= y + h;
  }
};

class UIBase {
public:
  std::string name;
  UIHitbox bounds;

public:
  virtual ~UIBase() = default;
  virtual void draw(const class Shader &shader) = 0;
};

class StaticElement : public UIBase {
public:
  GLuint textureID = 0;
  glm::vec3 color = {1.0f, 1.0f, 1.0f};
  bool hasTexture = false;

public:
  StaticElement(std::string name, UIHitbox box, glm::vec3 color);
  StaticElement(std::string name, UIHitbox box, GLuint tex_id);
  void draw(const class Shader &shader) override;
};

class InteractiveElement : public StaticElement {
public:
  std::function<void()> onClick;

public:
  InteractiveElement(std::string name, UIHitbox box, GLuint tex_id,
                     std::function<void()> cb);
  InteractiveElement(std::string name, UIHitbox box, glm::vec3 color,
                     std::function<void()> cb);
};

class UIManager {
private:
  std::vector<std::unique_ptr<UIBase>> m_elements;
  std::vector<InteractiveElement *> m_interactives;
  GLuint m_vao = 0;
  GLuint m_vbo = 0;

public:
  UIManager();
  ~UIManager();

  void addStaticElement(std::string name, UIHitbox box, glm::vec3 color);
  void addStaticElement(std::string name, UIHitbox box, GLuint tex_id);
  void addInteractiveElement(std::string name, UIHitbox box, GLuint tex_id,
                             std::function<void()> cb);
  void addInteractiveElement(std::string name, UIHitbox box, glm::vec3 color,
                             std::function<void()> cb);

  void render(int window_width, int window_height);
  bool handleClick(double pos_x, double pos_y);

  GLuint getVAO() const;

private:
  void _setup_buffers();
};
