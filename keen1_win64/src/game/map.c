/* World map (LEVEL80): Keen walks 4 px per frame in 8 directions, level entrances are plane-1 values 1..16, 20 = ship,
 * 38/41/46 = teleporter pads, bit-15 values = invisible walls (keen1_re STEP6 section 4, STEP7 section 7). */
#include <stdio.h>
#include <string.h>
#include "game_int.h"

static Level map_lv;
static int map_loaded;
static int32_t map_keen_x, map_keen_y;

static const int PAD_X[3] = {28, 36, 62}, PAD_Y[3] = {6, 5, 37};
static const char *PART_NAME[4] = {"Joystick", "Battery", "Vacuum Cleaner", "Everclear"};

static int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

static void map_camera(Game *g, const Obj *k, int snap)
{
    int kx = (k->x1 + k->x2) >> 9, ky = (k->y1 + k->y2) >> 9;
    int minx = 2 * 16, maxx = (g->lv.w - 22) * 16, miny = 2 * 16, maxy = (g->lv.h - 15) * 16 + 8;
    if (snap) { g->cam_x = kx - 160; g->cam_y = ky - 80; }
    if (kx - g->cam_x < 8 * 16) g->cam_x = kx - 8 * 16;
    if (kx - g->cam_x > 12 * 16) g->cam_x = kx - 12 * 16;
    if (ky - g->cam_y < 4 * 16) g->cam_y = ky - 4 * 16;
    if (ky - g->cam_y > 8 * 16) g->cam_y = ky - 8 * 16;
    g->cam_x = clampi(g->cam_x, minx, maxx < minx ? minx : maxx);
    g->cam_y = clampi(g->cam_y, miny, maxy < miny ? miny : maxy);
}

static void map_draw(Game *g, const Obj *k)
{
    int tx0 = g->cam_x >> 4, ty0 = g->cam_y >> 4, ox = -(g->cam_x & 15), oy = -(g->cam_y & 15), x, y;
    char buf[48];
    gfx_clear(g, 0);
    for (y = 0; y < 14; y++)
        for (x = 0; x < 21; x++) {
            int tx = tx0 + x, ty = ty0 + y;
            if (tx >= 0 && ty >= 0 && tx < g->lv.w && ty < g->lv.h) gfx_tile16(g, g->lv.p0[ty * g->lv.w + tx], ox + x * 16, oy + y * 16);
        }
    gfx_sprite(g, k->sprite, (k->x >> 8) - g->cam_x, (k->y >> 8) - g->cam_y);
    gfx_rect(g, 0, 0, 160, 9, gfx_text_bg());
    snprintf(buf, sizeof buf, "%07u L%d A%02d", (unsigned)g->s.score, g->s.lives < 0 ? 0 : g->s.lives, g->s.ammo > 99 ? 99 : g->s.ammo);
    gfx_text(g, 1, 1, buf);
}

/* Replace a finished level's marker with the "done" tiles (0x4D single, 0x4E.. for 2-wide / 2x2 cities).
 * Levels 1 and 7 have a 2x2 sign but only half of it carries the level marker: the other column/row is an
 * invisible wall (plane-1 bit 15). Grow the box over those wall tiles so the whole sign is drawn. */
static int g_gate_level;
static int is_wall(const Level *m, int x, int y)
{
    return x >= 0 && y >= 0 && x < m->w && y < m->h && m->p1[y * m->w + x] == (0x8000 | g_gate_level);
}

