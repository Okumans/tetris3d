#include "camera.h"
#include "game/tetromino.hpp"
#include "glad/gl.h"
#include "shader.h"

#include <GLFW/glfw3.h>
#include <deque>

class TetrisUIRenderer {
private:
  const Camera &m_camera;
  const float m_uiRange = 40.0f;
  const GLuint m_cubeVao;
  const GLuint m_quadVao;

public:
  TetrisUIRenderer(GLuint cubeVao, GLuint quadVao, const Camera &camera)
      : m_camera(camera), m_cubeVao(cubeVao), m_quadVao(quadVao) {}

  void renderPieceQueue(const std::deque<Tetromino> &queue, glm::vec3 startPos,
                        float gap, const Shader &tetrominoShader,
                        const Shader &uiShader, float scale = 1.0f) {
    float boxWidth = 6.0f;
    float boxHeight = (queue.size() * gap) + 4.0f;

    glm::vec3 boxCenter = startPos;
    if (!queue.empty()) {
      boxCenter.y -= (gap * (queue.size() - 1)) / 2.0f;
    }
    _draw2DBorder(boxCenter, boxWidth, boxHeight, uiShader);

    _setupORTO(tetrominoShader);
    for (size_t i = 0; i < queue.size(); ++i) {
      glm::vec3 pos =
          startPos + glm::vec3(0.0f, -(static_cast<float>(i) * gap), 0.0f);
      _drawStaticPiece(queue[i].getType(), pos, tetrominoShader, scale);
    }
  }

  void renderHoldPiece(BlockType type, glm::vec3 world_pos,
                       const Shader &tetrominoShader, const Shader &uiShader,
                       float scale = 1.0f) {
    _draw2DBorder(world_pos, 6.0f, 8.0f, uiShader);

    _setupORTO(tetrominoShader);
    _drawStaticPiece(type, world_pos, tetrominoShader, scale);
  }

private:
  void _draw2DBorder(glm::vec3 center, float width, float height,
                     const Shader &uiShader, float thickness = 0.1f) {
    uiShader.use();
    glBindVertexArray(m_quadVao);

    // Disable depth testing to draw the background flat on the screen
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float aspect = m_camera.GetAspect();
    glm::mat4 proj =
        glm::ortho(0.0f, m_uiRange * aspect, 0.0f, m_uiRange, -10.0f, 10.0f);
    uiShader.setMat4("u_projection", proj);

    // Border color (e.g., a nice visible grey/white)
    uiShader.setVec3("u_color", glm::vec3(0.6f, 0.6f, 0.6f));
    uiShader.setBool("u_hasTexture", false);

    // Calculate the bottom-left origin of the whole box
    float bl_x = center.x - (width / 2.0f);
    float bl_y = center.y - (height / 2.0f);
    float z = -5.0f;

    glm::mat4 models[4];

    // Left edge
    models[0] = glm::translate(glm::mat4(1.0f), glm::vec3(bl_x, bl_y, z));
    models[0] = glm::scale(models[0], glm::vec3(thickness, height, 1.0f));

    // Right edge
    models[1] = glm::translate(glm::mat4(1.0f),
                               glm::vec3(bl_x + width - thickness, bl_y, z));
    models[1] = glm::scale(models[1], glm::vec3(thickness, height, 1.0f));

    // Bottom edge
    models[2] = glm::translate(glm::mat4(1.0f), glm::vec3(bl_x, bl_y, z));
    models[2] = glm::scale(models[2], glm::vec3(width, thickness, 1.0f));

    // Top edge
    models[3] = glm::translate(glm::mat4(1.0f),
                               glm::vec3(bl_x, bl_y + height - thickness, z));
    models[3] = glm::scale(models[3], glm::vec3(width, thickness, 1.0f));

    // Draw the 4 edges
    for (int i = 0; i < 4; ++i) {
      uiShader.setMat4("u_model", models[i]);
      glDrawArrays(GL_TRIANGLES, 0, 6);
    }
  }

  void _setupORTO(const Shader &shader) {
    shader.use();

    glBindVertexArray(m_cubeVao);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    float aspect = m_camera.GetAspect();
    glm::mat4 proj =
        glm::ortho(0.0f, m_uiRange * aspect, 0.0f, m_uiRange, -10.0f, 10.0f);

    shader.setMat4("u_projection", proj);
    shader.setMat4("u_view", glm::mat4(1.0f));

    glClear(GL_DEPTH_BUFFER_BIT);
  }

  void _drawStaticPiece(BlockType type, glm::vec3 world_pos,
                        const Shader &shader, float scale) {
    switch (type) {
    case BlockType::Ghost:
    case BlockType::None:
      return;
    }

    TetrominoData data = TetrominoFactory::getConfig(type);
    glm::vec4 color = glm::vec4(data.color, 1.0f);

    float time = (float)glfwGetTime();
    glm::mat4 rotation =
        glm::rotate(glm::mat4(1.0f), time, glm::vec3(0.2f, 1.0f, 0.0f));

    for (const auto &offset : data.offsets) {
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, world_pos);
      model = glm::scale(model, glm::vec3(scale)); // Apply scale here
      model = model * rotation;
      model = glm::translate(model, glm::vec3(offset));

      shader.setMat4("u_model", model);
      shader.setBool("u_isGhost", false);
      shader.setVec4("u_color", color);
      shader.setFloat("u_time", time);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
  }
};
