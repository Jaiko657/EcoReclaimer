#define NOB_IMPLEMENTATION
#include "nob.h"

static bool build_xmlc(void)
{
    const char *src_file = "src/xml.c";
    const char *obj_file = "build/xml.o";
    const char *lib_file = "build/libxml.a";

    if (!nob_mkdir_if_not_exists("build")) return false;

#if defined(_WIN32)
    const char *cc = "gcc";
    const char *ar = "ar";
#else
    const char *cc = "gcc";
    const char *ar = "ar";
#endif

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, cc, "-std=c11", "-O2", "-c", src_file, "-o", obj_file);
    if (!nob_cmd_run(&cmd)) return false;

    nob_cmd_append(&cmd, ar, "rcs", lib_file, obj_file);
    if (!nob_cmd_run(&cmd)) return false;

    return true;
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!build_xmlc()) return 1;

    return 0;
}
