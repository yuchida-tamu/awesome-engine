#pragma once

#include <glad/glad.h>
#include <vector>
#include <iostream>
#include "stb_image.h"

class TextureLoader
{
public:
    TextureLoader(std::vector<std::string> filepaths);
    ~TextureLoader();

    void Bind();

private:
    std::vector<unsigned int> m_textures;
    void Load2D(unsigned int *texture, std::string &filepath);
    void Clear();
};