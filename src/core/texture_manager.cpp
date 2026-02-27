#include "texture_manager.hpp"
#include <glad/gl.h>
#include <memory>
#include <print>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

Texture::Texture(const char *path, bool flip) {
  m_texID = _load_texture(path, flip);
  m_ownTex = true;
}

Texture::Texture(GLuint tex_id, bool own) {
  m_texID = tex_id;
  m_ownTex = own;
}

Texture::~Texture() {
  if (m_ownTex)
    glDeleteTextures(1, &m_texID);
}

GLuint Texture::_load_texture(const char *path, bool flip) {
  GLuint textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;
  stbi_set_flip_vertically_on_load(flip);
  unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
  if (data) {
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    std::println("Texture failed to load at path: {}", path);
    stbi_image_free(data);
  }

  return textureID;
}

std::unordered_map<TextureType, std::unique_ptr<Texture>>
    TextureManager::textures;

Texture &TextureManager::loadTexture(TextureType type, const char *texture_path,
                                     bool flip) {
  textures[type] = std::make_unique<Texture>(texture_path, flip);
  return *TextureManager::textures.at(type);
}

Texture &TextureManager::getTexture(TextureType type) {
  return *TextureManager::textures.at(type);
}
