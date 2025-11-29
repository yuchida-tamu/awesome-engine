#pragma once
#include <iostream>
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

class Shader
{
public:
    Shader();
    ~Shader();
    void AddShader(const std::string &filepath, unsigned int shaderType);
    void LinkProgram();
    void UseProgram();

private:
    std::vector<unsigned int> m_shaderIds;
    unsigned int m_programId;

    // Logging
    int m_success;
    char m_infoLog[512];

    std::string LoadFileAsString(const std::string &filepath);
    void Clear();
};