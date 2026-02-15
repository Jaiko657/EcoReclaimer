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

typedef enum {
    BACKEND_RAYLIB,
    BACKEND_HEADLESS,
    BACKEND_OPENGL,
} build_backend_t;

typedef struct {
    build_backend_t id;
    const char *name;
    const char *source_root;
    const char *target_base;
    bool force_debug;
    bool define_headless;
    const char *inputs_ini_filename;
    const char **include_dirs;
    size_t include_dir_count;
    const char **link_flags;
    size_t link_flag_count;
} backend_config_t;

static const char *REQUIRED_BACKEND_FILES[] = {
    "gfx.c",
    "input.c",
    "input_tables.c",
    "platform.c",
    "time.c",
    "logger_backend.c",
};

static const char *RAYLIB_INCLUDE_DIRS[] = {
    RAYLIB_INCLUDE_DIR,
    XML_INCLUDE_DIR,
    "third_party/stb",
    "src",
};

static const char *HEADLESS_INCLUDE_DIRS[] = {
    XML_INCLUDE_DIR,
    "third_party/stb",
    "src",
};

#if defined(_WIN32)
#define GLFW_INCLUDE_DIR "third_party/glfw/include"
#define GLFW_LIB_DIR "third_party/glfw/build/src"
#else
/* System GLFW (e.g. libglfw3-dev on Debian/Ubuntu) */
#define GLFW_INCLUDE_DIR ""
#define GLFW_LIB_DIR ""
#endif

/* Use raylib's bundled GLFW for headers when building opengl backend (link with -lglfw). */
#define GLFW_INCLUDE_PATH "third_party/raylib/src/external/glfw/include"

static const char *OPENGL_INCLUDE_DIRS[] = {
    GLFW_INCLUDE_PATH,
    XML_INCLUDE_DIR,
    "third_party/stb",
    "src",
};

#if defined(_WIN32)
static const char *RAYLIB_LINK_FLAGS[] = {
    "-L", RAYLIB_LIB_DIR,
    "-lraylib",
    "-lgdi32",
    "-lwinmm",
};

static const char *HEADLESS_LINK_FLAGS[] = {
};
#else
static const char *RAYLIB_LINK_FLAGS[] = {
    "-L", RAYLIB_LIB_DIR,
    "-lraylib",
    "-lGL",
    "-lm",
    "-lpthread",
    "-ldl",
    "-lrt",
    "-lX11",
};

static const char *HEADLESS_LINK_FLAGS[] = {
    "-lm",
    "-lpthread",
    "-ldl",
    "-lrt",
};

#if defined(_WIN32)
static const char *OPENGL_LINK_FLAGS[] = {
    "-L", GLFW_LIB_DIR,
    "-lglfw3",
    "-lopengl32",
};
#else
static const char *OPENGL_LINK_FLAGS[] = {
    "-lglfw",
    "-lGL",
    "-lm",
    "-lpthread",
    "-ldl",
    "-lrt",
    "-lX11",
};
#endif
#endif

static const backend_config_t BACKEND_CONFIGS[] = {
    {
        .id = BACKEND_RAYLIB,
        .name = "raylib",
        .source_root = "src/backends/raylib",
        .target_base = "game",
        .force_debug = false,
        .define_headless = false,
        .inputs_ini_filename = "inputs_raylib.ini",
        .include_dirs = RAYLIB_INCLUDE_DIRS,
        .include_dir_count = sizeof(RAYLIB_INCLUDE_DIRS) / sizeof(RAYLIB_INCLUDE_DIRS[0]),
        .link_flags = RAYLIB_LINK_FLAGS,
        .link_flag_count = sizeof(RAYLIB_LINK_FLAGS) / sizeof(RAYLIB_LINK_FLAGS[0]),
    },
    {
        .id = BACKEND_HEADLESS,
        .name = "headless",
        .source_root = "src/backends/headless",
        .target_base = "game_headless",
        .force_debug = true,
        .define_headless = true,
        .inputs_ini_filename = "inputs_headless.ini",
        .include_dirs = HEADLESS_INCLUDE_DIRS,
        .include_dir_count = sizeof(HEADLESS_INCLUDE_DIRS) / sizeof(HEADLESS_INCLUDE_DIRS[0]),
        .link_flags = HEADLESS_LINK_FLAGS,
        .link_flag_count = sizeof(HEADLESS_LINK_FLAGS) / sizeof(HEADLESS_LINK_FLAGS[0]),
    },
    {
        .id = BACKEND_OPENGL,
        .name = "opengl",
        .source_root = "src/backends/opengl",
        .target_base = "game_gl",
        .force_debug = false,
        .define_headless = false,
        .inputs_ini_filename = "inputs_opengl.ini",
        .include_dirs = OPENGL_INCLUDE_DIRS,
        .include_dir_count = sizeof(OPENGL_INCLUDE_DIRS) / sizeof(OPENGL_INCLUDE_DIRS[0]),
        .link_flags = OPENGL_LINK_FLAGS,
        .link_flag_count = sizeof(OPENGL_LINK_FLAGS) / sizeof(OPENGL_LINK_FLAGS[0]),
    },
};

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

