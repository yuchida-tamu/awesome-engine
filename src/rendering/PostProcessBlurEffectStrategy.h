#pragma once

#include "glad/glad.h"
#include "rendering/PostProcessEffectStrategy.h"
#include "rendering/Shader.h"

class PostProcessBlurEffectStrategy : public IPostProcessEffectStrategy
{
public:
    PostProcessBlurEffectStrategy();
    ~PostProcessBlurEffectStrategy();
    void Apply(unsigned int outputFBO) override;

private:
    Shader m_shader;
};