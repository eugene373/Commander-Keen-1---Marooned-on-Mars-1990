/* In-level gameplay.  Rules and constants come from keen1_re (STEP4 enemy AI, STEP5 player/physics, STEP12 object loop,
 * STEP17/18 camera, STEP20 frame timing).  Places where the reverse engineering has no answer are marked GUESS. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game_int.h"

#define TD TICK_DELTA
#define EMPTY_TILE 0x8F
#define OT_DYING 0x0E

enum { P_WALK = 1, P_WINDUP, P_AIR, P_SHOOT, P_POGOG, P_POGOA, P_FROZEN, P_DEAD, P_EXIT };
enum { Y_WALK = 1, Y_STAND, Y_STUN };
enum { G_STAND = 1, G_MOVE };
enum { V_WALK = 1, V_JUMP, V_STAND };
enum { C_WALK = 1, C_PAUSE, T_FIRE };

#define DIR_RIGHT(d) ((d) >= 1 && (d) <= 3)
#define DIR_LEFT(d)  ((d) >= 5 && (d) <= 7)

static int g_player_done;       /* player object left the screen after dying */

/* ------------------------------------------------------------------ tiles & collision */
static int tile_at(const Level *lv, int tx, int ty)
{
    if (tx < 0 || ty < 0 || tx >= lv->w || ty >= lv->h) return -1;
    return lv->p0[ty * lv->w + tx];
}
static int tab(const unsigned short *T, int t) { return (t < 0 || t >= NUM_TILES) ? 0 : T[t]; }

static void obj_hitbox(Obj *o)
{
    const Sprite *s = &A.spr[o->sprite];
    o->x1 = o->x + s->hx1; o->y1 = o->y + s->hy1;
    o->x2 = o->x + s->hx2; o->y2 = o->y + s->hy2;
}

static int s_p1mask;      /* world-map mode: plane-1 values with bit 15 block like walls (WorldMap_Collide) */
static int blk(const Game *g, const unsigned short *T, int tx, int ty)
{
    if (tab(T, tile_at(&g->lv, tx, ty))) return 1;
    return s_p1mask && tx >= 0 && ty >= 0 && tx < g->lv.w && ty < g->lv.h && (g->lv.p1[ty * g->lv.w + tx] & 0x8000);
}

static int32_t clampi(int32_t v, int32_t lo, int32_t hi) { return v < lo ? lo : (v > hi ? hi : v); }

/* Obj_TileCollide (keen1_re STEP11/12/13, checked 4000/4000 against the original machine code): the box moves
 * vertically first, then horizontally; a blocking tile is only tested when a tile boundary was crossed. */
static int obj_tile_collide(Game *g, Obj *o)
{
    int vc = 0, tx, ty, tx1, tx2, ty1, ty2;
    o->dx = clampi(o->dx, -0xF00, 0xF00);
    o->dy = clampi(o->dy, -0xF00, 0xF00);
    o->y2 += o->dy; o->y1 += o->dy;
    tx1 = o->x1 >> 12; tx2 = o->x2 >> 12;
    if (o->dy > 0) {
        if ((o->y2 >> 12) != ((o->y2 - o->dy) >> 12)) {
            ty = o->y2 >> 12;
            for (tx = tx1; tx <= tx2; tx++)
                if (blk(g, TILE_FLOOR, tx, ty)) {
                    int a = (o->y2 + 1) % 0x1000;
                    o->yvel = 0; o->dy -= a; o->y1 -= a; o->y2 -= a; vc = COL_FLOOR;
                    break;
                }
        }
    } else if (o->dy < 0) {
        if ((o->y1 >> 12) != ((o->y1 - o->dy) >> 12)) {
            ty = o->y1 >> 12;
            for (tx = tx1; tx <= tx2; tx++)
                if (blk(g, TILE_CEIL, tx, ty)) {
                    int a = 0x1000 - (o->y1 % 0x1000);
                    o->yvel = 0; o->dy += a; o->y1 += a; o->y2 += a; vc = COL_CEIL;
                    break;
                }
        }
    }
    o->x1 += o->dx; o->x2 += o->dx;
    ty1 = o->y1 >> 12; ty2 = o->y2 >> 12;
    if (o->dx > 0) {
        if ((o->x2 >> 12) != ((o->x2 - o->dx) >> 12)) {
            tx = o->x2 >> 12;
            for (ty = ty1; ty <= ty2; ty++)
                if (blk(g, TILE_BLOCK_R, tx, ty)) {
                    int a = (o->x2 + 1) % 0x1000;
                    o->xvel = 0; o->dx -= a; o->x1 -= a; o->x2 -= a; vc |= COL_RIGHT;
                    break;
                }
        }
    } else if (o->dx < 0) {
        if ((o->x1 >> 12) != ((o->x1 - o->dx) >> 12)) {
            tx = o->x1 >> 12;
            for (ty = ty1; ty <= ty2; ty++)
                if (blk(g, TILE_BLOCK_L, tx, ty)) {
                    int a = 0x1000 - (o->x1 % 0x1000);
                    o->xvel = 0; o->dx += a; o->x1 += a; o->x2 += a; vc |= COL_LEFT;
                    break;
                }
        }
    }
    return vc;
}

int obj_move_map(Game *g, Obj *o)          /* world map: movement set directly in dx/dy */
{
    int vc;
    s_p1mask = 1;
    obj_hitbox(o);
    vc = obj_tile_collide(g, o);
    s_p1mask = 0;
    o->x += o->dx;
    o->y += o->dy;
    return vc;
}

static int obj_move(Game *g, Obj *o)       /* Obj_MoveAndCollide */
{
    int vc;
    o->dx += o->xvel * TD;
    o->dy += o->yvel * TD;
    obj_hitbox(o);
    vc = obj_tile_collide(g, o);
    o->x += o->dx;
    o->y += o->dy;
    return vc;
}

