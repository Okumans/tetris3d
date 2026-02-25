#include <cstddef>
#define _USE_MATH_DEFINES
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <GLFW/glfw3.h>

#include "app.hpp"

#ifndef ASSETS_PATH
#define ASSETS_PATH "./assets"
#endif

#ifndef SHADER_PATH
#define SHADER_PATH ASSETS_PATH "/shader"
#endif

void process_input(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, 1);
}

GLFWwindow *initialize_window(int width, int height, const char *title) {
  if (!glfwInit())
    exit(EXIT_FAILURE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  glfwWindowHint(GLFW_DEPTH_BITS, 24); // Add this before glfwCreateWindow
  GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
  if (!window) {
    fprintf(stderr, "Failed to create GLFW window\n");
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
    fprintf(stderr, "Failed to initialize GLAD\n");
    exit(EXIT_FAILURE);
  }
  return window;
}

//_________________________________________________MAIN______________________________________________________________//

int main(int argc, char *argv[]) {
  GLFWwindow *window = initialize_window(800, 600, "tetris 3D");

  double last_frame_time = glfwGetTime();

  {
    App application(window);

    while (!glfwWindowShouldClose(window)) {
      double current_frame_time = glfwGetTime();
      double delta_frame_time = current_frame_time - last_frame_time;
      last_frame_time = current_frame_time;

      process_input(window); // global event (like closing the window)

      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      application.render(delta_frame_time);

      glfwSwapBuffers(window);
      glfwPollEvents();
    }
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
