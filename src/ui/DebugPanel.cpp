#include "ui/DebugPanel.h"

#include "core/Config.h"
#include "ui/TextElement.h"
#include "ui/UIManager.h"

#include <memory>
#include <sstream>
#include <string>

DebugPanel::DebugPanel(UIManager &uiManager) {
  const float x = 10.0f;
  const float top = static_cast<float>(Config::WINDOW_HEIGHT) - 30.0f;
  const float lineHeight = 26.0f;

  for (int i = 0; i < kLineCount; ++i) {
    auto label = std::make_unique<TextElement>();
    m_lines[i] = label.get(); // UIManager owns it; we keep a raw ptr to mutate
    label->SetPosition({x, top - lineHeight * i}); // bottom-left origin
    label->SetScale(0.4f);
    label->SetColor({1.0f, 1.0f, 0.0f}); // yellow, readable over terrain
    label->SetVisible(false);            // hidden until toggled on
    uiManager.Register(std::move(label));
  }
}

void DebugPanel::Update(const DebugStats &stats) {
  m_lines[0]->SetText("FPS: " +
                      std::to_string(static_cast<int>(stats.fps + 0.5f)));
  m_lines[1]->SetText("Chunks: " + std::to_string(stats.chunkCount));
  m_lines[2]->SetText("Quads: " + std::to_string(stats.quadCount));

  std::ostringstream cam;
  cam << "Pos: " << static_cast<int>(stats.camPos.x) << ", "
      << static_cast<int>(stats.camPos.y) << ", "
      << static_cast<int>(stats.camPos.z) << "   Chunk: " << stats.camChunkX
      << ", " << stats.camChunkZ;
  m_lines[3]->SetText(cam.str());
}

void DebugPanel::SetVisible(bool visible) {
  m_visible = visible;
  for (TextElement *line : m_lines) {
    line->SetVisible(visible);
  }
}

void DebugPanel::Toggle() { SetVisible(!m_visible); }
