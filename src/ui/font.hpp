#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <map>
#include <string>

struct Character {
  glm::vec2 uvMin;
  glm::vec2 uvMax;
  glm::ivec2 size;
  glm::ivec2 bearing;
  unsigned int advance;
};

class BitmapFont {
public:
  BitmapFont();
  ~BitmapFont();

  bool loadDefaultFont();
  GLuint getTexID() const { return texID; }
  const Character &getCharacter(char c) const;

private:
  GLuint texID = 0;
  Character m_characters[256];
  void _generate_font_texture();
};
