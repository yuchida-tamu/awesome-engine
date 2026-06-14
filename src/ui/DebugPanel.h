#pragma once

#include <cstddef>
#include <glm/glm.hpp>

class UIManager;
class TextElement;

// Snapshot of the values shown in the debug overlay for one frame.
struct DebugStats {
  float fps;
  size_t chunkCount;
  size_t quadCount;
  glm::vec3 camPos;
  int camChunkX;
  int camChunkZ;
};

// A toggleable on-screen debug overlay (top-left). It does NOT own its text
// elements: it creates and registers them with the UIManager (which owns them),
// keeping raw pointers to update text and visibility each frame — the same
// pattern main.cpp uses for ButtonElement.
class DebugPanel {
public:
  explicit DebugPanel(UIManager &uiManager);

  void Update(const DebugStats &stats); // refresh the text lines
  void Toggle();
  void SetVisible(bool visible);
  bool IsVisible() const { return m_visible; }

private:
  static constexpr int kLineCount = 4;
  TextElement *m_lines[kLineCount] = {};
  bool m_visible = false;
};
