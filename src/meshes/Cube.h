#include <vector>

#include "Mesh.h"
#include "core/TextureLoader.h"
#include "shaders/Shader.h"

class Cube
{
public:
    Cube();
    Cube(std::string path);
    ~Cube();
    void Draw(Shader &shader);
    void SetTexture(std::string path);

private:
    Mesh m_mesh;
    Mesh createMesh();
    Mesh createMeshFromTexture(std::string path);
};