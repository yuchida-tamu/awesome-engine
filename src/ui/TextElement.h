#pragma once

#include "ui/UIElement.h"
#include <ft2build.h>
#include <glm/glm.hpp>
#include <iostream>
#include <map>
#include <string>
#include FT_FREETYPE_H

// Holds the metrics for a single glyph loaded from FreeType.
// Each glyph becomes a small OpenGL texture that we render as a textured quad.
struct GlyphMetrics {
  unsigned int textureID; // OpenGL texture handle for this glyph
  glm::ivec2 size;        // Width and height of the glyph in pixels
  glm::ivec2 bearing;     // Offset from baseline to top-left of glyph
  unsigned int advance; // Horizontal offset to the next glyph (in 1/64 pixels)
};

// Renders a line of text as a series of textured quads.
//
// How text rendering works (you'll implement this):
//   1. Load a .ttf font using FreeType (FT_Init_FreeType, FT_New_Face).
//   2. For each ASCII character (e.g., 0–127), render the glyph to a bitmap
//      and upload it to an OpenGL texture. Store the metrics in m_glyphs.
//   3. To render a string, iterate each character, look up its GlyphMetrics,
//      and draw a textured quad at the correct position using the bearing
//      and advance values to position each character.
//   4. Use a dedicated text shader (text.vert / text.frag) that samples
//      the single-channel glyph texture and applies a color uniform.
class TextElement : public UIElement {
public:
  TextElement();
  ~TextElement() override;

  // --- Text content ---
  void SetText(const std::string &text);
  std::string GetText() const;

  // --- Appearance ---
  void SetColor(glm::vec3 color);
  glm::vec3 GetColor() const;

  void SetScale(float scale);
  float GetScale() const;

  // --- UIElement overrides ---
  void Render(Shader &shader) override;

  void Update(float deltaTime) override;

private:
  std::string m_text;
  glm::vec3 m_color{1.0f, 1.0f, 1.0f}; // Default: white
  float m_scale = 1.0f;

  FT_Library m_ft;
  FT_Face m_face;

  // Maps ASCII character code -> glyph metrics.
  std::map<char, GlyphMetrics> m_glyphs;

  // TODO: OpenGL objects for rendering quads (VAO, VBO).
  unsigned int m_vao = 0;
  unsigned int m_vbo = 0;

  void LoadFont();
  void LoadCharacter();
};
