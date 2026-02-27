#pragma once

#include <glad/gl.h>
#include <memory>
#include <unordered_map>

enum class TextureType { NEXT, HOLD };

class Texture {
private:
  GLuint m_texID;
  bool m_ownTex;

public:
  Texture(const char *path, bool flip = false);
  Texture(GLuint tex_id, bool own = false);
  ~Texture();
  Texture(const Texture &) = delete;
  Texture &operator=(const Texture &) = delete;

  Texture(Texture &&other) noexcept
      : m_texID(other.m_texID), m_ownTex(other.m_ownTex) {
    other.m_ownTex = false;
    other.m_texID = 0;
  }

  GLuint getTexID() const { return m_texID; };

private:
  GLuint _load_texture(const char *path, bool flip);
};

class TextureManager {
public:
  static std::unordered_map<TextureType, std::unique_ptr<Texture>> textures;

  static Texture &loadTexture(TextureType type, const char *texture_path,
                              bool flip = false);

  static Texture &getTexture(TextureType type);
};
