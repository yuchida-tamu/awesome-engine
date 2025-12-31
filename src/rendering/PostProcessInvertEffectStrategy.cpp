#include "PostProcessInvertEffectStrategy.h"

PostProcessInvertEffectStrategy::PostProcessInvertEffectStrategy()
    : m_shader(Shader{"shaders/postprocess_common.vert", "shaders/postprocess_invert.frag"})
{
}

PostProcessInvertEffectStrategy::~PostProcessInvertEffectStrategy() {}

void PostProcessInvertEffectStrategy::Apply(unsigned int FBO)
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    glDisable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_shader.UseProgram();
}