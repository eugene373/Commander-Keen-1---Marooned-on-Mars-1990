/* Headless platform for tests: scripted input, virtual clock, PPM frame dumps.
 *   KEEN_SCRIPT="frames:dir:jump:pogo:key;..."   (dir 0-7 or 8, key = ASCII or 0, e.g. "3:8:0:0:13;40:2:0:0:0")
 *   KEEN_DUMP="12,80"      write dump_<frame>.ppm after those polls
 *   KEEN_MAXFRAMES=N       quit after N polls */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ck1.h"

static int64_t vt;
static int frame;
static const uint32_t *last;
typedef struct { int n, dir, jump, pogo, key; } Step;
static Step steps[256];
static int nsteps;

int plat_init(const char *t)
{
    const char *s = getenv("KEEN_SCRIPT");
    (void)t;
    while (s && *s && nsteps < 256) {
        Step *st = &steps[nsteps++];
        if (sscanf(s, "%d:%d:%d:%d:%d", &st->n, &st->dir, &st->jump, &st->pogo, &st->key) != 5) { nsteps--; break; }
        s = strchr(s, ';');
        if (!s) break;
        s++;
    }
    return 1;
}
void plat_shutdown(void) {}
const char *plat_exe_dir(void) { return "."; }
void plat_error(const char *m) { fprintf(stderr, "ERROR: %s\n", m); }
int64_t plat_time_us(void) { vt += 1000; return vt; }
void plat_sleep_ms(int ms) { vt += (int64_t)ms * 1000; }
void plat_play_wav(const uint8_t *w, size_t n) { (void)w; (void)n; }
void plat_present(const uint32_t *p) { last = p; }

static void dump(int f)
{
    char name[64];
    FILE *o;
    int i;
    snprintf(name, sizeof name, "dump_%04d.ppm", f);
    o = fopen(name, "wb");
    if (!o || !last) return;
    fprintf(o, "P6\n%d %d\n255\n", SCR_W, SCR_H);
    for (i = 0; i < SCR_W * SCR_H; i++) { fputc((last[i] >> 16) & 255, o); fputc((last[i] >> 8) & 255, o); fputc(last[i] & 255, o); }
    fclose(o);
}

int plat_poll(Input *in)
{
    int i, left = frame;
    const char *d = getenv("KEEN_DUMP"), *mx = getenv("KEEN_MAXFRAMES");
    memset(in, 0, sizeof *in);
    in->dir = 8;
    for (i = 0; i < nsteps; i++) {
        if (left < steps[i].n) {
            in->dir = steps[i].dir; in->jump = steps[i].jump; in->pogo = steps[i].pogo;
            if (left == 0 && steps[i].key) { in->key = steps[i].key; in->held[steps[i].key] = 1; }   /* key only on the first frame of a step */
            break;
        }
        left -= steps[i].n;
    }
    frame++;
    vt += 41192;
    if (d) { const char *p = d; while (p && *p) { if (atoi(p) == frame) dump(frame); p = strchr(p, ','); if (p) p++; } }
    if (mx && frame >= atoi(mx)) { dump(frame); return 0; }
    return 1;
}
