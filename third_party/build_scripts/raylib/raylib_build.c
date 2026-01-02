#define NOB_IMPLEMENTATION
#include "nob.h"

static bool run_cmd(Nob_Cmd *cmd)
{
    bool ok = nob_cmd_run(cmd);
    cmd->count = 0;
    return ok;
}

static bool ensure_raylib_dir(void)
{
    if (nob_file_exists("CMakeLists.txt")) return true;
    if (nob_file_exists("raylib/CMakeLists.txt")) {
        return nob_set_current_dir("raylib");
    }
    nob_log(NOB_ERROR, "Run from raylib dir or its parent.");
    return false;
}

#if defined(_WIN32)
static bool build_raylib_windows(void)
{
    const char *zip_name = "raylib-5.5_win64_mingw-w64.zip";
    const char *unzip_dir = "raylib-5.5_win64_mingw-w64";
    const char *url = "https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_win64_mingw-w64.zip";

    if (!nob_mkdir_if_not_exists("build")) return false;

    const char *start_dir = nob_get_current_dir_temp();
    if (!nob_set_current_dir("build")) return false;

    Nob_Cmd cmd = {0};
    if (!nob_file_exists(zip_name)) {
        nob_log(NOB_INFO, "Downloading raylib release...");
        nob_cmd_append(&cmd, "curl", "-LO", url);
        if (!run_cmd(&cmd)) {
            nob_set_current_dir(start_dir);
            return false;
        }
    }

    if (!nob_file_exists(unzip_dir)) {
        nob_log(NOB_INFO, "Extracting raylib release...");
        nob_cmd_append(&cmd, "unzip", zip_name);
        if (!run_cmd(&cmd)) {
            nob_set_current_dir(start_dir);
            return false;
        }
    }

    nob_set_current_dir(start_dir);
    return true;
}
#else
static bool build_raylib_posix(void)
{
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "cmake", "-S", ".", "-B", "build",
                   "-DCMAKE_BUILD_TYPE=Release",
                   "-DBUILD_EXAMPLES=OFF",
                   "-DBUILD_SHARED_LIBS=OFF");
    if (!run_cmd(&cmd)) return false;

    nob_cmd_append(&cmd, "cmake", "--build", "build",
                   "--config", "Release",
                   "--target", "raylib",
                   "--parallel");
    if (!run_cmd(&cmd)) return false;

    return true;
}
#endif

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    NOB_UNUSED(argc);
    NOB_UNUSED(argv);

    if (!ensure_raylib_dir()) return 1;

#if defined(_WIN32)
    if (!build_raylib_windows()) return 1;
#else
    if (!build_raylib_posix()) return 1;
#endif

    return 0;
}
