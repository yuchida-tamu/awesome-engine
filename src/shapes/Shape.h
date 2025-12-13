#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shaders/Shader.h"

class Shape
{
public:
    virtual ~Shape() = default;

    /**
     * Initialize the shape's OpenGL resources
     * This must be called before rendering
     * @return true if initialization succeeds, false if it fails.
     */
    virtual bool Initialize() = 0;

    /**
     *
     */

    /**
     * Render the shape using the provided shader program
     * Shader program and modelMatrix must be set for rendering.
     *
     * @param shader the shader program that is used to render the shape
     */
    virtual void Render(Shader *shader) = 0;

    /**
     * Clean up OpenGL resources (VAO, VBO, EBO).
     * Should be called when the shape is no longer needed.
     */
    virtual void Cleanup() = 0;

    /**
     * Get the number of vertices this shape contains.
     * Useful for debugging and optimization.
     *
     * @return The number of vertices
     */
    virtual GLuint GetVertexCount() const = 0;

protected:
    // Protected members can be accessed by derived classes
    GLuint m_VAO = 0;                   // Vertex Array Object
    GLuint m_VBO = 0;                   // Vertex Buffer Object
    GLuint m_EBO = 0;                   // Element Buffer Object (optional, 0 if not used)
    glm::mat4 m_ModelMatrix = glm::mat4(1.0); // Transformation matrix of the shape
    bool m_IsInitialized = false;
};