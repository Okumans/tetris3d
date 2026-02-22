#version 450 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec4 u_color;
uniform vec3 u_viewPos;

void main() {
  // 1. Separate RGB and Alpha for control
  vec3 baseColor = u_color.rgb;
  float alpha = u_color.a;

  // 2. Ambient Lighting
  float ambientStrength = 0.3;
  vec3 ambient = ambientStrength * baseColor;

  // 3. Diffuse Lighting
  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * baseColor;

  // 4. Rim Lighting (Glow at edges)
  vec3 viewDir = normalize(u_viewPos - FragPos);
  float rim = 1.0 - max(dot(viewDir, norm), 0.0);
  rim = pow(rim, 3.0);
  vec3 rimLight = rim * vec3(1.0) * 0.2; // Keep this vec3 to avoid affecting alpha

  // 5. Combine Lighting (RGB only)
  vec3 combinedRGB = ambient + diffuse + rimLight;

  // 6. Apply Vignette/Bevel logic to the RGB
  float edgeWidth = 0.05;
  float vignette = smoothstep(0.0, edgeWidth, TexCoords.x)
      * smoothstep(1.0, 1.0 - edgeWidth, TexCoords.x)
      * smoothstep(0.0, edgeWidth, TexCoords.y)
      * smoothstep(1.0, 1.0 - edgeWidth, TexCoords.y);

  combinedRGB *= (0.8 + 0.2 * vignette);

  // 7. Final Result: Use the calculated RGB and the ORIGINAL Alpha
  FragColor = vec4(combinedRGB, alpha);
}
