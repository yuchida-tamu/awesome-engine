#include "ui/TextElement.h"
#include <glad/glad.h>

#include <_abort.h>
#include <glm/gtc/type_ptr.hpp>
#include <utility>

// ===================================================================
// CONSTRUCTION / DESTRUCTION
// ===================================================================

TextElement::TextElement() {
#ifndef UNIT_TEST
  m_shader.emplace("shaders/default_ui.vert.glsl",
                    "shaders/default_ui.frag.glsl");

  LoadFont();

  // Set default font size
  FT_Set_Pixel_Sizes(m_face, 0, 48);

  if (FT_Load_Char(m_face, 'X', FT_LOAD_RENDER)) {
    std::cerr << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
    abort();
  }

  LoadCharacter();

  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
#endif
}

TextElement::~TextElement() {
  for (auto &[ch, glyph] : m_glyphs) {
    if (glyph.textureID) {
      glDeleteTextures(1, &glyph.textureID);
    }
  }

  if (m_vao) {
    glDeleteVertexArrays(1, &m_vao);
  }
  if (m_vbo) {
    glDeleteBuffers(1, &m_vbo);
  }
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

float TextElement::MeasureWidth() const {
  if (m_text.empty() || m_glyphs.empty()) {
    return 0.0f;
  }

  float width = 0.0f;
  for (char c : m_text) {
    auto it = m_glyphs.find(c);
    if (it != m_glyphs.end()) {
      width += (it->second.advance >> 6) * m_scale;
    }
  }
  return width;
}

// ===================================================================
// UIElement OVERRIDES
// ===================================================================

void TextElement::Render(const glm::mat4 &projection) {
  if (!m_shader)
    return;

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  m_shader->UseProgram();
  m_shader->SetUniformMatrix4FloatPtr("projection",
                                      glm::value_ptr(projection));
  m_shader->SetUniformVec3("textColor", glm::value_ptr(m_color));
  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(m_vao);

  float x = m_position.x;

  std::string::const_iterator c;
  for (c = m_text.begin(); c != m_text.end(); c++) {
    GlyphMetrics glyph = m_glyphs[*c];

    float xpos = x + glyph.bearing.x * m_scale;
    float ypos = m_position.y - (glyph.size.y - glyph.bearing.y) * m_scale;

    float w = glyph.size.x * m_scale;
    float h = glyph.size.y * m_scale;

    float vertices[6][4] = {
        {xpos, ypos + h, 0.0f, 0.0f}, {xpos, ypos, 0.0f, 1.0f},
        {xpos + w, ypos, 1.0f, 1.0f}, {xpos, ypos + h, 0.0f, 0.0f},
        {xpos + w, ypos, 1.0f, 1.0f}, {xpos + w, ypos + h, 1.0f, 0.0f}};

    glBindTexture(GL_TEXTURE_2D, glyph.textureID);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    x += (glyph.advance >> 6) * m_scale;
  }
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
}

void TextElement::Update(float deltaTime) {
  // TODO: Update any text animations (e.g., typewriter effect, blinking).
  // For static text, this can remain empty.
}

void TextElement::LoadFont() {
  if (FT_Init_FreeType(&m_ft)) {
    std::cerr << "ERROR::FREETYPE: Could not init FreeType Library"
              << std::endl;
    abort();
    return;
  }

  if (FT_New_Face(m_ft, "fonts/Roboto.ttf", 0, &m_face)) {
    std::cerr << "ERROR::FREETYPE: Failed to load font" << std::endl;
    abort();
    return;
  }
}

void TextElement::LoadCharacter() {
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte alignment DESTRUCTION
  for (unsigned char c = 0; c < 128; c++) {

    if (FT_Load_Char(m_face, c, FT_LOAD_RENDER)) {
      std::cerr << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
      abort();
    }
    // generate texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_face->glyph->bitmap.width,
                 m_face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
                 m_face->glyph->bitmap.buffer);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GlyphMetrics glyph = {
        texture,
        glm::ivec2(m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows),
        glm::ivec2(m_face->glyph->bitmap_left, m_face->glyph->bitmap_top),
        static_cast<unsigned int>(m_face->glyph->advance.x)};

    m_glyphs.insert(std::pair<char, GlyphMetrics>(c, glyph));
  }

  // Clear resources
  FT_Done_Face(m_face);
  FT_Done_FreeType(m_ft);
}
