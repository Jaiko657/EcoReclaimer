#include "gl_loader.h"
#include <stddef.h>

void (*glViewport)(int x, int y, int width, int height) = NULL;
void (*glClearColor)(float r, float g, float b, float a) = NULL;
void (*glClear)(unsigned int mask) = NULL;
void (*glGenBuffers)(int n, unsigned int *buffers) = NULL;
void (*glBindBuffer)(unsigned int target, unsigned int buffer) = NULL;
void (*glBufferData)(unsigned int target, long size, const void *data, unsigned int usage) = NULL;
void (*glDeleteBuffers)(int n, const unsigned int *buffers) = NULL;
void (*glGenVertexArrays)(int n, unsigned int *arrays) = NULL;
void (*glDeleteVertexArrays)(int n, const unsigned int *arrays) = NULL;
void (*glBindVertexArray)(unsigned int array) = NULL;
void (*glEnableVertexAttribArray)(unsigned int index) = NULL;
void (*glVertexAttribPointer)(unsigned int index, int size, unsigned int type, unsigned char normalized, int stride, const void *pointer) = NULL;
void (*glUseProgram)(unsigned int program) = NULL;
int (*glGetUniformLocation)(unsigned int program, const char *name) = NULL;
void (*glUniform1i)(int location, int v0) = NULL;
void (*glUniform4f)(int location, float v0, float v1, float v2, float v3) = NULL;
void (*glUniformMatrix4fv)(int location, int count, unsigned char transpose, const float *value) = NULL;
unsigned int (*glCreateProgram)(void) = NULL;
unsigned int (*glCreateShader)(unsigned int type) = NULL;
void (*glShaderSource)(unsigned int shader, int count, const char *const *string, const int *length) = NULL;
void (*glCompileShader)(unsigned int shader) = NULL;
void (*glGetShaderiv)(unsigned int shader, unsigned int pname, int *params) = NULL;
void (*glGetShaderInfoLog)(unsigned int shader, int bufSize, int *length, char *infoLog) = NULL;
void (*glAttachShader)(unsigned int program, unsigned int shader) = NULL;
void (*glLinkProgram)(unsigned int program) = NULL;
void (*glGetProgramiv)(unsigned int program, unsigned int pname, int *params) = NULL;
void (*glGetProgramInfoLog)(unsigned int program, int bufSize, int *length, char *infoLog) = NULL;
void (*glDeleteShader)(unsigned int shader) = NULL;
void (*glActiveTexture)(unsigned int texture) = NULL;
void (*glBindTexture)(unsigned int target, unsigned int texture) = NULL;
void (*glGenTextures)(int n, unsigned int *textures) = NULL;
void (*glDeleteTextures)(int n, const unsigned int *textures) = NULL;
void (*glTexImage2D)(unsigned int target, int level, int internalformat, int width, int height, int border, unsigned int format, unsigned int type, const void *pixels) = NULL;
void (*glTexSubImage2D)(unsigned int target, int level, int xoffset, int yoffset, int width, int height, unsigned int format, unsigned int type, const void *pixels) = NULL;
void (*glTexParameteri)(unsigned int target, unsigned int pname, int param) = NULL;
void (*glDrawArrays)(unsigned int mode, int first, int count) = NULL;
void (*glReadPixels)(int x, int y, int width, int height, unsigned int format, unsigned int type, void *pixels) = NULL;
void (*glEnable)(unsigned int cap) = NULL;
void (*glBlendFunc)(unsigned int sfactor, unsigned int dfactor) = NULL;
void (*glDisable)(unsigned int cap) = NULL;

static int load_one(void *get_proc(const char *name), const char *sym, void **out)
{
    void *p = get_proc(sym);
    if (!p) return 0;
    *out = p;
    return 1;
}

