/* Entry point: finds the game data and starts the game.   keen1.exe [path-to-folder-with-the-.CK1-files] */
#include <stdio.h>
#include <string.h>
#include "ck1.h"

#ifdef _WIN32
#define SEP "\\"
#else
#define SEP "/"
#endif

static int has_data(const char *dir)
{
    char p[1024];
    FILE *f;
    snprintf(p, sizeof p, "%s%sEGAHEAD.CK1", dir, SEP);
    f = fopen(p, "rb");
    if (f) fclose(f);
    return f != NULL;
}

int main(int argc, char **argv)
{
    char dir[1024];
    int rc, i;
    const char *exe;
    if (!plat_init("Commander Keen 1 - Marooned on Mars (64-bit)")) { fprintf(stderr, "cannot create the window\n"); return 1; }
    exe = plat_exe_dir();
    dir[0] = 0;
    if (argc > 1 && has_data(argv[1])) snprintf(dir, sizeof dir, "%s", argv[1]);
    {
        char cand[6][1024];
        snprintf(cand[0], 1024, "%s", exe);
        snprintf(cand[1], 1024, "%s%sdata", exe, SEP);
        snprintf(cand[2], 1024, "%s%sCKeen1", exe, SEP);
        snprintf(cand[3], 1024, "%s%s..%sCKeen1", exe, SEP, SEP);
        snprintf(cand[4], 1024, ".");
        snprintf(cand[5], 1024, "CKeen1");
        for (i = 0; i < 6 && !dir[0]; i++) if (has_data(cand[i])) snprintf(dir, sizeof dir, "%s", cand[i]);
    }
    if (!dir[0]) snprintf(dir, sizeof dir, "%s", argc > 1 ? argv[1] : exe);
    rc = game_run(dir);
    plat_shutdown();
    return rc;
}