static void obj_gravity(Obj *o)            /* Obj_ApplyGravity */
{
    int i;
    for (i = 1; i <= TD; i++) {
        o->yvel += 3;
        if (o->yvel > 200) o->yvel = 200; else if (o->yvel < -400) o->yvel = -400;
        if (i != TD) o->dy += o->yvel;
    }
}
static void obj_accel_x(Obj *o, int d)     /* Obj_AccelX: +-120 limit */
{
    int i;
    for (i = 1; i <= TD; i++) {
        o->xvel += d;
        if (o->xvel > 120) o->xvel = 120; else if (o->xvel < -120) o->xvel = -120;
        if (i != TD) o->dx += o->xvel;
    }
}
static void obj_accel_y(Obj *o, int maxs, int d)   /* Obj_AccelY */
{
    int i;
    for (i = 1; i <= TD; i++) {
        o->yvel += d;
        if (o->yvel > maxs) o->yvel = maxs; else if (o->yvel < -maxs) o->yvel = -maxs;
        if (i != TD) o->dy += o->yvel;
    }
}

static int boxes_overlap(const Obj *a, const Obj *b)
{
    return a->x1 <= b->x2 && b->x1 <= a->x2 && a->y1 <= b->y2 && b->y1 <= a->y2;
}

/* ------------------------------------------------------------------ object table */
static Obj *alloc_obj(Game *g)
{
    int i;
    for (i = 1; i < MAX_OBJECTS; i++)
        if (g->obj[i].type == OT_NONE) { memset(&g->obj[i], 0, sizeof(Obj)); return &g->obj[i]; }
    return NULL;
}

static Obj *spawn(Game *g, int type, int sprite, int32_t x, int32_t y)
{
    Obj *o = alloc_obj(g);
    if (!o) return NULL;
    o->type = type; o->sprite = sprite; o->x = x; o->y = y; o->active = 1; o->facing = 1;
    obj_hitbox(o);
    return o;
}

static void spawn_level_objects(Game *g)
{
    const Level *lv = &g->lv;
    int x, y;
    memset(g->obj, 0, sizeof g->obj);
    for (y = 0; y < lv->h; y++)
        for (x = 0; x < lv->w; x++) {
            int v = lv->p1[y * lv->w + x];
            int32_t px = (int32_t)x << 12, py = (int32_t)y << 12;
            Obj *o = NULL;
            switch (v) {
            case 255:
                o = &g->obj[0];
                memset(o, 0, sizeof *o);
                o->type = OT_PLAYER; o->active = 1; o->x = px; o->y = py; o->sprite = 13; o->state = P_AIR; o->facing = 1;
                obj_hitbox(o);
                break;
            case 1: o = spawn(g, OT_YORP, 48, px, py); if (o) { o->state = Y_WALK; o->xvel = 60; o->active = 0; } break;
            case 2: o = spawn(g, OT_GARG, 60, px, py); if (o) { o->state = G_STAND; o->active = 0; } break;
            case 3: o = spawn(g, OT_VORT, 78, px, py);
                    if (o) { o->state = V_STAND; o->hits = (lv->num == 16) ? 0x68 : 3; o->active = 0; } break;
            case 4: o = spawn(g, OT_CAN, 88, px, py); if (o) { o->state = C_WALK; o->xvel = 90; o->active = 0; } break;
            case 5: o = spawn(g, OT_TANK, 98, px, py); if (o) { o->state = C_WALK; o->xvel = 90; o->active = 0; } break;
            case 6: case 7: case 8: case 9:
                o = spawn(g, OT_CANNON, 0, px, py); if (o) { o->aux1 = v - 6; o->active = 0; } break;
            case 10: o = spawn(g, OT_CHAIN, 114, px, py); if (o) o->active = 0; break;
            default: break;
            }
        }
}

/* ------------------------------------------------------------------ player */
static int floor_type(Game *g, const Obj *o)
{
    int tx, ty = (o->y2 + 1) >> 12, best = 0;
    for (tx = o->x1 >> 12; tx <= (o->x2 >> 12); tx++) {
        int f = tab(TILE_FLOOR, tile_at(&g->lv, tx, ty));
        if (f > best) best = f;
    }
    return best ? best : 1;
}

static void player_die(Game *g, int forced)
{
    Obj *p = &g->obj[0];
    if (p->state == P_DEAD) return;
    if (g->god && !forced) return;
    p->state = P_DEAD;
    p->y += 0x800; p->xvel = p->yvel = 0; p->timer = 0; p->aux1 = 0; p->sprite = 22;
    snd_play(g, 8);
}

static void spawn_keen_shot(Game *g)
{
    Obj *p = &g->obj[0], *s;
    int32_t x = p->facing > 0 ? p->x + 0xC00 : p->x - 0x600, y = p->y + 0x900;
    int t = tile_at(&g->lv, (x + 0x400) >> 12, y >> 12);
    s = spawn(g, OT_KEENSHOT, 108, x, y);
    if (!s) return;
    s->xvel = p->facing > 0 ? 400 : -400;
    if (tab(TILE_BLOCK_L, t) || tab(TILE_BLOCK_R, t)) {   /* spawned inside a wall */
        s->type = OT_SPLASH; s->sprite = 110; s->xvel = 0; s->timer = 0;
        snd_play(g, 0x25);
    }
}

static void player_walk_anim(Obj *p)
{
    int base = p->facing > 0 ? 0 : 4;
    p->sprite = base + ((p->aux3 >> 3) & 3);
}

