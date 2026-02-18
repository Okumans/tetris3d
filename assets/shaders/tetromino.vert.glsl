#version 450 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
  // Calculate the position in world space
  FragPos = vec3(u_model * vec4(aPos, 1.0));

  // Transform normals to world space
  // (We use a Normal Matrix to handle scaling correctly)
  Normal = mat3(transpose(inverse(u_model))) * aNormal;

  TexCoords = aTexCoords;

  gl_Position = u_projection * u_view * vec4(FragPos, 1.0);
}
