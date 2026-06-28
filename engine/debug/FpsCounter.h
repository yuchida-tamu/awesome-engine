#pragma once

// Windowed FPS averager. Feed it each frame's delta time via Tick(); every
// `windowSeconds` of accumulated time the reported value refreshes to
// (frames / elapsed) over that window. Averaging avoids the wild frame-to-frame
// jitter of a raw 1/dt readout.
//
// Pure: no OpenGL, no globals, no internal clock (dt is passed in), so it is
// fully unit-testable.
class FpsCounter {
public:
  explicit FpsCounter(float windowSeconds = 0.25f)
      : m_window(windowSeconds) {}

  void Tick(float deltaSeconds) {
    m_accum += deltaSeconds;
    m_frames += 1;
    if (m_accum >= m_window) {
      m_fps = m_frames / m_accum;
      m_frames = 0;
      m_accum = 0.0f;
    }
  }

  // Last averaged value; 0 until the first window completes.
  float Fps() const { return m_fps; }

private:
  float m_window;
  float m_accum = 0.0f;
  int m_frames = 0;
  float m_fps = 0.0f;
};
