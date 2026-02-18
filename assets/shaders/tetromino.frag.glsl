#version 450 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 u_color;
uniform vec3 u_viewPos; // Camera position for shiny highlights

void main() {
  // 1. Ambient Lighting (The "Darkest" the block can be)
  float ambientStrength = 0.3;
  vec3 ambient = ambientStrength * u_color;

  // 2. Diffuse Lighting (Simple Sun-like light)
  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3)); // Light coming from top-right
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * u_color;

  // 3. Rim Lighting (Optional: adds a slight glow to the edges)
  vec3 viewDir = normalize(u_viewPos - FragPos);
  float rim = 1.0 - max(dot(viewDir, norm), 0.0);
  rim = pow(rim, 3.0);
  vec3 rimLight = rim * vec3(1.0) * 0.2;

  // Final result
  vec3 result = ambient + diffuse + rimLight;

  // Apply a slight vignette/bevel based on UVs (Optional)
  // Darkens the very edges of each cube face
  float edgeWidth = 0.05;
  float vignette = smoothstep(0.0, edgeWidth, TexCoords.x) * smoothstep(1.0, 1.0 - edgeWidth, TexCoords.x) * smoothstep(0.0, edgeWidth, TexCoords.y) * smoothstep(1.0, 1.0 - edgeWidth, TexCoords.y);

  result *= (0.8 + 0.2 * vignette);

  FragColor = vec4(result, 1.0);
}
