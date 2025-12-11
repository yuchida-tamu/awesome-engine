#include "Shape.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Cube : public Shape
{
public:
    Cube();
    ~Cube() override;

    bool Initialize() override;
    void Render(Shader *shader) override;
    void Translate(float x, float y, float z);
    void Rotate(float angle, float x, float y, float z);
    void ResetPosition();
    unsigned int GetVertexCount() const override;
    void Cleanup() override;

private:
    // Cube-specific data
    static constexpr unsigned int CUBE_VERTEX_COUNT = 36; // 6 faces * 2 triangles * 3 vertices
};