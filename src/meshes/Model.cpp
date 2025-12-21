#include "Model.h"

Model::Model(std::string path)
{
    loadModel(path);
}

Model::~Model()
{
    m_directory.clear();
    m_loadedTextures.clear();
    m_meshes.clear();
}

void Model::Draw(Shader &shader)
{
    for (unsigned int i = 0; i < m_meshes.size(); i++)
    {
        m_meshes[i].Draw(shader);
    }
}

void Model::loadModel(std::string path)
{
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    m_directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        // Use move semantics to avoid copying Mesh (which has non-copyable OpenGL handles)
        m_meshes.push_back(std::move(processMesh(mesh, scene)));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}
Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Mesh::Vertex> vertices;
    std::vector<Mesh::Texture> textures;
    std::vector<unsigned int> indices;

    // retrieve vertext data of a mesh
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Mesh::Vertex vertex;

        // It must be done this way to retrieve vector data since assimp's data type does not translate well with glm::vec3 as is.
        glm::vec3 position;
        position.x = mesh->mVertices[i].x;
        position.y = mesh->mVertices[i].y;
        position.z = mesh->mVertices[i].z;
        vertex.position = position;

        glm::vec3 normal;
        normal.x = mesh->mNormals[i].x;
        normal.y = mesh->mNormals[i].y;
        normal.z = mesh->mNormals[i].z;
        vertex.normal = normal;

        if (mesh->mTextureCoords[0])
        {
            glm::vec2 coordinate;
            coordinate.x = mesh->mTextureCoords[0][i].x;
            coordinate.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = coordinate;
        }
        else
        {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    // retrieve indices of each face
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // retrieve material index. sequentially stores diffuseMap and specularMap to a vector (dynamic array)
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Mesh::Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<Mesh::Texture> specularMap = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMap.begin(), specularMap.end());
    }

    return Mesh(vertices, textures, indices);
}
std::vector<Mesh::Texture> Model::loadMaterialTextures(aiMaterial *material, aiTextureType type, std::string typeName)
{
    std::vector<Mesh::Texture> textures;
    for (unsigned int i = 0; i < material->GetTextureCount(type); i++)
    {
        aiString str;
        material->GetTexture(type, i, &str);

        bool skip = false;
        for (unsigned int j = 0; j < m_loadedTextures.size(); j++)
        {
            if (std::strcmp(m_loadedTextures[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(m_loadedTextures[j]);
                skip = true;
                break;
            }
        }

        if (!skip)
        {
            Mesh::Texture texture;
            texture.id = loadTextureFromFile(str.C_Str(), m_directory, false);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            m_loadedTextures.push_back(texture);
        }
    }

    return textures;
};

unsigned int Model::loadTextureFromFile(const char *path, const std::string &directory, bool gamma)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        if (nrComponents == 3)
            format = GL_RGB;
        if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        std::cerr << "Texture failed to load at path: " << path << std::endl;
    }
    stbi_image_free(data);
    return textureID;
}
