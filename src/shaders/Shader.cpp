#include "Shader.h"
#include "core/Config.h"
#include <cstring>
#include <stdexcept>

Shader::Shader(const std::string &vertexShaderPath, const std::string &fragmentShaderPath)
{
    AddShader(vertexShaderPath, GL_VERTEX_SHADER);
    AddShader(fragmentShaderPath, GL_FRAGMENT_SHADER);
    LinkProgram();
}

void Shader::AddShader(const std::string &filepath, GLenum shaderType)
{
    std::string shaderString = LoadFileAsString(filepath);
    if (shaderString.empty())
    {
        throw std::runtime_error("Failed to load shader file: " + filepath);
    }

    GLuint shader = glCreateShader(shaderType);
    if (shader == 0)
    {
        throw std::runtime_error("Failed to create shader object for: " + filepath);
    }

    const char *sourcePointer = shaderString.c_str();
    glShaderSource(shader, 1, &sourcePointer, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &m_success);

    if (!m_success)
    {
        glGetShaderInfoLog(shader, Config::SHADER_LOG_SIZE, NULL, m_infoLog);
        glDeleteShader(shader);
        std::string errorMsg = "Shader compilation failed for " + filepath + ":\n" + m_infoLog;
        throw std::runtime_error(errorMsg);
    }

    m_shaderIds.push_back(shader);
}

void Shader::LinkProgram()
{
    m_programId = glCreateProgram();
    if (m_programId == 0)
    {
        throw std::runtime_error("Failed to create shader program");
    }

    for (GLuint shaderId : m_shaderIds)
    {
        glAttachShader(m_programId, shaderId);
    }

    glLinkProgram(m_programId);
    glGetProgramiv(m_programId, GL_LINK_STATUS, &m_success);
    if (!m_success)
    {
        glGetProgramInfoLog(m_programId, Config::SHADER_LOG_SIZE, NULL, m_infoLog);
        std::string errorMsg = "Shader Program linking failed:\n" + std::string(m_infoLog);
        // Clean up before throwing
        Clear();
        if (m_programId != 0)
        {
            glDeleteProgram(m_programId);
            m_programId = 0;
        }
        throw std::runtime_error(errorMsg);
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

GLint Shader::GetUniformLocation(const std::string &name)
{
    // Check cache first
    auto it = m_uniformCache.find(name);
    if (it != m_uniformCache.end())
    {
        return it->second;
    }

    // Not in cache, query OpenGL and store result
    GLint location = glGetUniformLocation(m_programId, name.c_str());
    m_uniformCache[name] = location;
    return location;
}

void Shader::SetUniformInt(const std::string &name, int value)
{
    GLint location = GetUniformLocation(name);
    if (location != -1)
    {
        glUniform1i(location, value);
    }
    else
    {
        std::cerr << "Warning: Uniform '" << name << "' not found or not used in shader." << std::endl;
    }
}

void Shader::SetUnifromFloat(const std::string &name, float value)
{
    GLint location = GetUniformLocation(name);
    glUniform1f(location, value);
}

void Shader::SetUniformVec3(const std::string &name, const float *ptr)
{
    GLint location = GetUniformLocation(name);
    glUniform3fv(location, 1, ptr);
}

void Shader::SetUniformMatrix4FloatPtr(const std::string &name, const float *ptr)
{
    GLint location = GetUniformLocation(name);
    glUniformMatrix4fv(location, 1, GL_FALSE, ptr);
}

std::string Shader::LoadFileAsString(const std::string &filepath)
{
    std::ifstream fileStream(filepath);
    if (!fileStream.is_open())
    {
        throw std::runtime_error("Could not open shader file at path: " + filepath);
    }
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    return buffer.str();
}

void Shader::Clear()
{
    for (GLuint shaderId : m_shaderIds)
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
    // Clear uniform cache
    m_uniformCache.clear();
}

Shader::Shader(Shader &&other) noexcept
    : m_shaderIds(std::move(other.m_shaderIds)),
      m_programId(other.m_programId),
      m_uniformCache(std::move(other.m_uniformCache)),
      m_success(other.m_success)
{
    // Copy the info log string
    std::memcpy(m_infoLog, other.m_infoLog, sizeof(m_infoLog));

    // Clear the moved-from object to prevent double-deletion
    other.m_shaderIds.clear();
    other.m_programId = 0;
    other.m_uniformCache.clear();
    other.m_success = true;
}

Shader &Shader::operator=(Shader &&other) noexcept
{
    if (this != &other)
    {
        // Clean up current resources
        Clear();
        if (m_programId != 0)
        {
            glDeleteProgram(m_programId);
        }

        // Move resources from other
        m_shaderIds = std::move(other.m_shaderIds);
        m_programId = other.m_programId;
        m_uniformCache = std::move(other.m_uniformCache);
        m_success = other.m_success;
        std::memcpy(m_infoLog, other.m_infoLog, sizeof(m_infoLog));

        // Clear the moved-from object
        other.m_shaderIds.clear();
        other.m_programId = 0;
        other.m_uniformCache.clear();
        other.m_success = true;
    }
    return *this;
}