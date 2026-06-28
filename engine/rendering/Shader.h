#pragma once
#include "core/Config.h"
#include <fstream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

class Shader {
public:
  Shader(const std::string &vertexShaderPath,
         const std::string &fragmentShaderPath);
  Shader(const std::string &vertexShaderPath,
         const std::string &geometryShaderPath,
         const std::string &fragmentShaderPath);
  ~Shader();

  // Rule of 5: Delete copy operations (OpenGL handles can't be safely copied)
  Shader(const Shader &) = delete;
  Shader &operator=(const Shader &) = delete;

  // Allow move operations for efficiency
  Shader(Shader &&other) noexcept;
  Shader &operator=(Shader &&other) noexcept;

  void UseProgram();

  void SetUniformInt(const std::string &uniformName, int value);
  void SetUnifromFloat(const std::string &uniformName, float value);
  void SetUniformVec2(const std::string &unifromName, const float *ptr);
  void SetUniformVec3(const std::string &uniformName, const float *ptr);
  void SetUniformVec4(const std::string &uniformName, const float *ptr);
  void SetUniformMatrix4FloatPtr(const std::string &uniformName,
                                 const float *ptr);

private:
  std::vector<GLuint> m_shaderIds;
  GLuint m_programId = 0;

  // Uniform location cache
  std::unordered_map<std::string, GLint> m_uniformCache;

  // Logging
  int m_success = true;
  char m_infoLog[Config::SHADER_LOG_SIZE];
  std::string LoadFileAsString(const std::string &filepath);

  void AddShader(const std::string &filepath, GLenum shaderType);
  void LinkProgram();
  void Clear();
  GLint GetUniformLocation(const std::string &name);
};