static void pl_clamp_level(Game *g, Obj *p)
{
    int32_t maxx = (int32_t)g->lv.w << 12;
    if (p->x1 < 0) { int32_t d = -p->x1; p->x += d; p->x1 += d; p->x2 += d; if (p->xvel < 0) p->xvel = 0; }
    if (p->x2 > maxx) { int32_t d = p->x2 - maxx; p->x -= d; p->x1 -= d; p->x2 -= d; if (p->xvel > 0) p->xvel = 0; }
    if (p->y1 > ((int32_t)g->lv.h << 12)) { snd_play(g, 0x1B); player_die(g, 1); }
}

static void keen_face(Obj *p) { if (p->xvel > 0) p->facing = 1; else if (p->xvel < 0) p->facing = -1; }

static void pl_steer(Game *g, Obj *p, int mag, int friction)
{
    int d = g->in.dir;
    if (DIR_RIGHT(d)) obj_accel_x(p, mag);
    else if (DIR_LEFT(d)) obj_accel_x(p, -mag);
    else if (friction) {
        if (p->xvel > 0) { obj_accel_x(p, -friction); if (p->xvel < 0) p->xvel = 0; }
        else if (p->xvel < 0) { obj_accel_x(p, friction); if (p->xvel > 0) p->xvel = 0; }
    }
}

static void pl_walk(Game *g, Obj *p)
{
    Input *in = &g->in;
    int d = in->dir, ft, vc, both = in->jump && in->pogo, prev_both = g->prev.jump && g->prev.pogo;
    int pogo_edge = in->pogo && !g->prev.pogo;

    if (both && !prev_both) {                       /* shoot: jump + pogo together */
        p->state = P_SHOOT; p->timer = 0; p->aux1 = 0;
        return;
    }
    if (in->jump && !in->pogo) {                    /* start the wind-up */
        p->aux2 = p->xvel; p->xvel = 0; p->aux1 = 0; p->timer = 0; p->aux3 = 0;
        p->state = P_WINDUP;
        p->sprite = p->facing > 0 ? 8 : 14;
        return;
    }
    if (pogo_edge && !in->jump && g->s.has_pogo) {
        p->xvel = 0; p->timer = 0; p->state = P_POGOG;
        p->sprite = p->facing > 0 ? 25 : 27;
        return;
    }
    ft = floor_type(g, p);
    if (ft < 3) {
        if (DIR_RIGHT(d)) { obj_accel_x(p, 2); if (p->xvel < 0) d = 8; }
        if (DIR_LEFT(d))  { obj_accel_x(p, -2); if (p->xvel > 0) d = 8; }
    }
    if (ft == 1 && d == 8) {                        /* friction only on normal floor */
        if (p->xvel > 0) { obj_accel_x(p, -3); if (p->xvel < 0) p->xvel = 0; }
        else if (p->xvel < 0) { obj_accel_x(p, 3); if (p->xvel > 0) p->xvel = 0; }
    }
    if (ft == 3) p->xvel = p->facing > 0 ? 180 : -180;      /* ice slide */
    keen_face(p);
    if (p->xvel) {
        int before = (p->aux3 >> 3) & 3;
        p->aux3 += TD;
        if ((((p->aux3 >> 3) & 3) != before) && (((p->aux3 >> 3) & 1) == 0)) snd_play(g, (((p->aux3 >> 4) & 1) ? 4 : 0x1E));
    } else p->aux3 = 0;
    player_walk_anim(p);
    obj_gravity(p);
    vc = obj_move(g, p);
    pl_clamp_level(g, p);
    if (!(vc & COL_FLOOR)) { p->state = P_AIR; snd_play(g, 0x1B); }
}

static void pl_windup(Game *g, Obj *p)
{
    Input *in = &g->in;
    int i, d = in->dir;
    p->timer += TD;
    if (in->jump) {
        p->aux1 += 6 * TD;
        if (p->aux1 > 216) p->aux1 = 216;
    } else if (!p->aux3) {                  /* released: a short tap still waits */
        p->aux3 = 1;
        if (p->timer < 12) p->timer = 24 - p->timer;
    }
    for (i = 0; i < TD; i++) {
        if (DIR_RIGHT(d)) p->aux2 += 2; else if (DIR_LEFT(d)) p->aux2 -= 2;
        p->aux2 = (int)clampi(p->aux2, -120, 120);
    }
    p->sprite = (p->facing > 0 ? 8 : 14) + (p->timer / 6 > 5 ? 5 : p->timer / 6);
    p->xvel = 0;
    obj_gravity(p);
    obj_move(g, p);
    if (p->timer >= 36) {
        p->state = P_AIR;
        p->yvel -= p->aux1;
        p->xvel = p->aux2;
        keen_face(p);
        snd_play(g, 6);
    }
}

static void pl_air(Game *g, Obj *p)
{
    Input *in = &g->in;
    int vc;
    pl_steer(g, p, 2, 1);
    keen_face(p);
    p->sprite = p->facing > 0 ? 13 : 19;
    obj_gravity(p);
    vc = obj_move(g, p);
    pl_clamp_level(g, p);
    if (vc & COL_CEIL) snd_play(g, 0x15);
    if (vc & COL_FLOOR) { p->state = P_WALK; snd_play(g, 7); return; }
    if (in->pogo && !g->prev.pogo && g->s.has_pogo) { p->state = P_POGOA; p->timer = 0; }
}

static void pl_pogo_ground(Game *g, Obj *p)
{
    int vc;
    p->timer += TD;
    p->sprite = p->facing > 0 ? 25 : 27;
    pl_steer(g, p, 1, 1);
    obj_gravity(p);
    vc = obj_move(g, p);
    pl_clamp_level(g, p);
    if (!(vc & COL_FLOOR)) { p->state = P_POGOA; return; }
    if (g->in.pogo && !g->prev.pogo && p->timer > TD) { p->state = P_WALK; return; }   /* GUESS: pogo button again leaves the stick */
    if (p->timer >= 22) {
        p->state = P_POGOA;
        p->yvel -= 200;
        snd_play(g, 6);
    }
}