bool gl_loader_init(void *get_proc(const char *name))
{
    if (!load_one(get_proc, "glViewport", (void **)&glViewport)) return false;
    if (!load_one(get_proc, "glClearColor", (void **)&glClearColor)) return false;
    if (!load_one(get_proc, "glClear", (void **)&glClear)) return false;
    if (!load_one(get_proc, "glGenBuffers", (void **)&glGenBuffers)) return false;
    if (!load_one(get_proc, "glBindBuffer", (void **)&glBindBuffer)) return false;
    if (!load_one(get_proc, "glBufferData", (void **)&glBufferData)) return false;
    if (!load_one(get_proc, "glDeleteBuffers", (void **)&glDeleteBuffers)) return false;
    if (!load_one(get_proc, "glGenVertexArrays", (void **)&glGenVertexArrays)) return false;
    if (!load_one(get_proc, "glDeleteVertexArrays", (void **)&glDeleteVertexArrays)) return false;
    if (!load_one(get_proc, "glBindVertexArray", (void **)&glBindVertexArray)) return false;
    if (!load_one(get_proc, "glEnableVertexAttribArray", (void **)&glEnableVertexAttribArray)) return false;
    if (!load_one(get_proc, "glVertexAttribPointer", (void **)&glVertexAttribPointer)) return false;
    if (!load_one(get_proc, "glUseProgram", (void **)&glUseProgram)) return false;
    if (!load_one(get_proc, "glGetUniformLocation", (void **)&glGetUniformLocation)) return false;
    if (!load_one(get_proc, "glUniform1i", (void **)&glUniform1i)) return false;
    if (!load_one(get_proc, "glUniform4f", (void **)&glUniform4f)) return false;
    if (!load_one(get_proc, "glUniformMatrix4fv", (void **)&glUniformMatrix4fv)) return false;
    if (!load_one(get_proc, "glCreateProgram", (void **)&glCreateProgram)) return false;
    if (!load_one(get_proc, "glCreateShader", (void **)&glCreateShader)) return false;
    if (!load_one(get_proc, "glShaderSource", (void **)&glShaderSource)) return false;
    if (!load_one(get_proc, "glCompileShader", (void **)&glCompileShader)) return false;
    if (!load_one(get_proc, "glGetShaderiv", (void **)&glGetShaderiv)) return false;
    if (!load_one(get_proc, "glGetShaderInfoLog", (void **)&glGetShaderInfoLog)) return false;
    if (!load_one(get_proc, "glAttachShader", (void **)&glAttachShader)) return false;
    if (!load_one(get_proc, "glLinkProgram", (void **)&glLinkProgram)) return false;
    if (!load_one(get_proc, "glGetProgramiv", (void **)&glGetProgramiv)) return false;
    if (!load_one(get_proc, "glGetProgramInfoLog", (void **)&glGetProgramInfoLog)) return false;
    if (!load_one(get_proc, "glDeleteShader", (void **)&glDeleteShader)) return false;
    if (!load_one(get_proc, "glActiveTexture", (void **)&glActiveTexture)) return false;
    if (!load_one(get_proc, "glBindTexture", (void **)&glBindTexture)) return false;
    if (!load_one(get_proc, "glGenTextures", (void **)&glGenTextures)) return false;
    if (!load_one(get_proc, "glDeleteTextures", (void **)&glDeleteTextures)) return false;
    if (!load_one(get_proc, "glTexImage2D", (void **)&glTexImage2D)) return false;
    if (!load_one(get_proc, "glTexSubImage2D", (void **)&glTexSubImage2D)) return false;
    if (!load_one(get_proc, "glTexParameteri", (void **)&glTexParameteri)) return false;
    if (!load_one(get_proc, "glDrawArrays", (void **)&glDrawArrays)) return false;
    if (!load_one(get_proc, "glReadPixels", (void **)&glReadPixels)) return false;
    if (!load_one(get_proc, "glEnable", (void **)&glEnable)) return false;
    if (!load_one(get_proc, "glBlendFunc", (void **)&glBlendFunc)) return false;
    if (!load_one(get_proc, "glDisable", (void **)&glDisable)) return false;
    return true;
}
