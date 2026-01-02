#define NOB_IMPLEMENTATION
#include "../third_party/nob.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define XML_INCLUDE_DIR "third_party/xml.c/src"
#define XML_LIB_DIR "third_party/xml.c/build"
#define XML_LIB_NAME "libxml.a"

#if defined(_WIN32)
#define RAYLIB_INCLUDE_DIR "third_party/raylib/build/raylib-5.5_win64_mingw-w64/include"
#define RAYLIB_LIB_DIR "third_party/raylib/build/raylib-5.5_win64_mingw-w64/lib"
#define EXE_EXT ".exe"
#else
#define RAYLIB_INCLUDE_DIR "third_party/raylib/src"
#define RAYLIB_LIB_DIR "third_party/raylib/build/raylib"
#define EXE_EXT ""
#endif

static bool cstr_ends_with(const char *s, const char *suffix)
{
    if (!s || !suffix) return false;
    size_t n = strlen(s);
    size_t m = strlen(suffix);
    if (m > n) return false;
    return memcmp(s + (n - m), suffix, m) == 0;
}

static bool is_dot_entry(const char *name)
{
    return name && (strcmp(name, ".") == 0 || strcmp(name, "..") == 0);
}

static bool list_contains_cstr(const Nob_File_Paths *xs, const char *s)
{
    if (!xs || !s) return false;
    for (size_t i = 0; i < xs->count; ++i) {
        if (xs->items[i] && strcmp(xs->items[i], s) == 0) return true;
    }
    return false;
}

static bool gather_headless_sources(Nob_File_Paths *out_sources, Nob_File_Paths *out_replacement_bases)
{
    Nob_File_Paths children = {0};
    if (!nob_read_entire_dir("src/headless", &children)) return false;

    for (size_t i = 0; i < children.count; ++i) {
        const char *name = children.items[i];
        if (is_dot_entry(name)) continue;
        if (!cstr_ends_with(name, ".c")) continue;

        const char *full = nob_temp_sprintf("src/headless/%s", name);
        nob_da_append(out_sources, full);

        if (cstr_ends_with(name, "_headless.c")) {
            size_t n = strlen(name);
            size_t suffix = strlen("_headless.c");
            size_t base_len = (n > suffix) ? (n - suffix) : 0;
            if (base_len == 0) continue;
            const char *base = nob_temp_sprintf("%.*s", (int)base_len, name);
            if (!list_contains_cstr(out_replacement_bases, base)) {
                nob_da_append(out_replacement_bases, base);
            }
        }
    }

    return true;
}

static bool gather_module_sources_recursive(const char *parent, const Nob_File_Paths *replacement_bases, Nob_File_Paths *out_sources)
{
    Nob_File_Paths children = {0};
    if (!nob_read_entire_dir(parent, &children)) return false;

    for (size_t i = 0; i < children.count; ++i) {
        const char *name = children.items[i];
        if (is_dot_entry(name)) continue;

        const char *full = nob_temp_sprintf("%s/%s", parent, name);
        Nob_File_Type type = nob_get_file_type(full);
        if (type == NOB_FILE_DIRECTORY) {
            if (!gather_module_sources_recursive(full, replacement_bases, out_sources)) return false;
            continue;
        }

        if (type != NOB_FILE_REGULAR) continue;
        if (!cstr_ends_with(name, ".c")) continue;

        size_t n = strlen(name);
        size_t ext = strlen(".c");
        size_t base_len = (n > ext) ? (n - ext) : n;
        const char *base = nob_temp_sprintf("%.*s", (int)base_len, name);
        if (replacement_bases && list_contains_cstr(replacement_bases, base)) continue;

        nob_da_append(out_sources, full);
    }

    return true;
}

static bool gather_module_sources(Nob_File_Paths *out_sources, const Nob_File_Paths *replacement_bases)
{
    return gather_module_sources_recursive("src/modules", replacement_bases, out_sources);
}

static void cmd_append_paths(Nob_Cmd *cmd, const Nob_File_Paths *paths)
{
    for (size_t i = 0; i < paths->count; ++i) {
        nob_cmd_append(cmd, paths->items[i]);
    }
}

static void cmd_append_flags(Nob_Cmd *cmd, bool debug_build)
{
    const char *debug_flags[] = {
        "-DDEBUG_BUILD=1",
        "-DDEBUG_COLLISION=1",
        "-DDEBUG_TRIGGERS=1",
        "-DDEBUG_FPS=1",
        "-g",
    };
    const char *release_flags[] = {
        "-DDEBUG_BUILD=0",
        "-DDEBUG_COLLISION=0",
        "-DDEBUG_TRIGGERS=0",
        "-DDEBUG_FPS=0",
        "-DNDEBUG",
    };

    const char **flags = debug_build ? debug_flags : release_flags;
    size_t count = debug_build
        ? sizeof(debug_flags) / sizeof(debug_flags[0])
        : sizeof(release_flags) / sizeof(release_flags[0]);

    for (size_t i = 0; i < count; ++i) {
        nob_cmd_append(cmd, flags[i]);
    }
}