static void mark_done(Game *g, int level)
{
    int x, y, minx = 9999, maxx = -1, miny = 9999, maxy = -1;
    Level *m = &g->lv;
    for (y = 0; y < m->h; y++)
        for (x = 0; x < m->w; x++)
            if (m->p1[y * m->w + x] == level) {
                if (x < minx) minx = x;
                if (x > maxx) maxx = x;
                if (y < miny) miny = y;
                if (y > maxy) maxy = y;
            }
    if (maxx < 0) return;
    g_gate_level = level;
    if (maxx == minx && maxy > miny) {                 /* 1 wide, 2 tall: the other column is an invisible wall */
        int ok_r = 1, ok_l = 1;
        for (y = miny; y <= maxy; y++) { ok_r &= is_wall(m, maxx + 1, y); ok_l &= is_wall(m, minx - 1, y); }
        if (ok_r) maxx++; else if (ok_l) minx--;
    }
    if (maxy == miny && maxx > minx) {                 /* 2 wide, 1 tall: the other row is an invisible wall */
        int ok_b = 1, ok_t = 1;
        for (x = minx; x <= maxx; x++) { ok_b &= is_wall(m, x, maxy + 1); ok_t &= is_wall(m, x, miny - 1); }
        if (ok_t) miny--; else if (ok_b) maxy++;
    }
    {
        int w = maxx - minx + 1, h = maxy - miny + 1;
        for (y = miny; y <= maxy; y++)
            for (x = minx; x <= maxx; x++) {
                m->p0[y * m->w + x] = (w == 1 && h == 1) ? 0x4D : (uint16_t)(0x4E + (y - miny) * 2 + (x - minx));
                /* the level's marker and its gate (invisible wall 0x8000|level) both go away: the way on is open */
                if (m->p1[y * m->w + x] == level || m->p1[y * m->w + x] == (0x8000 | level)) m->p1[y * m->w + x] = 0;
            }
    }
}

static void place_keen(Obj *k, int32_t x, int32_t y)
{
    const Sprite *s;
    k->x = x; k->y = y;
    k->sprite = 36;
    k->dx = k->dy = 0;
    s = &A.spr[k->sprite];
    k->x1 = k->x + s->hx1; k->y1 = k->y + s->hy1; k->x2 = k->x + s->hx2; k->y2 = k->y + s->hy2;
}

static void set_keen(Game *g, Obj *k, int tx, int ty) { (void)g; place_keen(k, (int32_t)tx << 12, (int32_t)ty << 12); }

static int all_parts(const Game *g) { return g->s.part[0] && g->s.part[1] && g->s.part[2] && g->s.part[3]; }

void worldmap_free(void)
{
    if (map_loaded) level_free(&map_lv);
    map_loaded = 0;
}

void worldmap_reset(void) { worldmap_free(); }