static void pl_pogo_air(Game *g, Obj *p)
{
    int vc;
    pl_steer(g, p, 1, 0);
    keen_face(p);
    p->sprite = p->facing > 0 ? 24 : 26;
    if (g->in.jump && p->yvel < 0) obj_accel_y(p, 200, -1);
    if (g->god && g->in.jump) p->yvel = -200;
    obj_gravity(p);
    vc = obj_move(g, p);
    pl_clamp_level(g, p);
    if (vc & COL_CEIL) snd_play(g, 0x15);
    if (vc & (COL_LEFT | COL_RIGHT)) snd_play(g, 5);
    if (vc & COL_FLOOR) { p->state = P_POGOG; p->timer = 0; }
}

static void pl_shoot(Game *g, Obj *p)
{
    int vc;
    p->timer += TD;
    p->sprite = p->facing > 0 ? 20 : 21;
    if (p->timer > 1 && !p->aux1) {
        p->aux1 = 1;
        if (g->s.ammo > 0) { snd_play(g, 0x0C); g->s.ammo--; spawn_keen_shot(g); }
        else snd_play(g, 0x24);
    }
    if (p->xvel > 0) { obj_accel_x(p, -3); if (p->xvel < 0) p->xvel = 0; }
    else if (p->xvel < 0) { obj_accel_x(p, 3); if (p->xvel > 0) p->xvel = 0; }
    obj_gravity(p);
    vc = obj_move(g, p);
    pl_clamp_level(g, p);
    if (!(vc & COL_FLOOR)) { p->state = P_AIR; return; }
    if (p->timer > 30 && !g->in.jump && !g->in.pogo) p->state = P_WALK;
}

static void pl_frozen(Game *g, Obj *p)
{
    int vc;
    p->timer -= TD;
    if (p->timer > 0) p->sprite = (((800 - p->timer) / 32) & 1) ? 29 : 28;
    else p->sprite = 31;
    if (p->xvel > 0) { obj_accel_x(p, -3); if (p->xvel < 0) p->xvel = 0; }
    else if (p->xvel < 0) { obj_accel_x(p, 3); if (p->xvel > 0) p->xvel = 0; }
    obj_gravity(p);
    vc = obj_move(g, p);
    pl_clamp_level(g, p);
    (void)vc;
    if (p->timer < -50) p->state = P_WALK;
}

static void pl_dead(Game *g, Obj *p)
{
    p->timer += TD;
    if (!p->aux1) {
        p->sprite = ((p->timer / 16) & 1) ? 23 : 22;
        if (p->timer >= 200) {
            p->aux1 = 1;
            p->xvel = rnd(g, 256) - 128;
            p->yvel = -400;
        }
        return;
    }
    obj_gravity(p);
    p->x += p->xvel * TD;
    p->y += p->yvel * TD + p->dy;
    p->dy = 0;
    obj_hitbox(p);
    if (p->y2 < ((int32_t)g->cam_y << 8)) g_player_done = 1;
}

static void pl_exit(Game *g, Obj *p)
{
    p->xvel = 60;
    p->facing = 1;
    p->aux3 += TD;
    player_walk_anim(p);
    obj_gravity(p);
    obj_move(g, p);
    if ((p->x >> 12) >= p->timer) g->level_result = 1;
}

static void player_think(Game *g, Obj *p)
{
    switch (p->state) {
    case P_WALK:   pl_walk(g, p); break;
    case P_WINDUP: pl_windup(g, p); break;
    case P_AIR:    pl_air(g, p); break;
    case P_SHOOT:  pl_shoot(g, p); break;
    case P_POGOG:  pl_pogo_ground(g, p); break;
    case P_POGOA:  pl_pogo_air(g, p); break;
    case P_FROZEN: pl_frozen(g, p); break;
    case P_DEAD:   pl_dead(g, p); break;
    case P_EXIT:   pl_exit(g, p); break;
    default:       p->state = P_AIR; break;
    }
}

/* ------------------------------------------------------------------ hint poles */
static const char *hint_text(int level)
{
    switch (level) {
    case 2:  return "You hear in your mind:\n\nIt is too bad that you cannot read the Standard Galactic Alphabet, human.";
    case 6:  return "A message echoes in your head:\n\nThe teleporter in the ice will send you to the dark side of Mars.";
    case 9:  return "A voice buzzes in your mind:\n\nThere is a hidden city. Look in the dark area of the city to the south.";
    case 10: return "You see these words in your head:\n\nYou will need a raygun in the end, but not to shoot the Vorticon...";
    case 11: return "You hear in your mind:\n\nGAAARRRRRGG!";
    case 12: return "A Yorpish whisper says:\n\nLook for dark, hidden bricks. You can see naught but their upper left corner.";
    case 15: return "A Yorpy mind-thought bellows:\n\nYou cannot kill the Vorticon Commander directly.";
    default: return NULL;
    }
}

/* ------------------------------------------------------------------ tile touch (Player_TouchTiles) */
static void door_open(Game *g, int tx, int ty, int id)
{
    int y;
    snd_play(g, 0x21);
    for (y = ty; y >= 0 && TILE_SPECIAL[g->lv.p0[y * g->lv.w + tx] % NUM_TILES] == id; y--) g->lv.p0[y * g->lv.w + tx] = EMPTY_TILE;
    for (y = ty + 1; y < g->lv.h && TILE_SPECIAL[g->lv.p0[y * g->lv.w + tx] % NUM_TILES] == id; y++) g->lv.p0[y * g->lv.w + tx] = EMPTY_TILE;
}

static int g_pending_hint;

