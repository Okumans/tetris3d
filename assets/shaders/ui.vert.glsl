#version 450 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 u_projection;
uniform mat4 u_model;
uniform vec2 u_uv_min = vec2(0.0, 0.0);
uniform vec2 u_uv_max = vec2(1.0, 1.0);

void main() {
  TexCoords = u_uv_min + aTexCoords * (u_uv_max - u_uv_min);
  gl_Position = u_projection * u_model * vec4(aPos, 0.0, 1.0);
}
