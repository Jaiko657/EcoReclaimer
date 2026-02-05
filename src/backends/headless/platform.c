#include "engine/core/platform/platform.h"
#include "engine/core/logger/logger.h"

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

struct platform_window {
    int width;
    int height;
};

static platform_window g_window = {0};
static void log_working_dir(void)
{
    char buf[PATH_MAX];
    if (getcwd(buf, sizeof(buf))) {
        LOGC(LOGCAT_MAIN, LOG_LVL_INFO, "SYSTEM: Working Directory: %s", buf);
    }
}

void platform_init(void)
{
    // Normalize CWD so relative paths like "assets/..." work when running build/src/game_headless.
    char exe[PATH_MAX];
    ssize_t n = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
    if (n > 0) {
        exe[n] = '\0';
        char* last_slash = strrchr(exe, '/');
        if (last_slash) {
            *last_slash = '\0';
            (void)chdir(exe);
        }
    }

    log_working_dir();
}

platform_window* platform_window_create(int width, int height, const char* title, int target_fps)
{
    (void)title;
    (void)target_fps;
    g_window.width = width > 0 ? width : 0;
    g_window.height = height > 0 ? height : 0;
    return &g_window;
}

void platform_window_destroy(platform_window* window)
{
    (void)window;
}

bool platform_window_ready(const platform_window* window)
{
    return window != NULL;
}

bool platform_window_should_close(const platform_window* window)
{
    static int frames = 0;
    static int max_frames = -1;
    (void)window;

    if (max_frames < 0) {
        const char* env = getenv("HEADLESS_MAX_TICKS");
        max_frames = env ? atoi(env) : 600;
        if (max_frames <= 0) max_frames = 1;
    }

    return (frames++ >= max_frames);
}

bool platform_should_close(void)
{
    return platform_window_should_close(&g_window);
}

int platform_window_width(const platform_window* window)
{
    return window ? window->width : 0;
}

int platform_window_height(const platform_window* window)
{
    return window ? window->height : 0;
}

void platform_poll_events(void)
{
}

bool platform_dir_exists(const char* path)
{
    if (!path) return false;
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

bool platform_make_dir(const char* path)
{
    if (!path) return false;
    return mkdir(path, 0755) == 0 || errno == EEXIST;
}

bool platform_file_exists(const char* path)
{
    if (!path) return false;
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

bool platform_take_screenshot(const char* path)
{
    (void)path;
    return false;
}
