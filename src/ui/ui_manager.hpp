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
  glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
  bool hasTexture = false;

public:
  StaticElement(std::string name, UIHitbox box, glm::vec4 color);
  StaticElement(std::string name, UIHitbox box, GLuint tex_id);
  void draw(const class Shader &shader) override;
};

class InteractiveElement : public StaticElement {
public:
  std::function<void()> onClick;

public:
  InteractiveElement(std::string name, UIHitbox box, GLuint tex_id,
                     std::function<void()> cb);
  InteractiveElement(std::string name, UIHitbox box, glm::vec4 color,
                     std::function<void()> cb);
};

#include "font.hpp"

class TextElement : public UIBase {
public:
  std::string text;
  const BitmapFont &font;
  glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
  float scale = 1.0f;

public:
  TextElement(std::string name, UIHitbox box, std::string text,
              const BitmapFont &font, glm::vec4 color, float scale = 1.0f);
  void draw(const class Shader &shader) override;
};

class UIManager {
private:
  std::vector<std::unique_ptr<UIBase>> m_elements;
  std::vector<InteractiveElement *> m_interactives;
  GLuint m_vao = 0;
  GLuint m_vbo = 0;
  float m_virtualWidth = 1.0f;
  float m_virtualHeight = 1.0f;
  int m_lastWindowWidth = 1;
  int m_lastWindowHeight = 1;

public:
  UIManager();
  ~UIManager();

  void addStaticElement(std::string name, UIHitbox box, glm::vec4 color);
  void addStaticElement(std::string name, UIHitbox box, GLuint tex_id);
  void addInteractiveElement(std::string name, UIHitbox box, GLuint tex_id,
                             std::function<void()> cb);
  void addInteractiveElement(std::string name, UIHitbox box, glm::vec4 color,
                             std::function<void()> cb);
  void addTextElement(std::string name, UIHitbox box, std::string text,
                      const BitmapFont &font, glm::vec4 color,
                      float scale = 1.0f);

  void render(int window_width, int window_height);
  bool handleClick(double pos_x, double pos_y);

  GLuint getVAO() const;

private:
  void _setup_buffers();
};
