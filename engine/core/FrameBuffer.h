#pragma once

class FrameBuffer {
public:
  FrameBuffer(int width, int height);
  ~FrameBuffer();

  void Bind();
  void Unbind();
  void Resize(int width, int height);
  unsigned int GetColorAttachment() const { return m_colorTexture; }
  int GetWidth() { return m_width; }
  int GetHeight() { return m_height; }

private:
  unsigned int m_fbo = 0, m_colorTexture = 0, m_depthRbo = 0;
  int m_width, m_height;

  void createAttachements();
};
