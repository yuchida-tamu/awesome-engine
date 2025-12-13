#include "Shader.h"

Shader::Shader(const std::string &vertexShaderPath, const std::string &fragmentShaderPath)
{
    AddShader(vertexShaderPath, GL_VERTEX_SHADER);
    AddShader(fragmentShaderPath, GL_FRAGMENT_SHADER);
    LinkProgram();
}

void Shader::AddShader(const std::string &filepath, unsigned int shaderType)
{
    std::string shaderString = LoadFileAsString(filepath);
    if (shaderString.empty())
    {
        return;
    }

    unsigned int shader = glCreateShader(shaderType);
    const char *sourcePointer = shaderString.c_str();
    glShaderSource(shader, 1, &sourcePointer, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &m_success);

    if (!m_success)
    {
        glGetShaderInfoLog(shader, 512, NULL, m_infoLog);
        std::cerr << "Error: Vertex Shader compilation failed\n"
                  << m_infoLog << std::endl;

        return;
    }

    m_shaderIds.push_back(shader);
}

void Shader::LinkProgram()
{
    m_programId = glCreateProgram();
    for (unsigned int shaderId : m_shaderIds)
    {
        glAttachShader(m_programId, shaderId);
    }

    glLinkProgram(m_programId);
    glGetProgramiv(m_programId, GL_LINK_STATUS, &m_success);
    if (!m_success)
    {
        glGetProgramInfoLog(m_programId, 512, NULL, m_infoLog);
        std::cerr << "Error: Shader Program linking failed\n"
                  << m_infoLog << std::endl;
    }

    // Clean up
    Clear();
}

void Shader::UseProgram()
{
    if (m_programId == 0)
    {
        std::cerr << "Error: Attempting to use a shader that has not been linked or failed to link." << std::endl;
        return;
    }

    glUseProgram(m_programId);
}

void Shader::SetUniformInt(const std::string &name, int value)
{
    glUniform1i(glGetUniformLocation(m_programId, name.c_str()), value);
}

void Shader::SetUniformMatrix4FloatPtr(const std::string &name, const float *ptr)
{
    glUniformMatrix4fv(glGetUniformLocation(m_programId, name.c_str()), 1, GL_FALSE, ptr);
}

std::string Shader::LoadFileAsString(const std::string &filepath)
{
    std::ifstream fileStream(filepath);
    if (!fileStream.is_open())
    {
        std::cerr << "Error: Could not open shader file at path: " << filepath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    return buffer.str();
}

void Shader::Clear()
{
    for (unsigned int shaderId : m_shaderIds)
    {
        glDeleteShader(shaderId);
    }
    m_shaderIds.clear(); // Clear the vector to prevent double-deletion
}

Shader::~Shader()
{
    Clear();
    // Delete the shader program
    if (m_programId != 0)
    {
        glDeleteProgram(m_programId);
        m_programId = 0;
    }
}