static void touch_tiles(Game *g)
{
    Obj *p = &g->obj[0];
    int tx, ty;
    static const int pts[] = {0, 0, 0, 0, 0, 0, 500, 100, 200, 1000, 5000};
    if (p->state == P_DEAD) return;
    for (tx = p->x1 >> 12; tx <= (p->x2 >> 12); tx++)
        for (ty = p->y1 >> 12; ty <= (p->y2 >> 12); ty++) {
            uint16_t *t;
            int id;
            if (tx < 0 || ty < 0 || tx >= g->lv.w || ty >= g->lv.h) continue;
            t = &g->lv.p0[ty * g->lv.w + tx];
            id = *t < NUM_TILES ? TILE_SPECIAL[*t] : 0;
            if (id == 0 || id > 26) continue;
            switch (id) {
            case 1: player_die(g, 0); break;
            case 2: case 3: case 4: case 5:
                if (g->s.key[id - 2]) door_open(g, tx, ty, id);   /* keys are kept (STEP18) */
                else p->x = (p->dx > 0) ? (p->x & ~0xFFF) : ((p->x + 0x1000) & ~0xFFF);
                break;
            case 6: case 7: case 8: case 9: case 10:
                score_add(g, pts[id]); snd_play(g, 9);
                *t = (*t < 0x131) ? EMPTY_TILE : 0x114;
                break;
            case 11: case 12: case 13: case 14:
                g->s.part[id - 11] = 1; score_add(g, 10000); snd_play(g, 0x0B); *t = EMPTY_TILE;
                break;
            case 15: g->s.ammo += 5; snd_play(g, 0x0A); *t = EMPTY_TILE; break;
            case 16: g->s.has_pogo = 1; snd_play(g, 0x0A); *t = EMPTY_TILE; break;
            case 17:
                if (p->state != P_WALK) break;
                snd_play(g, 0x0F);
                p->state = P_EXIT; p->timer = tx + 2; p->aux1 = ty;
                break;
            case 18: case 19: case 20: case 21:
                g->s.key[id - 18] = 1; snd_play(g, 0x20); *t = EMPTY_TILE;
                break;
            case 22:
                g_pending_hint = 1;
                *t = (g->lv.num == 11) ? 0x1B2 : 0x13B;
                break;
            case 24: g->level_result = 2; break;
            default: break;
            }
        }
}

/* ------------------------------------------------------------------ enemies */
static void make_dying(Game *g, Obj *o, int first_sprite, int frames, int snd)
{
    o->type = OT_DYING; o->sprite = first_sprite; o->aux1 = frames - 1; o->timer = 0;
    o->xvel = 0; o->state = 0;
    snd_play(g, snd);
    (void)g;
}

static void shatter_ice(Game *g, Obj *s)
{
    static const int vx[4] = {300, -300, 300, -300}, vy[4] = {-300, -300, 300, 300};
    int i;
    snd_play(g, 0x13);
    for (i = 0; i < 4; i++) {
        Obj *c = spawn(g, OT_ICECHUNK, 113, s->x, s->y);
        if (c) { c->xvel = vx[i]; c->yvel = vy[i]; c->timer = 0; }
    }
    s->type = OT_NONE;
}

static void to_splash(Game *g, Obj *s, int snd)
{
    s->type = OT_SPLASH; s->sprite = 110; s->xvel = s->yvel = 0; s->timer = 0;
    if (snd) snd_play(g, 0x25);
}

static void face_player(Game *g, Obj *o, int speed)
{
    o->xvel = (g->obj[0].x > o->x) ? speed : -speed;
}

static int blocked_ahead(Game *g, const Obj *o)     /* no floor beyond the leading edge of a patrolling robot */
{
    int tx = (o->xvel > 0 ? o->x2 : o->x1) >> 12, ty = (o->y2 + 1) >> 12;
    return !tab(TILE_FLOOR, tile_at(&g->lv, tx, ty));
}

static void yorp_think(Game *g, Obj *o)
{
    int vc;
    switch (o->state) {
    case Y_WALK:
        if (o->yvel == 0) {
            if (rnd(g, 256) < TD) o->yvel = -rnd(g, 128);
            else if (rnd(g, 256) < TD) { o->state = Y_STAND; o->timer = 0; o->xvel = 0; break; }
        }
        obj_gravity(o);
        vc = obj_move(g, o);
        if (vc & COL_RIGHT) o->xvel = -60;
        if (vc & COL_LEFT) o->xvel = 60;
        o->aux3 += TD;
        o->sprite = (o->xvel > 0 ? 52 : 54) + ((o->aux3 >> 4) & 1);
        break;
    case Y_STAND:
        o->timer += TD;
        o->sprite = 48 + ((o->timer / 30) & 3);
        obj_gravity(o);
        obj_move(g, o);
        if (o->timer >= 200) { o->state = Y_WALK; o->xvel = (g->obj[0].x > o->x) ? 60 : -60; }
        break;
    case Y_STUN:
        o->timer += TD;
        o->xvel = 0;
        o->sprite = 56 + ((o->timer / 16) & 1);
        obj_gravity(o);
        obj_move(g, o);
        if (o->timer >= 300) { o->state = Y_STAND; o->timer = 0; }     /* GUESS: stun length */
        break;
    }
}

static void garg_think(Game *g, Obj *o)
{
    int vc;
    if (o->state == G_STAND) {
        o->timer += TD;
        o->xvel = 0;
        o->sprite = 60 + ((o->timer / 24) & 3);
        obj_gravity(o);
        obj_move(g, o);
        if (o->timer >= 90) {
            int32_t dyk = g->obj[0].y - o->y;
            int charge = dyk > -0x1000 && dyk < 0x1000;           /* Keen on its row */
            face_player(g, o, charge ? 220 : 60);
            o->state = G_MOVE; o->timer = 0;
        }
    } else {
        o->timer += TD;
        o->aux3 += TD;
        o->sprite = (o->xvel > 0 ? 64 : 66) + ((o->aux3 >> 4) & 1);
        obj_gravity(o);
        vc = obj_move(g, o);
        if ((vc & (COL_LEFT | COL_RIGHT)) || o->timer >= 240) { o->state = G_STAND; o->timer = 0; }
    }
}

