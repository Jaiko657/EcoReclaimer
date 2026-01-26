#pragma once
#include <stdbool.h>

typedef struct platform_window platform_window;

// Headless builds may need to normalize the working directory for relative asset paths.
void platform_init(void);

// Window lifecycle.
platform_window* platform_window_create(int width, int height, const char* title, int target_fps);
void platform_window_destroy(platform_window* window);
bool platform_window_ready(const platform_window* window);
bool platform_window_should_close(const platform_window* window);
int platform_window_width(const platform_window* window);
int platform_window_height(const platform_window* window);
void platform_poll_events(void);
bool platform_should_close(void);

// Filesystem helpers.
bool platform_dir_exists(const char* path);
bool platform_make_dir(const char* path);
bool platform_file_exists(const char* path);

// Screenshot capture (path should be a writable file).
bool platform_take_screenshot(const char* path);
