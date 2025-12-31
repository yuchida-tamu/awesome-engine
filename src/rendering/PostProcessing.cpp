#include "PostProcessing.h"

float vertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
    // positions   // texCoords
    -1.0f, 1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 0.0f,

    -1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, -1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f};
PostProcessing::PostProcessing() : m_VAO(0), m_VBO(0), m_FBO(0), m_RBO(0), m_texture(0)
{
    // Create Frame Buffer
    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    // We are setting NULL as the texture's data because it is just allocating memory here at this point.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Attached the allocated texture memory to the framebuffer, which is one of the requirements
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);

    // Create Render buffer
    glGenRenderbuffers(1, &m_RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

    // Check the status of the currently bound framebuffer. Framebuffer needs to meet its requirements to be used
    // - We have to attach at least one buffer (color, depth or stencil buffer).
    // - There should be at least one color attachment.
    // - All attachments should be complete as well(reserved memory).
    // - Each buffer should have the same number of samples.
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Create VAO for screen quad
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

PostProcessing::~PostProcessing()
{
    if (m_VAO != 0)
    {
        glDeleteVertexArrays(1, &m_VAO);
    }

    if (m_VBO != 0)
    {
        glDeleteBuffers(1, &m_VBO);
    }

    if (m_FBO != 0)
    {
        glDeleteFramebuffers(1, &m_FBO);
    }

    if (m_RBO != 0)
    {
        glDeleteRenderbuffers(1, &m_RBO);
    }

    if (m_texture != 0)
    {
        glDeleteTextures(1, &m_texture);
    }
}

void PostProcessing::Begin()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    // Clear the screen (black)
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void PostProcessing::End()
{

    if (!m_strategy)
    {
        std::cout << "[DEBUG] No PostProcess Effect is set" << std::endl;
    }
    else
    {
        // use default frame buffer
        m_strategy->Apply(0);
    }

    glBindVertexArray(m_VAO);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void PostProcessing::SetStrategy(std::unique_ptr<IPostProcessEffectStrategy> strategy)
{
    m_strategy = std::move(strategy);
}