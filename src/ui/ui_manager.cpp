#include "ui_manager.hpp"

#include "glad/gl.h"
#include <glm/gtc/matrix_transform.hpp>

#include "core/geometry.hpp"
#include "core/shader_manager.hpp"

// UIElement
UIElement::UIElement(std::string name, UIHitbox box, glm::vec3 color,
                     std::function<void(UIElement *)> cb)
    : name(std::move(name)), hitbox(box), onClick(std::move(cb)), color(color),
      hasTexture(false) {}

UIElement::UIElement(std::string name, UIHitbox box, GLuint texID,
                     std::function<void(UIElement *)> cb)
    : name(std::move(name)), hitbox(box), onClick(std::move(cb)),
      textureID(texID), hasTexture(true) {}

// UIManager

UIManager::UIManager() { _setup_buffers(); }

UIManager::~UIManager() {
  glDeleteBuffers(1, &m_vbo);
  glDeleteVertexArrays(1, &m_vao);
}

void UIManager::addElement(std::string name, UIHitbox box, glm::vec3 color,
                           std::function<void(UIElement *)> cb) {
  auto el =
      std::make_unique<UIElement>(std::move(name), box, color, std::move(cb));
  m_elements.emplace(box, std::move(el));
}

void UIManager::addElement(std::string name, UIHitbox box, GLuint texID,
                           std::function<void(UIElement *)> cb) {
  auto el =
      std::make_unique<UIElement>(std::move(name), box, texID, std::move(cb));
  m_elements.emplace(box, std::move(el));
}

UIElement *UIManager::getElement(const std::string &name) const {
  for (auto &[_, element] : m_elements) {
    if (element->name == name) {
      return element.get();
    }
  }

  return nullptr;
}

bool UIManager::handleClick(double mouseX, double mouseY) {
  auto it_end = m_elements.upper_bound({(float)mouseX, 0, 0, 0});

  auto it = it_end;
  while (it != m_elements.begin()) {
    --it;
    const UIHitbox &box = it->first;

    // Pruning: if the element ends before the mouseX, we can stop
    // (Since we are iterating backwards from X > mouseX)
    if (box.x + box.w < mouseX)
      break;

    if (box.contains(mouseX, mouseY)) {
      it->second->onClick(it->second.get());
      return true;
    }
  }
  return false;
}

void UIManager::render(int windowWidth, int windowHeight) {
  Shader &shader = ShaderManager::getShader(ShaderType::UI);
  shader.use();

  glm::mat4 projection =
      glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f);
  shader.setMat4("u_projection", projection);

  glDisable(GL_DEPTH_TEST);

  // For icons/transparency
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBindVertexArray(m_vao);

  for (const auto &pair : m_elements) {
    const UIHitbox &box = pair.first;
    const auto &el = pair.second;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(box.x, box.y, 0.0f));
    model = glm::scale(model, glm::vec3(box.w, box.h, 1.0f));

    shader.setMat4("u_model", model);
    shader.setVec3("u_color", el->color);
    shader.setBool("u_hasTexture", el->hasTexture);

    if (el->hasTexture) {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, el->textureID);
      shader.setInt("u_icon", 0);
    }

    glDrawArrays(GL_TRIANGLES, 0, 6);
  }

  glEnable(GL_DEPTH_TEST);
}

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
