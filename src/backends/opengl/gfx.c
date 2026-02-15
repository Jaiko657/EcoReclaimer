#include "engine/gfx/gfx.h"
#include "engine/core/logger/logger.h"
#include "engine/core/platform/platform.h"
#include "gl_loader.h"
#include "opengl_ctx.h"

#include <GLFW/glfw3.h>

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

extern void time_backend_tick(void);
extern void input_backend_frame_sync(void);
extern void input_backend_scroll_callback(double xoffset, double yoffset);
static void scroll_cb(GLFWwindow *win, double xoffset, double yoffset)
{
    (void)win;
    input_backend_scroll_callback(xoffset, yoffset);
}

struct gfx_texture {
    unsigned int id;
    int width;
    int height;
};

static platform_window *s_window = NULL;
static int s_screen_w = 0;
static int s_screen_h = 0;
static unsigned int s_vao_quad = 0;
static unsigned int s_vbo_quad = 0;
static unsigned int s_prog_tex = 0;
static unsigned int s_prog_solid = 0;
static unsigned int s_white_tex = 0;
static const gfx_camera2d *s_current_cam = NULL;

static float clamp01(float v)
{
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

static void ortho_matrix(float *out, float left, float right, float bottom, float top)
{
    memset(out, 0, 16 * sizeof(float));
    out[0] = 2.0f / (right - left);
    out[5] = 2.0f / (top - bottom);
    out[10] = -1.0f;
    out[12] = -(right + left) / (right - left);
    out[13] = -(top + bottom) / (top - bottom);
    out[15] = 1.0f;
}

/*
 * 2D camera and world/screen transforms match raylib's Camera2D so that
 * drawing, camera follow, mouse→world (collision), and world→screen (billboards)
 * behave the same. Formula: screen = offset + zoom * R(rotation) * (world - target)
 * with rotation in degrees; NDC then maps screen pixel rect to [-1,1].
 */
#define DEG2RAD (3.14159265358979323846 / 180.0)

static void build_camera_matrix(float *out, int screen_w, int screen_h, const gfx_camera2d *cam)
{
    if (!cam) {
        ortho_matrix(out, 0, (float)screen_w, (float)screen_h, 0);
        return;
    }
    float zoom = cam->zoom > 0.0f ? cam->zoom : 1.0f;
    float w = (float)screen_w;
    float h = (float)screen_h;
    float rad = (float)(cam->rotation * DEG2RAD);
    float c = cosf(rad);
    float s = sinf(rad);
    float tx = cam->target.x;
    float ty = cam->target.y;
    float ox = cam->offset.x;
    float oy = cam->offset.y;
    /* world→screen then screen→NDC: column-major 4x4 from the formula above. */
    memset(out, 0, 16 * sizeof(float));
    out[0]  = (2.0f * zoom * c) / w;
    out[4]  = -(2.0f * zoom * s) / w;
    out[12] = 2.0f * (ox - zoom * (c * tx - s * ty)) / w - 1.0f;
    out[1]  = -(2.0f * zoom * s) / h;
    out[5]  = -(2.0f * zoom * c) / h;
    out[13] = 1.0f - 2.0f * oy / h + 2.0f * zoom * (s * tx + c * ty) / h;
    out[10] = -1.0f;
    out[15] = 1.0f;
}

static unsigned int compile_shader(unsigned int type, const char *src)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    int ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        LOGC(LOGCAT_REND, LOG_LVL_ERROR, "shader compile: %s", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static unsigned int create_program(const char *vs_src, const char *fs_src)
{
    unsigned int vs = compile_shader(GL_VERTEX_SHADER, vs_src);
    unsigned int fs = compile_shader(GL_FRAGMENT_SHADER, fs_src);
    if (!vs || !fs) {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return 0;
    }
    unsigned int prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);
    int ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(prog, sizeof(log), NULL, log);
        LOGC(LOGCAT_REND, LOG_LVL_ERROR, "program link: %s", log);
        return 0;
    }
    return prog;
}

static const char *VS_SOURCE =
    "#version 330 core\n"
    "layout(location = 0) in vec2 a_pos;\n"
    "layout(location = 1) in vec2 a_uv;\n"
    "layout(location = 2) in vec4 a_color;\n"
    "uniform mat4 u_mvp;\n"
    "out vec2 v_uv;\n"
    "out vec4 v_color;\n"
    "void main() {\n"
    "  gl_Position = u_mvp * vec4(a_pos, 0.0, 1.0);\n"
    "  v_uv = a_uv;\n"
    "  v_color = a_color;\n"
    "}\n";

