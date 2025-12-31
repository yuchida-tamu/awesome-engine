#include "rendering/PostProcessBlurEffectStrategy.h"
#include <iostream>

PostProcessBlurEffectStrategy::PostProcessBlurEffectStrategy() : m_shader(Shader{"shaders/postprocess_common.vert", "shaders/postprocess_blur.frag"})
{
}
PostProcessBlurEffectStrategy::~PostProcessBlurEffectStrategy()
{
}

void PostProcessBlurEffectStrategy::Apply(unsigned int FBO)
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO); // back to default
    glDisable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_shader.UseProgram();
}