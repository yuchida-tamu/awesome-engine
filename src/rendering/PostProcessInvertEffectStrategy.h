#pragma once
#include "rendering/PostProcessBlurEffectStrategy.h"

class PostProcessInvertEffectStrategy : public IPostProcessEffectStrategy
{
public:
    PostProcessInvertEffectStrategy();
    ~PostProcessInvertEffectStrategy();
    void Apply(unsigned int FBO) override;

private:
    Shader m_shader;
};