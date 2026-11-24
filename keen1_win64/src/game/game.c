/* Top level: asset loading, frame pacing, sound priority, dialogs, title screen, new game. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game_int.h"

/* ------------------------------------------------------------------ small services */
int rnd(Game *g, int n)
{
    g->rng = g->rng * 1664525u + 1013904223u;
    return n > 0 ? (int)((g->rng >> 8) % (uint32_t)n) : 0;
}

static int     s_snd_prio;
static int64_t s_snd_end_us;

void snd_reset(void) { s_snd_prio = 0; s_snd_end_us = 0; }

/* Sound_Play: a sound whose priority is lower than the one still playing is ignored (keen1_re STEP18). */
void snd_play(Game *g, int id)
{
    int64_t now = plat_time_us();
    (void)g;
    if (id <= 0 || id >= 64 || !A.snd[id]) return;
    if (now < s_snd_end_us && A.snd_prio[id] < s_snd_prio) return;
    s_snd_prio = A.snd_prio[id];
    s_snd_end_us = now + (int64_t)(1e6 * A.snd_ticks[id] / TICK_HZ);
    plat_play_wav(A.snd[id], A.snd_len[id]);
}

void score_add(Game *g, int pts)
{
    g->s.score += (uint32_t)pts;
    while (g->s.score >= g->s.next_life) {          /* extra Keen every 20000 points */
        g->s.lives++;
        g->s.next_life += 20000;
        snd_play(g, 0x1C);
    }
}

/* One presented frame: flip, hold 145.65/6 = 24.3 frames per second, read the keyboard. */
int frame_wait(Game *g)
{
    static int64_t next_us;
    int64_t now, frame_us = (int64_t)(1e6 * TICK_DELTA / TICK_HZ);
    gfx_flip(g);
    now = plat_time_us();
    if (next_us == 0 || now - next_us > 4 * frame_us) next_us = now;
    next_us += frame_us;
    while ((now = plat_time_us()) < next_us) {
        int64_t rem = next_us - now;
        if (rem > 2000) plat_sleep_ms((int)((rem - 1000) / 1000));
    }
    g->prev = g->in;
    if (!plat_poll(&g->in)) return 0;
    g->frame++;
    return 1;
}

/* ------------------------------------------------------------------ dialogs */
#define MAX_LINES 24
#define WRAP 30

static int wrap_text(const char *text, char lines[MAX_LINES][WRAP + 2])
{
    int n = 0, col = 0;
    const char *p = text;
    char word[64];
    lines[0][0] = 0;
    while (*p && n < MAX_LINES - 1) {
        if (*p == '\n') { n++; lines[n][0] = 0; col = 0; p++; continue; }
        if (*p == ' ') { p++; continue; }
        {
            int wl = 0;
            while (*p && *p != ' ' && *p != '\n' && wl < 60) word[wl++] = *p++;
            word[wl] = 0;
            if (col && col + 1 + wl > WRAP) { n++; lines[n][0] = 0; col = 0; }
            if (col) { strcat(lines[n], " "); col++; }
            if (wl > WRAP) word[WRAP] = 0, wl = WRAP;
            strcat(lines[n], word);
            col += wl;
        }
    }
    return n + 1;
}

static int box_wait(Game *g, const char *text, const char *footer, int yn)
{
    char lines[MAX_LINES][WRAP + 2];
    int n = wrap_text(text, lines), i, maxw = (int)strlen(footer), bw, bh, bx, by;
    for (i = 0; i < n; i++) if ((int)strlen(lines[i]) > maxw) maxw = (int)strlen(lines[i]);
    bw = maxw * 8 + 24;
    bh = (n + 3) * 8 + 16;
    bx = (SCR_W - bw) / 2;
    by = (SCR_H - bh) / 2;
    gfx_box(g, bx, by, bw, bh);
    for (i = 0; i < n; i++) gfx_text(g, bx + 12, by + 10 + i * 8, lines[i]);
    gfx_text(g, bx + (bw - (int)strlen(footer) * 8) / 2, by + bh - 18, footer);
    for (;;) {
        if (!frame_wait(g)) return -1;
        if (yn) {
            if (g->in.key == 'Y') return 1;
            if (g->in.key == 'N' || g->in.key == 27) return 0;
        } else if (g->in.key == 13 || g->in.key == ' ' || g->in.key == 27) return 1;
    }
}

int msg_box(Game *g, const char *text, int dim) { (void)dim; return box_wait(g, text, "Press ENTER", 0) > 0; }
int ask_yn(Game *g, const char *text) { return box_wait(g, text, "(Y/N)", 1); }

/* ------------------------------------------------------------------ title screen & new game */
static int title_screen(Game *g)
{
    int sel = 0;
    static const char *items[2] = {"NEW GAME", "QUIT"};
    for (;;) {
        int i;
        gfx_clear(g, 0);
        if (A.title) gfx_bitmap(g, A.title, A.title_w, A.title_h, (SCR_W - A.title_w) / 2, 6);
        gfx_text(g, 80, 126, "COMMANDER KEEN");
        gfx_text(g, 80, 134, "Marooned on Mars");
        gfx_box(g, 100, 150, 120, 36);
        for (i = 0; i < 2; i++) {
            gfx_text(g, 112, 158 + i * 12, i == sel ? ">" : " ");
            gfx_text(g, 124, 158 + i * 12, items[i]);
        }
        gfx_text(g, 4, 190, "Arrows  Ctrl/Z jump  Alt/X pogo  Space fire");
        if (!frame_wait(g)) return -1;
        if ((g->in.dir == 0 && g->prev.dir != 0) || (g->in.dir == 4 && g->prev.dir != 4)) sel ^= 1;
        if (g->in.key == 13 || g->in.key == ' ') return sel == 0;
        if (g->in.key == 27) return 0;
    }
}

static void new_game(Game *g)
{
    memset(&g->s, 0, sizeof g->s);
    g->s.lives = 4;
    g->s.next_life = 20000;
    g->god = 0;
    worldmap_reset();
}

int game_run(const char *datadir)
{
    Game *g;
    int r;
    if (assets_load(datadir)) {
        plat_error("Cannot load the Commander Keen 1 data files.\n\nPut LEVEL*.CK1, EGAHEAD.CK1, EGALATCH.CK1, EGASPRIT.CK1 and SOUNDS.CK1 next to the program, or pass the folder on the command line.");
        return 1;
    }
    g = (Game *)calloc(1, sizeof(Game));
    if (!g) return 1;
    g->a = &A;
    g->rng = (uint32_t)plat_time_us();
    while ((r = title_screen(g)) > 0) {
        new_game(g);
        if (worldmap_run(g) < 0) break;
    }
    free(g);
    assets_free();
    return 0;
}
