#include "ui/TextElement.h"

// ===================================================================
// CONSTRUCTION / DESTRUCTION
// ===================================================================

TextElement::TextElement() {
  // TODO: Set up OpenGL objects for rendering text quads.
  //   1. Generate a VAO and VBO (glGenVertexArrays, glGenBuffers).
  //   2. Configure the VBO for dynamic draw — each character quad
  //      has 6 vertices with (vec2 position, vec2 texCoord).
  //   3. Optionally call a LoadFont() method here to load glyphs.
}

TextElement::~TextElement() {
  // TODO: Clean up OpenGL resources.
  //   - Delete glyph textures (glDeleteTextures for each entry in m_glyphs).
  //   - Delete VAO and VBO (glDeleteVertexArrays, glDeleteBuffers).
}

// ===================================================================
// TEXT CONTENT
// ===================================================================

void TextElement::SetText(const std::string &text) { m_text = text; }

std::string TextElement::GetText() const { return m_text; }

// ===================================================================
// APPEARANCE
// ===================================================================

void TextElement::SetColor(glm::vec3 color) { m_color = color; }

glm::vec3 TextElement::GetColor() const { return m_color; }

void TextElement::SetScale(float scale) { m_scale = scale; }

float TextElement::GetScale() const { return m_scale; }

// ===================================================================
// UIElement OVERRIDES
// ===================================================================

void TextElement::Render(Shader &shader) {
  // TODO: Render the text string character by character.
  //
  // Outline:
  //   1. Activate the shader and set the "textColor" uniform to m_color.
  //   2. Bind the VAO.
  //   3. For each character in m_text:
  //      a. Look up the GlyphMetrics from m_glyphs.
  //      b. Calculate the quad position using:
  //         - xpos = x + glyph.bearing.x * m_scale
  //         - ypos = y - (glyph.size.y - glyph.bearing.y) * m_scale
  //      c. Calculate the quad size:
  //         - w = glyph.size.x * m_scale
  //         - h = glyph.size.y * m_scale
  //      d. Update the VBO with the 6 vertices for this quad.
  //      e. Bind the glyph texture (glBindTexture).
  //      f. Draw the quad (glDrawArrays(GL_TRIANGLES, 0, 6)).
  //      g. Advance the x cursor by (glyph.advance >> 6) * m_scale.
}

void TextElement::Update(float deltaTime) {
  // TODO: Update any text animations (e.g., typewriter effect, blinking).
  // For static text, this can remain empty.
}
