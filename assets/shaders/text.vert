// Text Vertex Shader
//
// Purpose: Transform screen-space text quad vertices using an orthographic
//          projection, and pass texture coordinates to the fragment shader.
//
// Inputs:
//   layout (location = 0) in vec4 vertex;
//     - vertex.xy = quad position in screen-space pixels
//     - vertex.zw = texture coordinates (0-1 range into the glyph texture)
//
// Uniforms:
//   uniform mat4 projection;
//     - Orthographic projection matrix that maps pixel coordinates to NDC.
//     - Create with: glm::ortho(0.0f, screenWidth, 0.0f, screenHeight)
//
// Outputs:
//   out vec2 TexCoords;
//     - Passed to fragment shader for sampling the glyph texture.
//
// TODO: Implement this shader.
//   1. Declare the vertex input and projection uniform.
//   2. Set gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
//   3. Set TexCoords = vertex.zw;
