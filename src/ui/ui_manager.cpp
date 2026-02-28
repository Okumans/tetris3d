#include "ui_manager.hpp"

#include "glad/gl.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "core/geometry.hpp"
#include "core/shader_manager.hpp"
#include "glm/fwd.hpp"

// StaticElement
StaticElement::StaticElement(std::string name, UIHitbox box, glm::vec4 color)
    : color(color), hasTexture(false) {
  this->name = std::move(name);
  this->bounds = box;
}

StaticElement::StaticElement(std::string name, UIHitbox box, GLuint tex_id)
    : textureID(tex_id), hasTexture(true) {
  this->name = std::move(name);
  this->bounds = box;
}

void StaticElement::draw(const Shader &shader) {
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(bounds.x, bounds.y, 0.0f));
  model = glm::scale(model, glm::vec3(bounds.w, bounds.h, 1.0f));

  shader.setMat4("u_model", model);
  shader.setVec4("u_color", color);
  shader.setBool("u_hasTexture", hasTexture);

  if (hasTexture) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    shader.setInt("u_icon", 0);
  }

  glDrawArrays(GL_TRIANGLES, 0, 6);
}

// InteractiveElement
InteractiveElement::InteractiveElement(std::string name, UIHitbox box,
                                       GLuint tex_id, std::function<void()> cb)
    : StaticElement(std::move(name), box, tex_id), onClick(std::move(cb)) {}

InteractiveElement::InteractiveElement(std::string name, UIHitbox box,
                                       glm::vec4 color,
                                       std::function<void()> cb)
    : StaticElement(std::move(name), box, color), onClick(std::move(cb)) {}

// TextElement
TextElement::TextElement(std::string name, UIHitbox box, std::string text,
                         const BitmapFont &font, glm::vec4 color, float scale)
    : text(std::move(text)), font(font), color(color), scale(scale) {
  this->name = std::move(name);
  this->bounds = box;
}

void TextElement::draw(const Shader &shader) {
  shader.setVec4("u_color", color);
  shader.setBool("u_hasTexture", true);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, font.getTexID());
  shader.setInt("u_icon", 0);

  float currentX = bounds.x;
  float currentY = bounds.y;

  for (char c : text) {
    const Character &ch = font.getCharacter(c);

    float w = ch.size.x * scale;
    float h = ch.size.y * scale;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(currentX, currentY, 0.0f));
    model = glm::scale(model, glm::vec3(w, h, 1.0f));

    shader.setMat4("u_model", model);
    shader.setVec2("u_uv_min", ch.uvMin);
    shader.setVec2("u_uv_max", ch.uvMax);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    currentX += ch.advance * scale;
  }

  // Reset UVs for next elements
  shader.setVec2("u_uv_min", glm::vec2(0.0f, 0.0f));
  shader.setVec2("u_uv_max", glm::vec2(1.0f, 1.0f));
}

// UIManager
UIManager::UIManager() { _setup_buffers(); }

UIManager::~UIManager() {
  glDeleteBuffers(1, &m_vbo);
  glDeleteVertexArrays(1, &m_vao);
}

void UIManager::addStaticElement(std::string name, UIHitbox box,
                                 glm::vec4 color) {
  m_elements.push_back(
      std::make_unique<StaticElement>(std::move(name), box, color));
}

void UIManager::addStaticElement(std::string name, UIHitbox box,
                                 GLuint tex_id) {
  m_elements.push_back(
      std::make_unique<StaticElement>(std::move(name), box, tex_id));
}

void UIManager::addInteractiveElement(std::string name, UIHitbox box,
                                      GLuint tex_id, std::function<void()> cb) {
  auto element = std::make_unique<InteractiveElement>(std::move(name), box,
                                                      tex_id, std::move(cb));
  m_interactives.push_back(element.get());
  m_elements.push_back(std::move(element));
}

void UIManager::addInteractiveElement(std::string name, UIHitbox box,
                                      glm::vec4 color,
                                      std::function<void()> cb) {
  auto element = std::make_unique<InteractiveElement>(std::move(name), box,
                                                      color, std::move(cb));
  m_interactives.push_back(element.get());
  m_elements.push_back(std::move(element));
}

void UIManager::addTextElement(std::string name, UIHitbox box, std::string text,
                               const BitmapFont &font, glm::vec4 color,
                               float scale) {
  m_elements.push_back(std::make_unique<TextElement>(
      std::move(name), box, std::move(text), font, color, scale));
}

UIBase *UIManager::getElement(const std::string &name) {
  auto it = std::ranges::find_if(
      m_elements, [&name](const auto &el) { return el->name == name; });

  return it != m_elements.end() ? it->get() : nullptr;
}

bool UIManager::handleClick(double mouseX, double mouseY) {
  // Convert screen pixels to virtual coordinates
  float vx = (float)mouseX * (m_virtualWidth / (float)m_lastWindowWidth);
  float vy = (float)mouseY * (m_virtualHeight / (float)m_lastWindowHeight);

  for (auto it = m_interactives.rbegin(); it != m_interactives.rend(); ++it) {
    if ((*it)->visible && (*it)->bounds.contains(vx, vy)) {
      if ((*it)->onClick) {
        (*it)->onClick();
        return true;
      }
    }
  }
  return false;
}

void UIManager::render(int windowWidth, int windowHeight) {
  Shader &shader = ShaderManager::getShader(ShaderType::UI);
  shader.use();

  m_lastWindowWidth = windowWidth;
  m_lastWindowHeight = windowHeight;
  m_virtualHeight = 40.0f;
  float aspect = (float)windowWidth / (float)windowHeight;
  m_virtualWidth = m_virtualHeight * aspect;

  glm::mat4 projection =
      glm::ortho(0.0f, m_virtualWidth, m_virtualHeight, 0.0f);
  shader.setMat4("u_projection", projection);

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glBindVertexArray(m_vao);

  for (const auto &el : m_elements) {
    if (el->visible)
      el->draw(shader);
  }

  glEnable(GL_DEPTH_TEST);
}

GLuint UIManager::getVAO() const { return m_vao; }

void UIManager::_setup_buffers() {
  UIVertex vertices[] = {
      {{0.0f, 0.0f}, {0.0f, 0.0f}}, {{0.0f, 1.0f}, {0.0f, 1.0f}},
      {{1.0f, 1.0f}, {1.0f, 1.0f}}, {{0.0f, 0.0f}, {0.0f, 0.0f}},
      {{1.0f, 1.0f}, {1.0f, 1.0f}}, {{1.0f, 0.0f}, {1.0f, 0.0f}}};

  // Create UI VBO
  glCreateBuffers(1, &m_vbo);
  glNamedBufferStorage(m_vbo, sizeof(vertices), vertices, 0);

  // Setup UI VAO
  glCreateVertexArrays(1, &m_vao);

  // index 0: vec2; position attribute
  glEnableVertexArrayAttrib(m_vao, 0);
  glVertexArrayAttribFormat(m_vao, 0, 2, GL_FLOAT, GL_FALSE,
                            offsetof(UIVertex, pos));
  glVertexArrayAttribBinding(m_vao, 0, 0);

  // index 1: vec2; position attribute
  glEnableVertexArrayAttrib(m_vao, 1);
  glVertexArrayAttribFormat(m_vao, 1, 2, GL_FLOAT, GL_FALSE,
                            offsetof(UIVertex, uv));
  glVertexArrayAttribBinding(m_vao, 1, 0);

  // Link VAO <-> VBO
  glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(UIVertex));

  glEnable(GL_DEPTH_TEST);
}
