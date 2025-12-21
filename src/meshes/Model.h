#pragma once

#include <string>
#include <vector>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "shaders/Shader.h"
#include "meshes/Mesh.h"

#include "stb_image.h"

class Model
{
public:
    Model(std::string path);
    ~Model();

    void Draw(Shader &shader);

private:
    std::vector<Mesh> m_meshes;
    std::vector<Mesh::Texture> m_loadedTextures;
    std::string m_directory;

    void loadModel(std::string path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Mesh::Texture> loadMaterialTextures(aiMaterial *material, aiTextureType type, std::string typeName);
    unsigned int loadTextureFromFile(const char *path, const std::string &directory, bool gamma);
};