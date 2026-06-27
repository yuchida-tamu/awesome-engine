#pragma once

class Timestep {
public:
  Timestep(float val = 0.0f) : m_seconds(val) {}
  float Seconds() const { return m_seconds; }
  float Milliseconds() const { return m_seconds * 1000.0f; }
  operator float() const { return m_seconds; }

private:
  float m_seconds = 0.0f;
};
