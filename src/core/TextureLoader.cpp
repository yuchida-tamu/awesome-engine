#include "TextureLoader.h"

TextureLoader::TextureLoader(std::vector<std::string> filepaths)
{
    stbi_set_flip_vertically_on_load(true);

    for (std::string path : filepaths)
    {
        GLuint texture;
        Load2D(&texture, path);
        m_textures.push_back(texture);
    }
}

TextureLoader::~TextureLoader()
{
    Clear();
}

TextureLoader::TextureLoader(TextureLoader &&other) noexcept
    : m_textures(std::move(other.m_textures))
{
    // Clear the moved-from object's textures to prevent double-deletion
    other.m_textures.clear();
}

TextureLoader &TextureLoader::operator=(TextureLoader &&other) noexcept
{
    if (this != &other)
    {
        // Clean up current resources
        Clear();

        // Move resources from other
        m_textures = std::move(other.m_textures);

        // Clear the moved-from object
        other.m_textures.clear();
    }
    return *this;
}

void TextureLoader::Clear()
{
    if (!m_textures.empty())
    {
        // m_textures.size() は size_t 型
        // glDeleteTextures は GLsizei 型を期待
        // キャストが必要
        glDeleteTextures(
            static_cast<GLsizei>(m_textures.size()), // テクスチャの数
            m_textures.data()                        // テクスチャIDの配列
        );
    }
    // ✅ ベクターをクリア（メモリを解放）
    m_textures.clear();

    // ✅ バインディングを解除（念のため）
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureLoader::Bind()
{
    // OpenGLの定数定義（概念的）
    // #define GL_TEXTURE0 0x84C0
    // #define GL_TEXTURE1 0x84C1 // GL_TEXTURE0 + 1
    // #define GL_TEXTURE2 0x84C2 // GL_TEXTURE0 + 2
    // ... 最大 GL_TEXTURE31 まで（実装によってはもっと多い）

    // つまり、GL_TEXTURE0 + i で動的に指定できる
    // glActiveTexture(GL_TEXTURE0 + 0); // GL_TEXTURE0 と同じ
    // glActiveTexture(GL_TEXTURE0 + 1); // GL_TEXTURE1 と同じ
    // glActiveTexture(GL_TEXTURE0 + 2); // GL_TEXTURE2 と同じ
    for (size_t i = 0; i < m_textures.size(); ++i)
    {
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(i));
        glBindTexture(GL_TEXTURE_2D, m_textures[i]);
    }
}

void TextureLoader::Load2D(GLuint *texture, std::string &filepath)
{
    glGenTextures(1, texture);
    if (*texture == 0)
    {
        std::cerr << "Error: Failed to generate texture object for: " << filepath << std::endl;
        return;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    unsigned char *textureData = stbi_load(filepath.c_str(), &width, &height, &nrChannels, 0);
    if (textureData)
    {
        if (width <= 0 || height <= 0)
        {
            std::cerr << "Error: Invalid texture dimensions for: " << filepath << " (width: " << width << ", height: " << height << ")" << std::endl;
            stbi_image_free(textureData);
            glBindTexture(GL_TEXTURE_2D, 0);
            return;
        }

        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        GLenum internalFormat = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, textureData);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cerr << "Error: Failed to load texture image: " << filepath << std::endl;
    }
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
}