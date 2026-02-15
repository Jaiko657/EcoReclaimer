#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "engine/core/platform/platform.h"
#include "engine/core/logger/logger.h"
#include "opengl_ctx.h"

#include <GLFW/glfw3.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

struct platform_window {
    GLFWwindow *handle;
    int width;
    int height;
};

static platform_window *g_window_ptr = NULL;

void platform_init(void)
{
    if (!glfwInit()) {
        LOGC(LOGCAT_MAIN, LOG_LVL_FATAL, "platform: glfwInit failed");
        return;
    }
#if defined(__linux__)
    char exe[PATH_MAX];
    ssize_t n = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
    if (n > 0) {
        exe[n] = '\0';
        char *last_slash = strrchr(exe, '/');
        if (last_slash) {
            *last_slash = '\0';
            (void)chdir(exe);
        }
    }
#elif defined(_WIN32)
    /* TODO: set CWD from executable dir on Windows */
#endif
}

platform_window *platform_window_create(int width, int height, const char *title, int target_fps)
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); /* Match raylib: fixed-size window by default */
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *win = glfwCreateWindow(
        width > 0 ? (int)(size_t)width : 1280,
        height > 0 ? (int)(size_t)height : 720,
        title && title[0] ? title : "Game",
        NULL,
        NULL);
    if (!win) {
        LOGC(LOGCAT_MAIN, LOG_LVL_FATAL, "platform: glfwCreateWindow failed");
        return NULL;
    }

    glfwMakeContextCurrent(win);
    if (target_fps > 0) {
        glfwSwapInterval(1);
    } else {
        glfwSwapInterval(0);
    }

    platform_window *w = (platform_window *)malloc(sizeof(platform_window));
    if (!w) {
        glfwDestroyWindow(win);
        return NULL;
    }
    w->handle = win;
    glfwGetFramebufferSize(win, &w->width, &w->height);
    g_window_ptr = w;
    opengl_ctx_set_window(win);
    return w;
}

void platform_window_destroy(platform_window *window)
{
    if (!window) return;
    if (window->handle) {
        glfwDestroyWindow(window->handle);
        window->handle = NULL;
    }
    if (g_window_ptr == window) g_window_ptr = NULL;
    free(window);
}

bool platform_window_ready(const platform_window *window)
{
    return window != NULL && window->handle != NULL;
}

bool platform_window_should_close(const platform_window *window)
{
    if (!window || !window->handle) return true;
    return glfwWindowShouldClose(window->handle) != 0;
}

int platform_window_width(const platform_window *window)
{
    if (!window || !window->handle) return 0;
    int w = 0;
    glfwGetFramebufferSize(window->handle, &w, NULL);
    return w;
}

int platform_window_height(const platform_window *window)
{
    if (!window || !window->handle) return 0;
    int h = 0;
    glfwGetFramebufferSize(window->handle, NULL, &h);
    return h;
}

void platform_poll_events(void)
{
    glfwPollEvents();
}

bool platform_should_close(void)
{
    return g_window_ptr ? platform_window_should_close(g_window_ptr) : true;
}

bool platform_dir_exists(const char *path)
{
    if (!path) return false;
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

bool platform_make_dir(const char *path)
{
    if (!path) return false;
    return mkdir(path, 0755) == 0 || errno == EEXIST;
}

bool platform_file_exists(const char *path)
{
    if (!path) return false;
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

bool platform_take_screenshot(const char *path)
{
    if (!path || !g_window_ptr || !g_window_ptr->handle) return false;
    /* Screenshot requires GL context; handled in gfx backend. */
    (void)path;
    return false;
}
