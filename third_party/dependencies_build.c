#define NOB_IMPLEMENTATION
#include "nob.h"

static bool run_cmd(Nob_Cmd *cmd)
{
    bool ok = nob_cmd_run(cmd);
    cmd->count = 0;
    return ok;
}

static bool ensure_third_party_dir(void)
{
    if (nob_file_exists("nob.h") || nob_file_exists("nob.h/nob.h")) return true;
    if (nob_file_exists("third_party/nob.h/nob.h")) {
        return nob_set_current_dir("third_party");
    }
    nob_log(NOB_ERROR, "Run from repo root or third_party directory.");
    return false;
}

static bool git_submodule_update(void)
{
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "git", "submodule", "update", "--init", "--recursive");
    return run_cmd(&cmd);
}

static bool apply_patch_if_needed(const char *repo_dir, const char *patch_path)
{
    const char *start_dir = nob_get_current_dir_temp();
    if (!nob_set_current_dir(repo_dir)) return false;

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "git", "apply", "--reverse", "--check", patch_path);
    if (run_cmd(&cmd)) {
        nob_set_current_dir(start_dir);
        return true;
    }

    nob_cmd_append(&cmd, "git", "apply", "--check", patch_path);
    if (!run_cmd(&cmd)) {
        nob_log(NOB_ERROR, "Patch does not apply cleanly: %s", patch_path);
        nob_set_current_dir(start_dir);
        return false;
    }

    nob_cmd_append(&cmd, "git", "apply", patch_path);
    if (!run_cmd(&cmd)) {
        nob_set_current_dir(start_dir);
        return false;
    }

    nob_set_current_dir(start_dir);
    return true;
}

static const char *build_exe_name(const char *base)
{
#if defined(_WIN32)
    return nob_temp_sprintf("%s.exe", base);
#else
    return base;
#endif
}

static bool ensure_build_exe(const char *repo_dir, const char *exe_base, const char *build_script)
{
    const char *exe_name = build_exe_name(exe_base);
    const char *exe_path = nob_temp_sprintf("%s/%s", repo_dir, exe_name);
    if (nob_file_exists(exe_path)) return true;

    const char *start_dir = nob_get_current_dir_temp();
    if (!nob_set_current_dir(repo_dir)) return false;

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "gcc", "-O2", "-std=c11", "-I", "..",
                   "-o", exe_name, build_script);
    if (!run_cmd(&cmd)) {
        nob_set_current_dir(start_dir);
        return false;
    }

    nob_set_current_dir(start_dir);
    return true;
}

static bool run_build_exe(const char *repo_dir, const char *exe_base)
{
    const char *exe_name = build_exe_name(exe_base);
    const char *start_dir = nob_get_current_dir_temp();
    if (!nob_set_current_dir(repo_dir)) return false;

    Nob_Cmd cmd = {0};
#if defined(_WIN32)
    nob_cmd_append(&cmd, exe_name);
#else
    nob_cmd_append(&cmd, nob_temp_sprintf("./%s", exe_name));
#endif

    if (!run_cmd(&cmd)) {
        nob_set_current_dir(start_dir);
        return false;
    }

    nob_set_current_dir(start_dir);
    return true;
}

static int dependencies_build_main(int argc, char **argv)
{
    NOB_UNUSED(argc);
    NOB_UNUSED(argv);

    if (!ensure_third_party_dir()) return 1;
    if (!git_submodule_update()) return 1;
    if (!apply_patch_if_needed("xml.c", "../build_scripts/xml.c/attribute_parsing.patch")) return 1;
    if (!ensure_build_exe("xml.c", "xmlc_build", "../build_scripts/xml.c/xmlc_build.c")) return 1;
    if (!run_build_exe("xml.c", "xmlc_build")) return 1;
    if (!ensure_build_exe("raylib", "raylib_build", "../build_scripts/raylib/raylib_build.c")) return 1;
    if (!run_build_exe("raylib", "raylib_build")) return 1;
    if (!ensure_build_exe("Unity", "unity_build", "../build_scripts/unity/unity_build.c")) return 1;
    if (!run_build_exe("Unity", "unity_build")) return 1;

    return 0;
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);
    return dependencies_build_main(argc, argv);
}

