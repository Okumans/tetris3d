#include "geometry.h"

#include "glad/gl.h"

// void draw_ui_quad(GLuint ui_vao) {
//   static GLuint quad_vbo = 0;
//
//   if (quad_vbo == 0) {
//     UIVertex vertices[] = {
//         {{0.0f, 0.0f}, {0.0f, 0.0f}}, {{0.0f, 1.0f}, {0.0f, 1.0f}},
//         {{1.0f, 1.0f}, {1.0f, 1.0f}}, {{0.0f, 0.0f}, {0.0f, 0.0f}},
//         {{1.0f, 1.0f}, {1.0f, 1.0f}}, {{1.0f, 0.0f}, {1.0f, 0.0f}}};
//
//     glCreateBuffers(1, &quad_vbo);
//     glNamedBufferStorage(quad_vbo, sizeof(vertices), vertices, 0);
//     glVertexArrayVertexBuffer(ui_vao, 0, quad_vbo, 0, sizeof(UIVertex));
//   }
//
//   glBindVertexArray(ui_vao);
//   glDrawArrays(GL_TRIANGLES, 0, 6);
// }
