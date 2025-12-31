#pragma once
#include "rendering/PostProcessEffectStrategy.h"

class PostProcessEdgeEffectStrategy : public IPostProcessEffectStrategy
{
public:
    PostProcessEdgeEffectStrategy();
    ~PostProcessEdgeEffectStrategy();
    void Apply(unsigned int) override;

private:
    Shader m_shader;
};