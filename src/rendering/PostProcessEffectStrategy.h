#pragma once
#include "rendering/Shader.h"

class IPostProcessEffectStrategy
{
public:
    virtual ~IPostProcessEffectStrategy() = default;
    virtual void Apply(unsigned int outputFBO) = 0;
};