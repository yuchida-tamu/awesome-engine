#include "Cube.h"

// Cube vertex data: position (x,y,z) + texture coordinates (u,v)
static const float CUBE_VERTICES[] = {
    // Positions          // Texture Coords
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

    -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f};

Cube::Cube() {};
Cube::~Cube()
{
    Cleanup();
};

bool Cube::Initialize()
{
    if (m_IsInitialized)
    {
        return true;
    }

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_VERTICES), CUBE_VERTICES, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void *)0);
    glEnableVertexAttribArray(0);

    // texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // release the bindings
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_IsInitialized = true;

    return true;
}

void Cube::Translate(float x, float y, float z)
{
    m_ModelMatrix = glm::translate(m_ModelMatrix, glm::vec3(x, y, z));
}

void Cube::Rotate(float angle, float x, float y, float z)
{
    m_ModelMatrix = glm::rotate(m_ModelMatrix, glm::radians(angle), glm::vec3(x, y, z));
}

void Cube::ResetPosition()
{
    m_ModelMatrix = glm::mat4(1.0f);
}

void Cube::Render(Shader *shader)
{
    if (!m_IsInitialized)
    {
        std::cerr << "Error: it must be initialized" << std::endl;
        return;
    }

    shader->SetUniformMatrix4FloatPtr("model", glm::value_ptr(m_ModelMatrix));
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, CUBE_VERTEX_COUNT);
    glBindVertexArray(0); // Unbind VAO after rendering
}
GLuint Cube::GetVertexCount() const
{
    return CUBE_VERTEX_COUNT;
}

void Cube::Cleanup()
{
    // Delete OpenGL resources if they exist
    if (m_VAO != 0)
    {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0; // Reset to indicate it's been deleted
    }

    if (m_VBO != 0)
    {
        glDeleteBuffers(1, &m_VBO);
        m_VBO = 0;
    }

    if (m_EBO != 0)
    {
        glDeleteBuffers(1, &m_EBO);
        m_EBO = 0;
    }

    // Reset state flags
    // Note: We don't delete m_Shader or m_ModelMatrix because:
    // - m_ModelMatrix is just data, no cleanup needed
    m_ModelMatrix = glm::mat4(1.0f); // Reset to identity matrix

    // Reset state flags
    m_IsInitialized = false;
}