static const char *FS_TEX_SOURCE =
    "#version 330 core\n"
    "uniform sampler2D u_tex;\n"
    "in vec2 v_uv;\n"
    "in vec4 v_color;\n"
    "out vec4 frag;\n"
    "void main() {\n"
    "  frag = texture(u_tex, v_uv) * v_color;\n"
    "}\n";

static const char *FS_SOLID_SOURCE =
    "#version 330 core\n"
    "in vec4 v_color;\n"
    "out vec4 frag;\n"
    "void main() {\n"
    "  frag = v_color;\n"
    "}\n";

bool gfx_init_renderer(platform_window *window)
{
    s_window = window;
    if (!s_window || !opengl_ctx_get_window()) {
        LOGC(LOGCAT_REND, LOG_LVL_FATAL, "Renderer: window not ready");
        return false;
    }
    glfwMakeContextCurrent(opengl_ctx_get_window());
    if (!gl_loader_init((void *)glfwGetProcAddress)) {
        LOGC(LOGCAT_REND, LOG_LVL_FATAL, "Renderer: GL loader failed");
        return false;
    }
    s_screen_w = platform_window_width(s_window);
    s_screen_h = platform_window_height(s_window);
    glfwSetScrollCallback(opengl_ctx_get_window(), scroll_cb);

    s_prog_tex = create_program(VS_SOURCE, FS_TEX_SOURCE);
    s_prog_solid = create_program(VS_SOURCE, FS_SOLID_SOURCE);
    if (!s_prog_tex || !s_prog_solid) return false;

    glGenVertexArrays(1, &s_vao_quad);
    glGenBuffers(1, &s_vbo_quad);
    glBindVertexArray(s_vao_quad);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo_quad);
    glBufferData(GL_ARRAY_BUFFER, 6 * (2 + 2 + 4) * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(4 * sizeof(float)));

    glGenTextures(1, &s_white_tex);
    glBindTexture(GL_TEXTURE_2D, s_white_tex);
    unsigned char white[4] = { 255, 255, 255, 255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return true;
}

void gfx_shutdown(void)
{
    if (s_white_tex) {
        glDeleteTextures(1, &s_white_tex);
        s_white_tex = 0;
    }
    if (s_vao_quad) {
        glDeleteVertexArrays(1, &s_vao_quad);
        s_vao_quad = 0;
    }
    if (s_vbo_quad) {
        glDeleteBuffers(1, &s_vbo_quad);
        s_vbo_quad = 0;
    }
    s_vao_quad = s_vbo_quad = 0;
    s_window = NULL;
}

bool gfx_window_ready(void)
{
    return s_window != NULL && platform_window_ready(s_window);
}

int gfx_screen_width(void)
{
    if (s_window) return platform_window_width(s_window);
    return s_screen_w;
}

int gfx_screen_height(void)
{
    if (s_window) return platform_window_height(s_window);
    return s_screen_h;
}

void gfx_begin_frame(void)
{
    time_backend_tick();
    input_backend_frame_sync();
    if (s_window) {
        s_screen_w = platform_window_width(s_window);
        s_screen_h = platform_window_height(s_window);
        glViewport(0, 0, s_screen_w, s_screen_h);
    }
}

void gfx_end_frame(void)
{
    if (opengl_ctx_get_window())
        glfwSwapBuffers(opengl_ctx_get_window());
}

void gfx_clear(gfx_color color)
{
    glClearColor(clamp01(color.r), clamp01(color.g), clamp01(color.b), clamp01(color.a));
    glClear(GL_COLOR_BUFFER_BIT);
}

