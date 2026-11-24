#ifndef GAME_INT_H
#define GAME_INT_H
#include "ck1.h"

uint32_t ega_color(int i);

/* render.c */
void gfx_clear(Game *g, int c);
void gfx_rect(Game *g, int x, int y, int w, int h, int c);
void gfx_tile16(Game *g, int tile, int x, int y);
void gfx_sprite(Game *g, int spr, int x, int y);
void gfx_bitmap(Game *g, const uint8_t *pix, int w, int h, int x, int y);
void gfx_text(Game *g, int x, int y, const char *s);
int  gfx_text_bg(void);
void gfx_box(Game *g, int x, int y, int w, int h);
void gfx_flip(Game *g);

/* game.c helpers */
void snd_play(Game *g, int id);
void snd_reset(void);
int  rnd(Game *g, int n);
void score_add(Game *g, int pts);
/* One frame of presentation + input; returns 0 if the window was closed. Sleeps to hold the 24.3 fps frame rate. */
int  frame_wait(Game *g);
/* Modal message box with word-wrapped text; waits for ENTER/SPACE/ESC. Returns 0 if the window was closed. */
int  msg_box(Game *g, const char *text, int dim_background);
/* Yes/No prompt (Y / N / ESC). Returns 1 yes, 0 no, -1 if the window was closed. */
int  ask_yn(Game *g, const char *text);

/* play.c */
int  level_play(Game *g, int num);          /* returns Game.level_result, or -1 if the window was closed, -2 if Keen died */
void level_player_reset_keys(Game *g);
int  obj_move_map(Game *g, Obj *o);        /* tile collision for the world map (uses g->lv) */

/* map.c */
void worldmap_reset(void);                  /* forget the map (new game) */
int  worldmap_run(Game *g);                 /* returns 1 = continue game (finale reached), 0 = game over, -1 = window closed */

#endif
