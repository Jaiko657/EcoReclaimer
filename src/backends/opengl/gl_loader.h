#ifndef GL_LOADER_H
#define GL_LOADER_H

#include <stdbool.h>

bool gl_loader_init(void *get_proc(const char *name));

extern void (*glViewport)(int x, int y, int width, int height);
extern void (*glClearColor)(float r, float g, float b, float a);
extern void (*glClear)(unsigned int mask);
extern void (*glGenBuffers)(int n, unsigned int *buffers);
extern void (*glBindBuffer)(unsigned int target, unsigned int buffer);
extern void (*glBufferData)(unsigned int target, long size, const void *data, unsigned int usage);
extern void (*glDeleteBuffers)(int n, const unsigned int *buffers);
extern void (*glGenVertexArrays)(int n, unsigned int *arrays);
extern void (*glDeleteVertexArrays)(int n, const unsigned int *arrays);
extern void (*glBindVertexArray)(unsigned int array);
extern void (*glEnableVertexAttribArray)(unsigned int index);
extern void (*glVertexAttribPointer)(unsigned int index, int size, unsigned int type, unsigned char normalized, int stride, const void *pointer);
extern void (*glUseProgram)(unsigned int program);
extern int (*glGetUniformLocation)(unsigned int program, const char *name);
extern void (*glUniform1i)(int location, int v0);
extern void (*glUniform4f)(int location, float v0, float v1, float v2, float v3);
extern void (*glUniformMatrix4fv)(int location, int count, unsigned char transpose, const float *value);
extern unsigned int (*glCreateProgram)(void);
extern unsigned int (*glCreateShader)(unsigned int type);
extern void (*glShaderSource)(unsigned int shader, int count, const char *const *string, const int *length);
extern void (*glCompileShader)(unsigned int shader);
extern void (*glGetShaderiv)(unsigned int shader, unsigned int pname, int *params);
extern void (*glGetShaderInfoLog)(unsigned int shader, int bufSize, int *length, char *infoLog);
extern void (*glAttachShader)(unsigned int program, unsigned int shader);
extern void (*glLinkProgram)(unsigned int program);
extern void (*glGetProgramiv)(unsigned int program, unsigned int pname, int *params);
extern void (*glGetProgramInfoLog)(unsigned int program, int bufSize, int *length, char *infoLog);
extern void (*glDeleteShader)(unsigned int shader);
extern void (*glActiveTexture)(unsigned int texture);
extern void (*glBindTexture)(unsigned int target, unsigned int texture);
extern void (*glGenTextures)(int n, unsigned int *textures);
extern void (*glDeleteTextures)(int n, const unsigned int *textures);
extern void (*glTexImage2D)(unsigned int target, int level, int internalformat, int width, int height, int border, unsigned int format, unsigned int type, const void *pixels);
extern void (*glTexSubImage2D)(unsigned int target, int level, int xoffset, int yoffset, int width, int height, unsigned int format, unsigned int type, const void *pixels);
extern void (*glTexParameteri)(unsigned int target, unsigned int pname, int param);
extern void (*glDrawArrays)(unsigned int mode, int first, int count);
extern void (*glReadPixels)(int x, int y, int width, int height, unsigned int format, unsigned int type, void *pixels);
extern void (*glEnable)(unsigned int cap);
extern void (*glBlendFunc)(unsigned int sfactor, unsigned int dfactor);
extern void (*glDisable)(unsigned int cap);

#define GL_COLOR_BUFFER_BIT         0x00004000
#define GL_DEPTH_BUFFER_BIT         0x00000100
#define GL_TRIANGLES                0x0004
#define GL_ARRAY_BUFFER             0x8892
#define GL_STATIC_DRAW              0x88E4
#define GL_DYNAMIC_DRAW             0x88E8
#define GL_FRAGMENT_SHADER          0x8B30
#define GL_VERTEX_SHADER            0x8B31
#define GL_COMPILE_STATUS           0x8B81
#define GL_LINK_STATUS               0x8B82
#define GL_TEXTURE0                 0x84C0
#define GL_TEXTURE_2D                0x0DE1
#define GL_RGBA                      0x1908
#define GL_UNSIGNED_BYTE             0x1401
#define GL_TEXTURE_MIN_FILTER        0x2801
#define GL_TEXTURE_MAG_FILTER        0x2800
#define GL_LINEAR                    0x2601
#define GL_NEAREST                   0x2600
#define GL_BLEND                    0x0BE2
#define GL_ONE                      1
#define GL_ONE_MINUS_SRC_ALPHA       0x0303
#define GL_SRC_ALPHA                 0x0302
#define GL_FLOAT                     0x1406
#define GL_FALSE                     0

#endif
