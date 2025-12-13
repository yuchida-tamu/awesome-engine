#pragma once
#include <iostream>
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <glm/glm.hpp>

class Shader
{
public:
    Shader(const std::string &vertexShaderPath, const std::string &fragmentShaderPath);
    ~Shader();
    void UseProgram();

    void SetUniformInt(const std::string &uniformName, int value);
    void SetUniformMatrix4FloatPtr(const std::string &uniformName, const float *ptr);

private:
    std::vector<unsigned int> m_shaderIds;
    unsigned int m_programId = 0;

    // Logging
    int m_success = true;
    char m_infoLog[512];
    std::string LoadFileAsString(const std::string &filepath);

    void AddShader(const std::string &filepath, unsigned int shaderType);
    void LinkProgram();
    void Clear();
};