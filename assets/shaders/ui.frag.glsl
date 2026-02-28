#version 450 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D u_icon;
uniform vec4 u_color;
uniform bool u_hasTexture;

void main() {
  if (u_hasTexture) {
    vec4 sampled = texture(u_icon, TexCoords);
    // Multiply by u_color to allow "tinting" icons
    FragColor = sampled * u_color;
  } else {
    FragColor = u_color;
  }
}
