#pragma once

#include <string>
#include <iostream>

#include "glad/glad.h"
#include "rendering/Shader.h"
#include "rendering/PostProcessEffectStrategy.h"
#include "core/Config.h"

class PostProcessing
{

public:
    PostProcessing();
    ~PostProcessing();

    void SetStrategy(std::unique_ptr<IPostProcessEffectStrategy> strategy);
    void ClearStrategy();
    void Begin();
    void End();

private:
    unsigned int m_VAO, m_VBO, m_FBO, m_RBO;
    unsigned int m_texture;
    std::unique_ptr<IPostProcessEffectStrategy> m_strategy;
};