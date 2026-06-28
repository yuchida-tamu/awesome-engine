#include "core/FrameBuffer.h"
#include "rendering/PostProcessing.h"
#include "rendering/Shader.h"
#include <iostream>

FrameBuffer::FrameBuffer(int width, int height)
    : m_width(width), m_height(height) {
  glGenFramebuffers(1, &m_fbo);
  glGenTextures(1, &m_colorTexture);
  glGenRenderbuffers(1, &m_depthRbo);

  createAttachements();
}

FrameBuffer::~FrameBuffer() {
  if (m_fbo != 0) {
    glDeleteFramebuffers(1, &m_fbo);
  }

  if (m_depthRbo != 0) {
    glDeleteRenderbuffers(1, &m_depthRbo);
  }

  if (m_colorTexture != 0) {
    glDeleteTextures(1, &m_colorTexture);
  }
}

void FrameBuffer::Bind() {
  if (m_fbo == 0) {
    std::cerr << "Failed to bind a framebuffer: Framebuffer is not created"
              << std::endl;
    return;
  }

  glBindTexture(GL_TEXTURE_2D, m_colorTexture);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
  glViewport(0, 0, m_width, m_height);
}

void FrameBuffer::Unbind() {
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void FrameBuffer::Resize(int width, int height) {
  m_width = width;
  m_height = height;

  createAttachements();
}

void FrameBuffer::createAttachements() {

  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glBindTexture(GL_TEXTURE_2D, m_colorTexture);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         m_colorTexture, 0);

  glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width,
                        m_height);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, m_depthRbo);

  // Requirements of framebuffer
  // - at least one buffer is attached (color, depth, or stencil buffer)
  // - there should be at least one color texture attachment
  // - all attachments should be complete (memoroy is reserved)
  // - each buffer should have the same number of samples
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete"
              << std::endl;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