static void vort_think(Game *g, Obj *o)
{
    int vc;
    switch (o->state) {
    case V_STAND:
        o->timer += TD;
        o->xvel = 0;
        o->sprite = 78 + ((o->timer / 40) & 1);
        obj_gravity(o);
        obj_move(g, o);
        if (o->timer >= 60) { o->state = V_WALK; face_player(g, o, rnd(g, 2) ? 120 : 90); o->timer = 0; }
        break;
    case V_WALK:
        if (o->yvel == 0) {
            int r = rnd(g, 256);
            if (r < TD * 2) { o->state = V_JUMP; o->yvel = -(100 + rnd(g, 200)); }
            else if (r < TD * 3) { o->state = V_STAND; o->timer = 0; break; }
        }
        obj_gravity(o);
        vc = obj_move(g, o);
        if (vc & COL_RIGHT) o->xvel = -90;
        if (vc & COL_LEFT) o->xvel = 90;
        o->aux3 += TD;
        o->sprite = (o->xvel > 0 ? 74 : 70) + ((o->aux3 >> 4) & 3);
        break;
    case V_JUMP:
        obj_gravity(o);
        vc = obj_move(g, o);
        o->sprite = o->xvel > 0 ? 81 : 80;
        if (vc & COL_FLOOR) o->state = V_WALK;
        break;
    }
}

static void robot_think(Game *g, Obj *o, int base_left, int stand, int speed)      /* CAN and TANK patrol */
{
    int vc;
    if (o->state == C_WALK) {
        if (o->xvel == 0) o->xvel = o->facing > 0 ? speed : -speed;
        o->facing = o->xvel > 0 ? 1 : -1;
        obj_gravity(o);
        vc = obj_move(g, o);
        o->aux3 += TD;
        o->sprite = (o->xvel > 0 ? base_left : base_left + 4) + ((o->aux3 >> 4) & 3);   /* positive xvel = *LEFT* frames (STEP4) */
        if ((vc & COL_FLOOR) && (blocked_ahead(g, o))) {
            o->x -= o->dx; o->dx = 0; obj_hitbox(o);
            o->state = C_PAUSE; o->timer = 0; o->facing = -o->facing; o->xvel = 0;
        } else if (vc & (COL_LEFT | COL_RIGHT)) {
            o->state = C_PAUSE; o->timer = 0; o->facing = o->facing > 0 ? -1 : 1; o->xvel = 0;
        } else if (o->type == OT_TANK && (vc & COL_FLOOR) && rnd(g, 256) < TD) {     /* GUESS: random stop to fire */
            o->state = T_FIRE; o->timer = 0; o->aux1 = 0; o->xvel = 0;
        }
    } else if (o->state == C_PAUSE) {
        o->timer += TD;
        o->sprite = stand + ((o->timer / 20) & 1);
        obj_gravity(o);
        obj_move(g, o);
        if (o->timer >= 50) { o->state = C_WALK; o->xvel = o->facing > 0 ? speed : -speed; }
    } else {    /* T_FIRE */
        o->timer += TD;
        o->sprite = stand + ((o->timer / 20) & 1);
        obj_gravity(o);
        obj_move(g, o);
        if (!o->aux1 && o->timer >= 30) {
            Obj *s = spawn(g, OT_TANKSHOT, 109, o->facing > 0 ? o->x + 0x1000 : o->x - 0x600, o->y + 0x600);
            o->aux1 = 1;
            snd_play(g, 0x26);
            if (s) s->xvel = o->facing > 0 ? 350 : -350;
        }
        if (o->timer >= 70) { o->state = C_WALK; o->xvel = o->facing > 0 ? speed : -speed; }
    }
}

static void shot_move(Game *g, Obj *o)
{
    int vc;
    o->dx = o->xvel * TD;
    obj_hitbox(o);
    vc = obj_tile_collide(g, o);
    o->x += o->dx;
    if (vc & (COL_LEFT | COL_RIGHT)) to_splash(g, o, 1);
}

static void cannon_think(Game *g, Obj *o)
{
    o->timer += TD;
    if (o->timer >= 128) {
        Obj *s;
        int tx = o->x >> 12, ty = o->y >> 12, ctx = g->cam_x >> 4, cty = g->cam_y >> 4;
        o->timer = 0;
        if (tx < ctx - 1 || tx > ctx + 21 || ty < cty - 1 || ty > cty + 13) return;   /* only while on screen */
        snd_play(g, 0x17);
        s = spawn(g, OT_ICESHOT, 112, o->x, o->y);
        if (s) {
            static const int vx[4] = {200, 0, 0, -200}, vy[4] = {-200, -200, 200, -200};
            s->xvel = vx[o->aux1 & 3]; s->yvel = vy[o->aux1 & 3];
        }
    }
}

static void iceshot_think(Game *g, Obj *o)
{
    int vc = obj_move(g, o);
    if (vc) shatter_ice(g, o);
}

static void chunk_think(Game *g, Obj *o)
{
    o->timer += TD;
    obj_gravity(o);
    obj_move(g, o);
    if (o->timer >= 60) o->type = OT_NONE;
}

static void splash_think(Game *g, Obj *o)
{
    (void)g;
    o->timer += TD;
    o->sprite = (o->timer >= 12) ? 111 : 110;
    if (o->timer >= 30) o->type = OT_NONE;
}

static void dying_think(Game *g, Obj *o)
{
    o->timer += TD;
    if (o->timer >= 40) {                    /* Death_Animate: one frame every 0x28 ticks */
        o->timer -= 40;
        if (o->aux1 > 0) { o->sprite++; o->aux1--; }
    }
    obj_gravity(o);
    obj_move(g, o);
}

