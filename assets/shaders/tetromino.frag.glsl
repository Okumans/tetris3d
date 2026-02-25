#version 450 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec4 u_color;
uniform vec3 u_viewPos;
uniform float u_time;
uniform bool u_isGhost;

void main() {
  vec3 baseColor = u_color.rgb;
  float alpha = u_color.a;

  vec3 norm = normalize(Normal);
  vec3 viewDir = normalize(u_viewPos - FragPos);

  if (u_isGhost) {
    float edge = step(0.95, TexCoords.x) + step(0.95, TexCoords.y) +
        step(TexCoords.x, 0.05) + step(TexCoords.y, 0.05);
    edge = clamp(edge, 0.0, 1.0);

    vec3 ghostColor = baseColor;
    float ghostAlpha = mix(0.1, 0.6, edge);

    FragColor = vec4(ghostColor, ghostAlpha * alpha);
  }

  else {
    float ambientStrength = 0.35;
    vec3 ambient = ambientStrength * baseColor;

    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.5));
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * baseColor;

    float rim = pow(1.0 - max(dot(viewDir, norm), 0.0), 3.0);
    vec3 rimLight = rim * vec3(1.0) * 0.2;

    vec3 combinedRGB = ambient + diffuse + rimLight;

    // Bevel look
    float edgeWidth = 0.05;
    float vignette = smoothstep(0.0, edgeWidth, TexCoords.x)
        * smoothstep(1.0, 1.0 - edgeWidth, TexCoords.x)
        * smoothstep(0.0, edgeWidth, TexCoords.y)
        * smoothstep(1.0, 1.0 - edgeWidth, TexCoords.y);

    FragColor = vec4(combinedRGB * (0.8 + 0.2 * vignette), alpha);
  }
}
