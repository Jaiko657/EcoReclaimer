#define NOB_IMPLEMENTATION
#include "nob.h"

static bool ensure_unity_dir(void)
{
    if (nob_file_exists("src/unity.c") && nob_file_exists("src/unity.h")) return true;
    if (nob_file_exists("Unity/src/unity.c") && nob_file_exists("Unity/src/unity.h")) {
        return nob_set_current_dir("Unity");
    }
    nob_log(NOB_ERROR, "Run from Unity dir or its parent.");
    return false;
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    NOB_UNUSED(argc);
    NOB_UNUSED(argv);

    if (!ensure_unity_dir()) return 1;

    return 0;
}
