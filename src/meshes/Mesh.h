#pragma once

#include "glm/glm.hpp"
#include <glad/glad.h>
#include <string>
#include <vector>

#include "shaders/Shader.h"

class Mesh
{
public:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
    };

    struct Texture
    {
        unsigned int id;
        std::string type;
        std::string path;
    };
    Mesh(std::vector<Vertex> vertices, std::vector<Texture> textures, std::vector<unsigned int> indices);
    ~Mesh();

    // Rule of 5: Delete copy operations (OpenGL handles can't be safely copied)
    Mesh(const Mesh &) = delete;
    Mesh &operator=(const Mesh &) = delete;

    // Allow move operations for efficiency
    Mesh(Mesh &&other) noexcept;
    Mesh &operator=(Mesh &&other) noexcept;

    void Draw(Shader &shader);

private:
    unsigned int m_VAO, m_VBO, m_EBO;
    std::vector<Vertex> m_vertices;
    std::vector<Texture> m_textures;
    std::vector<unsigned int> m_indices;

    void initialize();
    void cleanup();
};