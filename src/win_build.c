#define NOB_IMPLEMENTATION
#include "../third_party/nob.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define RAYLIB_INCLUDE_DIR "third_party/raylib/build/raylib-5.5_win64_mingw-w64/include"
#define RAYLIB_LIB_DIR "third_party/raylib/build/raylib-5.5_win64_mingw-w64/lib"

#define XML_INCLUDE_DIR "third_party/xml.c/src"
#define XML_LIB_DIR "third_party/xml.c/build"
#define XML_LIB_NAME "libxml.a"

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

static bool gather_module_sources_recursive(const char *parent, Nob_File_Paths *out_sources)
{
    Nob_File_Paths children = {0};
    if (!nob_read_entire_dir(parent, &children)) return false;

    for (size_t i = 0; i < children.count; ++i) {
        const char *name = children.items[i];
        if (is_dot_entry(name)) continue;

        const char *full = nob_temp_sprintf("%s/%s", parent, name);
        Nob_File_Type type = nob_get_file_type(full);
        if (type == NOB_FILE_DIRECTORY) {
            if (!gather_module_sources_recursive(full, out_sources)) return false;
            continue;
        }

        if (type != NOB_FILE_REGULAR) continue;
        if (!cstr_ends_with(name, ".c")) continue;

        nob_da_append(out_sources, full);
    }

    return true;
}

static bool gather_module_sources(Nob_File_Paths *out_sources)
{
    return gather_module_sources_recursive("src/modules", out_sources);
}

static void cmd_append_paths(Nob_Cmd *cmd, const Nob_File_Paths *paths)
{
    for (size_t i = 0; i < paths->count; ++i) {
        nob_cmd_append(cmd, paths->items[i]);
    }
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    bool debug_build = false;
    for (int i = 1; i < argc; ++i) {
        if (nob_sv_eq(nob_sv_from_cstr(argv[i]), nob_sv_from_cstr("--debug"))) debug_build = true;
        if (nob_sv_eq(nob_sv_from_cstr(argv[i]), nob_sv_from_cstr("--release"))) debug_build = false;
    }

    if (!nob_mkdir_if_not_exists("build")) return 1;
    if (!nob_mkdir_if_not_exists("build/src")) return 1;
    nob_log(NOB_INFO, "Mode: %s", debug_build ? "debug" : "release");

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

    const char *xml_lib_file = nob_temp_sprintf("%s/%s", XML_LIB_DIR, XML_LIB_NAME);
    nob_log(NOB_INFO, "Linking against %s", xml_lib_file);

    Nob_File_Paths module_sources = {0};
    if (!gather_module_sources(&module_sources)) return 1;

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "gcc");
    nob_cmd_append(&cmd, "-std=c99", "-Wall", "-Wextra", "-fno-fast-math", "-fno-finite-math-only");
    if (debug_build) {
        for (size_t i = 0; i < sizeof(debug_flags) / sizeof(debug_flags[0]); ++i) {
            nob_cmd_append(&cmd, debug_flags[i]);
        }
    } else {
        for (size_t i = 0; i < sizeof(release_flags) / sizeof(release_flags[0]); ++i) {
            nob_cmd_append(&cmd, release_flags[i]);
        }
    }
    nob_cmd_append(&cmd, "-I", RAYLIB_INCLUDE_DIR, "-I", XML_INCLUDE_DIR, "-I", "src");
    nob_cmd_append(&cmd, "src/main.c");
    cmd_append_paths(&cmd, &module_sources);
    nob_cmd_append(&cmd, "-o", "build/src/game.exe");
    nob_cmd_append(&cmd, "-L", RAYLIB_LIB_DIR, "-lraylib", "-lgdi32", "-lwinmm");
    nob_cmd_append(&cmd, "-L", XML_LIB_DIR, "-l:libxml.a");

    if (!nob_cmd_run_sync_and_reset(&cmd)) return 1;

    return 0;
}