int worldmap_run(Game *g)
{
    Obj k;
    int x, y, armed = 0, anim = 0, step_snd = 0, face = 1;
    static const int VX[8] = {0, 1, 1, 1, 0, -1, -1, -1}, VY[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
    int started = map_loaded;

    if (!map_loaded) {
        if (level_load(&map_lv, 80)) { msg_box(g, "Cannot load LEVEL80.CK1", 0); return -1; }
        map_loaded = 1;
    }
    g->lv = map_lv;
    memset(&k, 0, sizeof k);
    k.type = OT_PLAYER;
    if (!started) {
        for (y = 0; y < g->lv.h; y++)
            for (x = 0; x < g->lv.w; x++)
                if (g->lv.p1[y * g->lv.w + x] == 255) { set_keen(g, &k, x, y); g->lv.p1[y * g->lv.w + x] = 0; }
    } else place_keen(&k, map_keen_x, map_keen_y);
    map_camera(g, &k, 1);

    for (;;) {
        int dir, vc = 0, marker, cx, cy;
        map_draw(g, &k);
        if (!frame_wait(g)) return -1;
        if (g->in.key == 27) {
            int r = ask_yn(g, "Quit to the title screen?\nPress Y or N");
            if (r < 0) return -1;
            if (r) { map_keen_x = k.x; map_keen_y = k.y; map_lv = g->lv; return 0; }
        }
        if (g->in.held['G'] && g->in.held['O'] && g->in.held['D'] && !g->prev.held['D']) { g->god = !g->god; snd_play(g, 0x19); }
        dir = g->in.dir;
        k.dx = k.dy = 0;
        if (dir >= 0 && dir < 8) {
            k.dx = VX[dir] * 1024;
            k.dy = VY[dir] * 1024;
            if (VX[dir] > 0) face = 0; else if (VX[dir] < 0) face = 2;
            else face = VY[dir] > 0 ? 1 : 3;
            if (g->god) { k.x += k.dx; k.y += k.dy; k.x1 += k.dx; k.x2 += k.dx; k.y1 += k.dy; k.y2 += k.dy; }
            else vc = obj_move_map(g, &k);
            anim++;
            if (--step_snd <= 0) { snd_play(g, vc ? 2 : 1); step_snd = 6; }
        } else { anim = 0; }
        k.sprite = 32 + face * 4 + ((anim >> 2) & 3);
        {
            const Sprite *s = &A.spr[k.sprite];
            k.x1 = k.x + s->hx1; k.y1 = k.y + s->hy1; k.x2 = k.x + s->hx2; k.y2 = k.y + s->hy2;
        }
        k.x = clampi(k.x, 0, (g->lv.w << 12) - 0x1000);
        k.y = clampi(k.y, 0, (g->lv.h << 12) - 0x1000);
        map_camera(g, &k, 0);

        cx = ((k.x1 + k.x2) >> 1) >> 12;
        cy = ((k.y1 + k.y2) >> 1) >> 12;
        marker = (cx >= 0 && cy >= 0 && cx < g->lv.w && cy < g->lv.h) ? g->lv.p1[cy * g->lv.w + cx] : 0;
        if (marker == 0 || marker == 255 || marker >= 0x8000) { armed = 1; continue; }
        if (!armed) continue;
        armed = 0;

        if (marker >= 1 && marker <= 16 && !g->s.level_done[marker - 1]) {
            int r;
            snd_play(g, 3);
            map_keen_x = k.x; map_keen_y = k.y;
            map_lv = g->lv;
            r = level_play(g, marker);
            g->lv = map_lv;
            if (r == -1) return -1;
            if (r == -4) { msg_box(g, "Cannot load that level file.", 0); continue; }
            if (r == -3) { map_camera(g, &k, 1); continue; }              /* left the level with ESC */
            if (r == -2 || r == 0) {
                char buf[64];
                g->s.lives--;
                if (g->s.lives < 0) {
                    snd_play(g, 0x10);
                    map_lv = g->lv;
                    msg_box(g, "GAME OVER", 0);
                    return 0;
                }
                snd_play(g, 0x29);
                snprintf(buf, sizeof buf, "Keens left: %d", g->s.lives);
                map_draw(g, &k);
                if (!msg_box(g, buf, 0)) return -1;
                continue;
            }
            if (r == 1) {
                g->s.level_done[marker - 1] = 1;
                mark_done(g, marker);
            } else if (r == 2) {                                           /* teleporter in the ice: dark side of Mars */
                snd_play(g, 0x12);
                set_keen(g, &k, PAD_X[2], PAD_Y[2]);
                map_camera(g, &k, 1);
            }
            if (all_parts(g)) {
                map_draw(g, &k);
                map_lv = g->lv;
                msg_box(g, "You found all four parts of your ship!\n\nBilly Blaze's ship is repaired and he heads home to Earth.\n\nTHE END", 0);
                return 1;
            }
            continue;
        }
        if (marker == 20) {                                                 /* the ship */
            char buf[200];
            int i, any = 0;
            snprintf(buf, sizeof buf, "Your ship is missing these parts:\n\n");
            for (i = 0; i < 4; i++)
                if (!g->s.part[i]) { strncat(buf, PART_NAME[i], sizeof buf - strlen(buf) - 4); strncat(buf, "\n", sizeof buf - strlen(buf) - 1); any = 1; }
            if (any) strncat(buf, "\nGO GET THEM!", sizeof buf - strlen(buf) - 1);
            else { map_lv = g->lv; msg_box(g, "You found all four parts of your ship!\n\nTHE END", 0); return 1; }
            snd_play(g, 0x19);
            if (!msg_box(g, buf, 0)) return -1;
            continue;
        }
        if (marker == 38 || marker == 41 || marker == 46) {                 /* teleporter pads */
            int dst = (marker == 38) ? 1 : (marker == 41) ? 0 : 1;
            snd_play(g, 0x12);
            set_keen(g, &k, PAD_X[dst], PAD_Y[dst]);
            map_camera(g, &k, 1);
            continue;
        }
    }
}