static void draw_quad(float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float r, float g, float b, float a)
{
    float verts[] = {
        x0, y0, u0, v0, r, g, b, a,
        x1, y0, u1, v0, r, g, b, a,
        x1, y1, u1, v1, r, g, b, a,
        x0, y0, u0, v0, r, g, b, a,
        x1, y1, u1, v1, r, g, b, a,
        x0, y1, u0, v1, r, g, b, a,
    };
    glBindVertexArray(s_vao_quad);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo_quad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

static void draw_quad4(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3,
                      float u0, float v0, float u1, float v1, float r, float g, float b, float a)
{
    float verts[] = {
        x0, y0, u0, v0, r, g, b, a,
        x1, y1, u1, v0, r, g, b, a,
        x2, y2, u1, v1, r, g, b, a,
        x0, y0, u0, v0, r, g, b, a,
        x2, y2, u1, v1, r, g, b, a,
        x3, y3, u0, v1, r, g, b, a,
    };
    glBindVertexArray(s_vao_quad);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo_quad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void gfx_begin_world(const gfx_camera2d *cam)
{
    s_current_cam = cam;
}

void gfx_end_world(void)
{
    s_current_cam = NULL;
}

static void get_mvp(float *mvp)
{
    int w = gfx_screen_width();
    int h = gfx_screen_height();
    if (s_current_cam)
        build_camera_matrix(mvp, w, h, s_current_cam);
    else
        ortho_matrix(mvp, 0, (float)w, (float)h, 0);
}

gfx_vec2 gfx_world_to_screen(gfx_vec2 world, const gfx_camera2d *cam)
{
    if (!cam) {
        return world;
    }
    float zoom = cam->zoom > 0.0f ? cam->zoom : 1.0f;
    float rad = (float)(cam->rotation * DEG2RAD);
    float c = cosf(rad);
    float s = sinf(rad);
    float dx = world.x - cam->target.x;
    float dy = world.y - cam->target.y;
    gfx_vec2 out;
    out.x = cam->offset.x + zoom * (c * dx - s * dy);
    out.y = cam->offset.y + zoom * (s * dx + c * dy);
    return out;
}

gfx_vec2 gfx_screen_to_world(gfx_vec2 screen, const gfx_camera2d *cam)
{
    if (!cam) {
        return screen;
    }
    float zoom = cam->zoom > 0.0f ? cam->zoom : 1.0f;
    float rad = (float)(cam->rotation * DEG2RAD);
    float c = cosf(rad);
    float s = sinf(rad);
    float dx = (screen.x - cam->offset.x) / zoom;
    float dy = (screen.y - cam->offset.y) / zoom;
    gfx_vec2 out;
    out.x = cam->target.x + (c * dx + s * dy);
    out.y = cam->target.y + (-s * dx + c * dy);
    return out;
}

void gfx_draw_texture_pro(const gfx_texture *tex, gfx_rect src, gfx_rect dst, gfx_vec2 origin, float rotation, gfx_color tint)
{
    if (!tex || tex->id == 0) return;
    float mvp[16];
    get_mvp(mvp);
    glUseProgram(s_prog_tex);
    glUniform1i(glGetUniformLocation(s_prog_tex, "u_tex"), 0);
    glUniformMatrix4fv(glGetUniformLocation(s_prog_tex, "u_mvp"), 1, GL_FALSE, mvp);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    float tw = (float)tex->width;
    float th = (float)tex->height;
    /* Texel-center sampling to avoid gaps between adjacent tiles. */
    float u0 = (src.x + 0.5f) / tw;
    float v0 = (src.y + 0.5f) / th;
    float u1 = (src.x + src.w - 0.5f) / tw;
    float v1 = (src.y + src.h - 0.5f) / th;
    /* Pivot at (dst.x, dst.y); top-left = pivot + R*(-origin) to match raylib. */
    float cx = dst.x;
    float cy = dst.y;
    float c = cosf(rotation);
    float s = sinf(rotation);
    float dx0 = -origin.x;
    float dy0 = -origin.y;
    float dx1 = dst.w - origin.x;
    float dy1 = dst.h - origin.y;
    float x0 = cx + dx0 * c - dy0 * s;
    float y0 = cy + dx0 * s + dy0 * c;
    float x1 = cx + dx1 * c - dy0 * s;
    float y1 = cy + dx1 * s + dy0 * c;
    float x2 = cx + dx1 * c - dy1 * s;
    float y2 = cy + dx1 * s + dy1 * c;
    float x3 = cx + dx0 * c - dy1 * s;
    float y3 = cy + dx0 * s + dy1 * c;
    float r = clamp01(tint.r), g_ = clamp01(tint.g), b = clamp01(tint.b), a = clamp01(tint.a);
    draw_quad4(x0, y0, x1, y1, x2, y2, x3, y3, u0, v0, u1, v1, r, g_, b, a);
}

void gfx_draw_rect(gfx_rect r, gfx_color color)
{
    float mvp[16];
    get_mvp(mvp);
    glUseProgram(s_prog_tex);
    glUniform1i(glGetUniformLocation(s_prog_tex, "u_tex"), 0);
    glUniformMatrix4fv(glGetUniformLocation(s_prog_tex, "u_mvp"), 1, GL_FALSE, mvp);
    glBindTexture(GL_TEXTURE_2D, s_white_tex);
    float r_ = clamp01(color.r), g_ = clamp01(color.g), b_ = clamp01(color.b), a_ = clamp01(color.a);
    draw_quad(r.x, r.y, r.x + r.w, r.y + r.h, 0, 0, 1, 1, r_, g_, b_, a_);
}

void gfx_draw_rect_lines(gfx_rect r, gfx_color color)
{
    float t = 1.0f;
    float r_ = clamp01(color.r), g_ = clamp01(color.g), b_ = clamp01(color.b), a_ = clamp01(color.a);
    float mvp[16];
    get_mvp(mvp);
    glUseProgram(s_prog_tex);
    glUniform1i(glGetUniformLocation(s_prog_tex, "u_tex"), 0);
    glUniformMatrix4fv(glGetUniformLocation(s_prog_tex, "u_mvp"), 1, GL_FALSE, mvp);
    glBindTexture(GL_TEXTURE_2D, s_white_tex);
    draw_quad(r.x, r.y, r.x + r.w, r.y + t, 0, 0, 1, 1, r_, g_, b_, a_);
    draw_quad(r.x + r.w - t, r.y, r.x + r.w, r.y + r.h, 0, 0, 1, 1, r_, g_, b_, a_);
    draw_quad(r.x, r.y + r.h - t, r.x + r.w, r.y + r.h, 0, 0, 1, 1, r_, g_, b_, a_);
    draw_quad(r.x, r.y, r.x + t, r.y + r.h, 0, 0, 1, 1, r_, g_, b_, a_);
}

void gfx_draw_line(gfx_vec2 a, gfx_vec2 b, float thickness, gfx_color color)
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 1e-6f) return;
    float nx = -dy / len * thickness * 0.5f;
    float ny = dx / len * thickness * 0.5f;
    float r_ = clamp01(color.r), g_ = clamp01(color.g), b_ = clamp01(color.b), a_ = clamp01(color.a);
    float mvp[16];
    get_mvp(mvp);
    glUseProgram(s_prog_tex);
    glUniform1i(glGetUniformLocation(s_prog_tex, "u_tex"), 0);
    glUniformMatrix4fv(glGetUniformLocation(s_prog_tex, "u_mvp"), 1, GL_FALSE, mvp);
    glBindTexture(GL_TEXTURE_2D, s_white_tex);
    /* Quad corners (a+n), (b+n), (b-n), (a-n). */
    draw_quad4(
        a.x + nx, a.y + ny,
        b.x + nx, b.y + ny,
        b.x - nx, b.y - ny,
        a.x - nx, a.y - ny,
        0, 0, 1, 1, r_, g_, b_, a_);
}