/* ------------------------------------------------------------------ contacts */
static void kill_last_vorticon(Game *g)
{
    int i;
    for (i = MAX_OBJECTS - 1; i > 0; i--)
        if (g->obj[i].type == OT_VORT) { make_dying(g, &g->obj[i], 82, 6, 0x27); return; }
}

static void contact(Game *g, Obj *self, Obj *other, int otype)
{
    switch (self->type) {
    case OT_PLAYER: {
        Obj *p = self;
        switch (otype) {
        case OT_YORP:
            if (other->state == Y_STUN) return;
            if (p->yvel > 0 && (p->y + 0x800) < other->y) {            /* landed on its head */
                other->state = Y_STUN; other->timer = 0;
                p->yvel = 0; p->state = P_WALK; snd_play(g, 0x1F);
                return;
            }
            p->yvel = 0; p->xvel = other->xvel > 0 ? 240 : -240; snd_play(g, 0x1D);
            return;
        case OT_CAN: case OT_TANK:
            p->yvel = 0; p->xvel = other->xvel > 0 ? 240 : -240; snd_play(g, 0x1D);
            return;
        case OT_GARG: case OT_VORT:
            player_die(g, 0);
            return;
        case OT_TANKSHOT:
            if (p->state == P_FROZEN) { p->timer = 800; return; }
            player_die(g, 0);
            to_splash(g, other, 1);
            return;
        case OT_ICESHOT:
            p->state = P_FROZEN; p->xvel = other->xvel; p->yvel = other->yvel; p->timer = 800;
            snd_play(g, 0x28);
            shatter_ice(g, other);
            return;
        default: return;
        }
    }
    case OT_YORP:
        if (otype == OT_KEENSHOT || otype == OT_TANKSHOT) make_dying(g, self, 58, 2, 0x22);
        return;
    case OT_GARG:
        if (otype == OT_KEENSHOT || otype == OT_TANKSHOT) make_dying(g, self, 68, 2, 0x23);
        return;
    case OT_VORT:
        if (otype == OT_KEENSHOT || otype == OT_TANKSHOT) {
            if (self->hits == 0) make_dying(g, self, 82, 6, 0x27);
            else self->hits--;
        }
        return;
    case OT_CHAIN:
        if (otype == OT_KEENSHOT) {          /* the level-16 chain kills the Vorticon (STEP4) */
            snd_play(g, 0x27);
            kill_last_vorticon(g);
            self->type = OT_NONE;
        }
        return;
    case OT_KEENSHOT:
        if (otype >= OT_YORP && otype <= OT_TANK) to_splash(g, self, 1);
        else if (otype == OT_CHAIN) to_splash(g, self, 1);
        return;
    case OT_TANKSHOT:
        if (otype != OT_TANK && otype != OT_DYING && otype != OT_PLAYER &&
            otype >= OT_YORP && otype <= OT_VORT) to_splash(g, self, 1);
        return;
    default: return;
    }
}

/* ------------------------------------------------------------------ camera, clipping, drawing */
static void camera_limits(const Game *g, int *minx, int *maxx, int *miny, int *maxy)
{
    *minx = 2 * 16; *maxx = (g->lv.w - 22) * 16;
    *miny = 2 * 16; *maxy = (g->lv.h - 15) * 16 + 8;
    if (*maxx < *minx) *maxx = *minx;
    if (*maxy < *miny) *maxy = *miny;
}

static void camera_init(Game *g)
{
    const Obj *p = &g->obj[0];
    int minx, maxx, miny, maxy;
    camera_limits(g, &minx, &maxx, &miny, &maxy);
    g->cam_x = (int)clampi((p->x >> 8) - 160, minx, maxx);
    g->cam_y = (int)clampi((p->y >> 8) - 80, miny, maxy);
}

static void camera_update(Game *g)
{
    const Obj *p = &g->obj[0];
    int minx, maxx, miny, maxy, kx = ((p->x1 + p->x2) >> 9), ky = ((p->y1 + p->y2) >> 9);
    if (p->state == P_DEAD && p->aux1) return;
    camera_limits(g, &minx, &maxx, &miny, &maxy);
    if (kx - g->cam_x < 9 * 16) g->cam_x = kx - 9 * 16;
    if (kx - g->cam_x > 11 * 16) g->cam_x = kx - 11 * 16;
    if (ky - g->cam_y < 3 * 16) g->cam_y = ky - 3 * 16;
    if (ky - g->cam_y > 7 * 16) g->cam_y = ky - 7 * 16;
    g->cam_x = (int)clampi(g->cam_x, minx, maxx);
    g->cam_y = (int)clampi(g->cam_y, miny, maxy);
}

static void clip_objects(Game *g)       /* Obj_ClipToView */
{
    int i, ctx = g->cam_x >> 4, cty = g->cam_y >> 4;
    for (i = 1; i < MAX_OBJECTS; i++) {
        Obj *o = &g->obj[i];
        int tx, ty;
        if (!o->type) continue;
        tx = o->x >> 12; ty = o->y >> 12;
        if (o->x < 0 || o->y < 0 || tx >= g->lv.w || ty >= g->lv.h) { o->type = OT_NONE; continue; }
        if (tx < ctx - 8 || tx > ctx + 0x1C || ty < cty - 8 || ty > cty + 0x12) {
            if (o->type < 9 || o->type == OT_CANNON) o->active = 0;
            else o->type = OT_NONE;
        }
    }
}

