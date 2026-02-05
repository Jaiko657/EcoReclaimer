#include "engine/core/platform/platform.h"

#include "raylib.h"
#include <stddef.h>

struct platform_window {
    int width;
    int height;
};

static platform_window g_window = {0};

void platform_init(void)
{
    ChangeDirectory(GetApplicationDirectory());
}

platform_window* platform_window_create(int width, int height, const char* title, int target_fps)
{
    InitWindow(width, height, title ? title : "Game");
    if (!IsWindowReady()) {
        CloseWindow();
        return NULL;
    }

    g_window.width = width;
    g_window.height = height;
    SetTargetFPS(target_fps >= 0 ? target_fps : 60);
    return &g_window;
}

void platform_window_destroy(platform_window* window)
{
    (void)window;
    if (IsWindowReady()) CloseWindow();
}

bool platform_window_ready(const platform_window* window)
{
    (void)window;
    return IsWindowReady();
}

bool platform_window_should_close(const platform_window* window)
{
    (void)window;
    return WindowShouldClose();
}

int platform_window_width(const platform_window* window)
{
    (void)window;
    return GetScreenWidth();
}

int platform_window_height(const platform_window* window)
{
    (void)window;
    return GetScreenHeight();
}

void platform_poll_events(void)
{
    PollInputEvents();
}

bool platform_should_close(void)
{
    return WindowShouldClose();
}

bool platform_dir_exists(const char* path)
{
    return path && DirectoryExists(path);
}

bool platform_make_dir(const char* path)
{
    return path && MakeDirectory(path);
}

bool platform_file_exists(const char* path)
{
    return path && FileExists(path);
}

bool platform_take_screenshot(const char* path)
{
    if (!path) return false;
    TakeScreenshot(path);
    return true;
}