static int fixed_char_width(int font_size)
{
    (void)font_size;
    return 8;
}

void gfx_draw_text(const char *text, int x, int y, int font_size, gfx_color color)
{
    if (!text) return;
    int cw = fixed_char_width(font_size);
    float r = clamp01(color.r), g = clamp01(color.g), b = clamp01(color.b), a = clamp01(color.a);
    float mvp[16];
    get_mvp(mvp);
    glUseProgram(s_prog_tex);
    glUniform1i(glGetUniformLocation(s_prog_tex, "u_tex"), 0);
    glUniformMatrix4fv(glGetUniformLocation(s_prog_tex, "u_mvp"), 1, GL_FALSE, mvp);
    glBindTexture(GL_TEXTURE_2D, s_white_tex);
    for (const char *p = text; *p; p++) {
        int px = x + (int)(p - text) * cw;
        draw_quad((float)px, (float)y, (float)(px + cw), (float)(y + font_size), 0, 0, 1, 1, r, g, b, a);
    }
}

int gfx_measure_text(const char *text, int font_size)
{
    if (!text) return 0;
    return (int)strlen(text) * fixed_char_width(font_size);
}

void gfx_texture_unload(gfx_texture *tex)
{
    if (!tex) return;
    if (tex->id) glDeleteTextures(1, &tex->id);
    free(tex);
}

bool gfx_texture_size(const gfx_texture *tex, int *out_w, int *out_h)
{
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    if (!tex) return false;
    if (out_w) *out_w = tex->width;
    if (out_h) *out_h = tex->height;
    return true;
}

void gfx_texture_debug_info(const gfx_texture *tex, gfx_texture_info *out)
{
    if (!out) return;
    if (!tex) {
        out->id = 0;
        out->width = 0;
        out->height = 0;
        return;
    }
    out->id = (uint32_t)tex->id;
    out->width = tex->width;
    out->height = tex->height;
}

gfx_texture *gfx_texture_create_rgba8(int width, int height, const unsigned char *pixels)
{
    if (width <= 0 || height <= 0 || !pixels) return NULL;
    gfx_texture *tex = (gfx_texture *)malloc(sizeof(*tex));
    if (!tex) return NULL;
    glGenTextures(1, &tex->id);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    tex->width = width;
    tex->height = height;
    return tex;
}

bool gfx_texture_update_rgba8(gfx_texture *tex, int width, int height, const unsigned char *pixels)
{
    if (!tex || !pixels || width <= 0 || height <= 0) return false;
    glBindTexture(GL_TEXTURE_2D, tex->id);
    if (width == tex->width && height == tex->height) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        tex->width = width;
        tex->height = height;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}
