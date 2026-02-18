#include "shader_manager.h"
#include <memory>
#include <unordered_map>

std::unordered_map<ShaderType, std::unique_ptr<Shader>> ShaderManager::shaders;

Shader &ShaderManager::loadShader(ShaderType type, const char *vertShaderPath,
                                  const char *fragShaderPath) {
  ShaderManager::shaders[type] =
      std::make_unique<Shader>(Shader(vertShaderPath, fragShaderPath));

  return *ShaderManager::shaders.at(type);
}

Shader &ShaderManager::getShader(ShaderType type) {
  return *ShaderManager::shaders.at(type);
}
