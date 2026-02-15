#ifndef OPENGL_CTX_H
#define OPENGL_CTX_H

struct GLFWwindow;

void opengl_ctx_set_window(struct GLFWwindow *window);
struct GLFWwindow *opengl_ctx_get_window(void);

#endif
