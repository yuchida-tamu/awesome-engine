// Text Fragment Shader
//
// Purpose: Sample the glyph texture and apply a text color.
//          FreeType glyphs are single-channel (grayscale) bitmaps stored
//          in the red channel of the texture.
//
// Inputs:
//   in vec2 TexCoords;
//     - From vertex shader, used to sample the glyph texture.
//
// Uniforms:
//   uniform sampler2D text;
//     - The glyph texture (single-channel, stored in red).
//   uniform vec3 textColor;
//     - The RGB color to apply to the text.
//
// Output:
//   out vec4 FragColor;
//
// TODO: Implement this shader.
//   1. Sample the glyph texture: float alpha = texture(text, TexCoords).r;
//   2. Output the text color with the sampled alpha:
//      FragColor = vec4(textColor, alpha);
//   3. Fragments with alpha near 0 will be transparent (background shows through).