static const char *target_path(const char *name)
{
    return nob_temp_sprintf("build/src/%s%s", name, EXE_EXT);
}

static bool build_game(bool debug_build)
{
    Nob_File_Paths module_sources = {0};
    if (!gather_module_sources(&module_sources, NULL)) return false;

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "gcc");
    nob_cmd_append(&cmd, "-std=c99", "-Wall", "-Wextra", "-fno-fast-math", "-fno-finite-math-only");
    cmd_append_flags(&cmd, debug_build);
    nob_cmd_append(&cmd, "-I", RAYLIB_INCLUDE_DIR, "-I", XML_INCLUDE_DIR, "-I", "src");
    nob_cmd_append(&cmd, "src/main.c");
    cmd_append_paths(&cmd, &module_sources);
    nob_cmd_append(&cmd, "-o", target_path(debug_build ? "game_debug" : "game"));
    nob_cmd_append(&cmd, "-L", RAYLIB_LIB_DIR, "-lraylib");
#if defined(_WIN32)
    nob_cmd_append(&cmd, "-lgdi32", "-lwinmm");
#else
    nob_cmd_append(&cmd, "-lGL", "-lm", "-lpthread", "-ldl", "-lrt", "-lX11");
#endif
    nob_cmd_append(&cmd, "-L", XML_LIB_DIR, "-l:" XML_LIB_NAME);

    return nob_cmd_run_sync_and_reset(&cmd);
}

static bool build_headless(void)
{
    Nob_File_Paths headless_sources = {0};
    Nob_File_Paths replacement_bases = {0};
    Nob_File_Paths module_sources = {0};
    if (!gather_headless_sources(&headless_sources, &replacement_bases)) return false;
    if (!gather_module_sources(&module_sources, &replacement_bases)) return false;

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "gcc");
    nob_cmd_append(&cmd, "-std=c99", "-Wall", "-Wextra", "-fno-fast-math", "-fno-finite-math-only");
    cmd_append_flags(&cmd, true);
    nob_cmd_append(&cmd, "-DHEADLESS=1");
    nob_cmd_append(&cmd, "-I", XML_INCLUDE_DIR, "-I", "src");
    nob_cmd_append(&cmd, "src/main.c");
    cmd_append_paths(&cmd, &module_sources);
    cmd_append_paths(&cmd, &headless_sources);
    nob_cmd_append(&cmd, "-o", target_path("game_headless"));
    nob_cmd_append(&cmd, "-L", XML_LIB_DIR, "-l:" XML_LIB_NAME);
#if !defined(_WIN32)
    nob_cmd_append(&cmd, "-lm", "-lpthread", "-ldl", "-lrt");
#endif

    return nob_cmd_run_sync_and_reset(&cmd);
}

static bool copy_dir_recursive(const char *src_dir, const char *dst_dir)
{
    if (!nob_mkdir_if_not_exists(dst_dir)) return false;

    Nob_File_Paths children = {0};
    if (!nob_read_entire_dir(src_dir, &children)) return false;

    for (size_t i = 0; i < children.count; ++i) {
        const char *name = children.items[i];
        if (is_dot_entry(name)) continue;

        const char *src_path = nob_temp_sprintf("%s/%s", src_dir, name);
        const char *dst_path = nob_temp_sprintf("%s/%s", dst_dir, name);
        Nob_File_Type type = nob_get_file_type(src_path);

        if (type == NOB_FILE_DIRECTORY) {
            if (!copy_dir_recursive(src_path, dst_path)) return false;
        } else if (type == NOB_FILE_REGULAR) {
            if (!nob_copy_file(src_path, dst_path)) return false;
        } else {
            nob_log(NOB_ERROR, "unsupported asset entry type for %s", src_path);
            return false;
        }
    }

    return true;
}

static bool copy_assets_to_build(void)
{
    if (!nob_file_exists("assets")) {
        return true;
    }

    Nob_File_Type type = nob_get_file_type("assets");
    if (type != NOB_FILE_DIRECTORY) {
        nob_log(NOB_ERROR, "assets exists but is not a directory");
        return false;
    }

    if (!nob_mkdir_if_not_exists("build/src/assets")) return false;
    return copy_dir_recursive("assets", "build/src/assets");
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    bool do_headless = false;
    bool debug_build = false;

    for (int i = 1; i < argc; ++i) {
        Nob_String_View arg = nob_sv_from_cstr(argv[i]);
        if (nob_sv_eq(arg, nob_sv_from_cstr("--headless"))) {
            do_headless = true;
            debug_build = true;
        } else if (nob_sv_eq(arg, nob_sv_from_cstr("--debug"))) {
            debug_build = true;
        } else if (nob_sv_eq(arg, nob_sv_from_cstr("--release"))) {
            debug_build = false;
        }
    }

    if (!nob_mkdir_if_not_exists("build")) return 1;
    if (!nob_mkdir_if_not_exists("build/src")) return 1;
    if (!copy_assets_to_build()) return 1;

    if (do_headless) {
        if (!build_headless()) return 1;
    } else {
        if (!build_game(debug_build)) return 1;
    }

    return 0;
}