static bool is_legacy_backend_file(const char *name)
{
    return cstr_ends_with(name, "_raylib.c")
        || cstr_ends_with(name, "_headless.c")
        || cstr_ends_with(name, "_sdl.c");
}

static void cmd_append_paths(Nob_Cmd *cmd, const Nob_File_Paths *paths)
{
    for (size_t i = 0; i < paths->count; ++i) {
        nob_cmd_append(cmd, paths->items[i]);
    }
}

static void cmd_append_cstr_array(Nob_Cmd *cmd, const char **items, size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        nob_cmd_append(cmd, items[i]);
    }
}

static void cmd_append_flags(Nob_Cmd *cmd, bool debug_build)
{
    const char *debug_flags[] = {
        "-DDEBUG_BUILD=1",
        "-g",
    };
    const char *release_flags[] = {
        "-DDEBUG_BUILD=0",
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

static bool gather_backend_sources_recursive(const char *parent, Nob_File_Paths *out_sources)
{
    Nob_File_Paths children = {0};
    if (!nob_read_entire_dir(parent, &children)) return false;

    for (size_t i = 0; i < children.count; ++i) {
        const char *name = children.items[i];
        if (is_dot_entry(name)) continue;

        const char *full = nob_temp_sprintf("%s/%s", parent, name);
        Nob_File_Type type = nob_get_file_type(full);

        if (type == NOB_FILE_DIRECTORY) {
            if (!gather_backend_sources_recursive(full, out_sources)) return false;
            continue;
        }

        if (type != NOB_FILE_REGULAR) continue;
        if (!cstr_ends_with(name, ".c")) continue;

        nob_da_append(out_sources, full);
    }

    return true;
}

static bool gather_common_sources_recursive(const char *parent, Nob_File_Paths *out_sources)
{
    Nob_File_Paths children = {0};
    if (!nob_read_entire_dir(parent, &children)) return false;

    for (size_t i = 0; i < children.count; ++i) {
        const char *name = children.items[i];
        if (is_dot_entry(name)) continue;

        const char *full = nob_temp_sprintf("%s/%s", parent, name);
        Nob_File_Type type = nob_get_file_type(full);
        if (type == NOB_FILE_DIRECTORY) {
            if (strcmp(full, "src/backends") == 0) continue;
            if (!gather_common_sources_recursive(full, out_sources)) return false;
            continue;
        }

        if (type != NOB_FILE_REGULAR) continue;
        if (!cstr_ends_with(name, ".c")) continue;
        if (is_legacy_backend_file(name)) continue;

        nob_da_append(out_sources, full);
    }

    return true;
}

static bool gather_common_sources(Nob_File_Paths *out_sources)
{
    if (!gather_common_sources_recursive("src/engine", out_sources)) return false;
    if (!gather_common_sources_recursive("src/game", out_sources)) return false;
    if (!gather_common_sources_recursive("src/shared", out_sources)) return false;
    return true;
}

static bool validate_backend_modules(const backend_config_t *cfg)
{
    bool ok = true;
    for (size_t i = 0; i < sizeof(REQUIRED_BACKEND_FILES) / sizeof(REQUIRED_BACKEND_FILES[0]); ++i) {
        const char *name = REQUIRED_BACKEND_FILES[i];
        const char *path = nob_temp_sprintf("%s/%s", cfg->source_root, name);
        if (nob_get_file_type(path) != NOB_FILE_REGULAR) {
            if (ok) {
                nob_log(NOB_ERROR, "backend '%s' is missing required module files:", cfg->name);
            }
            nob_log(NOB_ERROR, "  - %s", path);
            ok = false;
        }
    }
    return ok;
}

static const backend_config_t *find_backend_config(build_backend_t id)
{
    for (size_t i = 0; i < sizeof(BACKEND_CONFIGS) / sizeof(BACKEND_CONFIGS[0]); ++i) {
        if (BACKEND_CONFIGS[i].id == id) return &BACKEND_CONFIGS[i];
    }
    return NULL;
}

static bool build_backend_target(build_backend_t backend, bool debug_build)
{
    const backend_config_t *cfg = find_backend_config(backend);
    if (!cfg) {
        nob_log(NOB_ERROR, "unknown backend id: %d", (int)backend);
        return false;
    }

    if (!validate_backend_modules(cfg)) return false;

    Nob_File_Paths backend_sources = {0};
    Nob_File_Paths common_sources = {0};

    if (!gather_backend_sources_recursive(cfg->source_root, &backend_sources)) return false;
    if (!gather_common_sources(&common_sources)) return false;

    bool effective_debug = cfg->force_debug ? true : debug_build;
    const char *target_name = cfg->force_debug
        ? cfg->target_base
        : (debug_build ? nob_temp_sprintf("%s_debug", cfg->target_base) : cfg->target_base);

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "gcc");
    nob_cmd_append(&cmd, "-std=c99", "-Wall", "-Wextra", "-fno-fast-math", "-fno-finite-math-only");
    cmd_append_flags(&cmd, effective_debug);
    if (cfg->define_headless) {
        nob_cmd_append(&cmd, "-DHEADLESS=1");
    }
    if (cfg->id == BACKEND_OPENGL) {
        nob_cmd_append(&cmd, "-DGLFW_INCLUDE_NONE");
    }
    nob_cmd_append(&cmd, nob_temp_sprintf("-DINPUTS_INI_FILENAME=\"%s\"", cfg->inputs_ini_filename));

    for (size_t i = 0; i < cfg->include_dir_count; ++i) {
        nob_cmd_append(&cmd, "-I", cfg->include_dirs[i]);
    }

    nob_cmd_append(&cmd, "src/main.c");
    cmd_append_paths(&cmd, &common_sources);
    cmd_append_paths(&cmd, &backend_sources);

    nob_cmd_append(&cmd, "-o", target_path(target_name));
    cmd_append_cstr_array(&cmd, cfg->link_flags, cfg->link_flag_count);
    nob_cmd_append(&cmd, "-L", XML_LIB_DIR, "-l:" XML_LIB_NAME);

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

static bool ensure_doxyfile(void)
{
    static const char *default_doxyfile =
        "PROJECT_NAME = \"Eco Reclaimer\"\n"
        "OUTPUT_DIRECTORY = docs\n"
        "INPUT = src\n"
        "RECURSIVE = YES\n"
        "EXTRACT_ALL = YES\n"
        "GENERATE_HTML = YES\n"
        "HTML_OUTPUT = html\n"
        "GENERATE_XML = YES\n"
        "XML_OUTPUT = xml\n"
        "GENERATE_LATEX = NO\n"
        "HAVE_DOT = YES\n"
        "DOT_IMAGE_FORMAT = svg\n"
        "INTERACTIVE_SVG = YES\n";

    if (nob_file_exists("Doxyfile")) {
        return true;
    }

    return nob_write_entire_file("Doxyfile", default_doxyfile, strlen(default_doxyfile));
}

static bool build_docs(void)
{
    if (!ensure_doxyfile()) return false;

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "doxygen", "Doxyfile");
    return nob_cmd_run_sync_and_reset(&cmd);
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    bool do_headless = false;
    bool do_sdl = false;
    bool do_opengl = false;
    bool debug_build = false;
    bool do_docs = false;

    for (int i = 1; i < argc; ++i) {
        Nob_String_View arg = nob_sv_from_cstr(argv[i]);
        if (nob_sv_eq(arg, nob_sv_from_cstr("--headless"))) {
            do_headless = true;
        } else if (nob_sv_eq(arg, nob_sv_from_cstr("--opengl"))) {
            do_opengl = true;
        } else if (nob_sv_eq(arg, nob_sv_from_cstr("--sdl"))) {
            do_sdl = true;
        } else if (nob_sv_eq(arg, nob_sv_from_cstr("--docs"))) {
            do_docs = true;
        } else if (nob_sv_eq(arg, nob_sv_from_cstr("--debug"))) {
            debug_build = true;
        } else if (nob_sv_eq(arg, nob_sv_from_cstr("--release"))) {
            debug_build = false;
        }
    }

    if (do_sdl) {
        nob_log(NOB_ERROR, "--sdl is not supported in the backend-directory build. Available: default(raylib), --headless, --opengl");
        return 1;
    }
    if (do_headless && do_opengl) {
        nob_log(NOB_ERROR, "cannot use both --headless and --opengl");
        return 1;
    }

    if (do_docs) {
        if (!build_docs()) return 1;
        return 0;
    }

    if (!nob_mkdir_if_not_exists("build")) return 1;
    if (!nob_mkdir_if_not_exists("build/src")) return 1;
    if (!copy_assets_to_build()) return 1;

    build_backend_t backend = BACKEND_RAYLIB;
    if (do_headless) backend = BACKEND_HEADLESS;
    else if (do_opengl) backend = BACKEND_OPENGL;
    if (!build_backend_target(backend, debug_build)) return 1;

    return 0;
}