static void draw_level(Game *g)
{
    int tx0 = g->cam_x >> 4, ty0 = g->cam_y >> 4, ox = -(g->cam_x & 15), oy = -(g->cam_y & 15), x, y, i;
    char buf[64];
    gfx_clear(g, 0);
    for (y = 0; y < 14; y++)
        for (x = 0; x < 21; x++) {
            int t = tile_at(&g->lv, tx0 + x, ty0 + y);
            if (t >= 0) gfx_tile16(g, t, ox + x * 16, oy + y * 16);
        }
    for (i = MAX_OBJECTS - 1; i >= 1; i--) {
        const Obj *o = &g->obj[i];
        if (o->type && o->active && o->type != OT_CANNON) gfx_sprite(g, o->sprite, (o->x >> 8) - g->cam_x, (o->y >> 8) - g->cam_y);
    }
    gfx_sprite(g, g->obj[0].sprite, (g->obj[0].x >> 8) - g->cam_x, (g->obj[0].y >> 8) - g->cam_y);
    /* status strip */
    gfx_rect(g, 0, 0, 160, 9, gfx_text_bg());
    snprintf(buf, sizeof buf, "%07u L%d A%02d%s", (unsigned)g->s.score, g->s.lives < 0 ? 0 : g->s.lives, g->s.ammo > 99 ? 99 : g->s.ammo, g->god ? " GOD" : "");
    gfx_text(g, 1, 1, buf);
    for (i = 0; i < 4; i++) {
        static const int kc[4] = {14, 12, 10, 9};
        if (g->s.key[i]) gfx_rect(g, 114 + i * 8, 1, 6, 7, kc[i]);
    }
    if (g->s.has_pogo) gfx_text(g, 148, 1, "P");
}

/* ------------------------------------------------------------------ main level loop */
static void activate_objects(Game *g)
{
    int i, ctx = g->cam_x >> 4, cty = g->cam_y >> 4;
    for (i = 1; i < MAX_OBJECTS; i++) {
        Obj *o = &g->obj[i];
        int tx, ty;
        if (!o->type || o->active) continue;
        tx = o->x >> 12; ty = o->y >> 12;
        if (tx > ctx - 2 && tx < ctx + 0x17 && ty > cty - 2 && ty < cty + 0xC) {
            o->active = 1;
            if (o->type == OT_YORP && o->state == Y_STUN) { o->state = Y_STAND; o->timer = 0; }
        }
    }
}

static void cheat_keys(Game *g)
{
    static int god_prev;
    int god_now = g->in.held['G'] && g->in.held['O'] && g->in.held['D'];
    if (god_now && !god_prev) { g->god = !g->god; snd_play(g, 0x19); }
    god_prev = god_now;
    if (g->in.held['C'] && g->in.held['T'] && g->in.held[' ']) {
        int i;
        g->s.has_pogo = 1; g->s.ammo = 100;
        for (i = 0; i < 4; i++) g->s.key[i] = 1;
    }
}

void level_player_reset_keys(Game *g) { memset(g->s.key, 0, sizeof g->s.key); }

int level_play(Game *g, int num)
{
    int i;
    if (level_load(&g->lv, num)) return -4;
    level_player_reset_keys(g);
    g->level_result = 0;
    g_player_done = 0;
    g_pending_hint = 0;
    spawn_level_objects(g);
    if (g->obj[0].type != OT_PLAYER) { level_free(&g->lv); return -4; }
    camera_init(g);
    activate_objects(g);
    snd_reset();

    for (;;) {
        Obj *p = &g->obj[0];
        draw_level(g);
        if (!frame_wait(g)) { level_free(&g->lv); return -1; }
        if (g->in.key == 27) {
            int r = ask_yn(g, "Leave this level?\nPress Y or N");
            if (r < 0) { level_free(&g->lv); return -1; }
            if (r) { level_free(&g->lv); return -3; }
        }
        if (g->in.key == 'P') { if (!msg_box(g, "Paused\n\nPress ENTER", 0)) { level_free(&g->lv); return -1; } }
        cheat_keys(g);

        activate_objects(g);
        for (i = 0; i < MAX_OBJECTS; i++) {
            Obj *o = &g->obj[i];
            if (!o->type) continue;
            if (i && !o->active) continue;
            o->dx = o->dy = 0;
            switch (o->type) {
            case OT_PLAYER:   player_think(g, o); break;
            case OT_YORP:     yorp_think(g, o); break;
            case OT_GARG:     garg_think(g, o); break;
            case OT_VORT:     vort_think(g, o); break;
            case OT_CAN:      robot_think(g, o, 88, 96, 90); break;
            case OT_TANK:     robot_think(g, o, 98, 106, 90); break;
            case OT_KEENSHOT: case OT_TANKSHOT: shot_move(g, o); break;
            case OT_ICESHOT:  iceshot_think(g, o); break;
            case OT_ICECHUNK: chunk_think(g, o); break;
            case OT_SPLASH:   splash_think(g, o); break;
            case OT_DYING:    dying_think(g, o); break;
            case OT_CANNON:   cannon_think(g, o); break;
            default: break;
            }
            obj_hitbox(o);
        }
        /* contacts between every pair of active objects */
        for (i = 0; i < MAX_OBJECTS; i++) {
            int j;
            Obj *a = &g->obj[i];
            if (!a->type || (i && !a->active) || a->type == OT_CANNON) continue;
            for (j = i + 1; j < MAX_OBJECTS; j++) {
                Obj *b = &g->obj[j];
                if (!b->type || !b->active || b->type == OT_CANNON) continue;
                if (!a->type) break;
                if (boxes_overlap(a, b)) {
                    int ta = a->type, tb = b->type;
                    contact(g, a, b, tb);
                    if (b->type) contact(g, b, a, ta);
                }
            }
        }
        touch_tiles(g);
        camera_update(g);
        clip_objects(g);
        if (g_pending_hint) {
            const char *t = hint_text(g->lv.num);
            g_pending_hint = 0;
            if (t) { draw_level(g); if (!msg_box(g, t, 0)) { level_free(&g->lv); return -1; } }
        }
        if (g->level_result) { int r = g->level_result; level_free(&g->lv); return r; }
        if (g_player_done) { level_free(&g->lv); return -2; }
        (void)p;
    }
}
