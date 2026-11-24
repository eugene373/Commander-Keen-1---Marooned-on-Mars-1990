/* Commander Keen 1 (Marooned on Mars, v1.31) -- native 64-bit port.
 *
 * Built on the formats and game rules recovered in the keen1_re project (README.md / STEP*.md).
 * Positions and velocities use the original fixed-point units: 256 units = 1 pixel, 0x1000 = one 16x16 tile. */
#ifndef CK1_H
#define CK1_H

#include <stdint.h>
#include <stddef.h>

#define SCR_W 320
#define SCR_H 200
#define NUM_TILES 611
#define NUM_SPRITES 119
#define MAX_OBJECTS 160
#define TICK_HZ 145.65          /* original timer interrupt rate */
#define TICK_DELTA 6            /* ticks per game frame on a 286+ (24.3 fps), see keen1_re STEP20 */

/* ------------------------------------------------------------------ per-tile tables (tiletab.c, generated) */
extern const unsigned short TILE_SPAN[NUM_TILES], TILE_SPECIAL[NUM_TILES], TILE_FLOOR[NUM_TILES],
                            TILE_BLOCK_L[NUM_TILES], TILE_CEIL[NUM_TILES], TILE_BLOCK_R[NUM_TILES];

/* ------------------------------------------------------------------ platform layer (plat_win32.c / plat_null.c) */
typedef struct Input {
    int dir;                    /* 0=N 1=NE 2=E 3=SE 4=S 5=SW 6=W 7=NW 8=none (as Input_Poll in the original) */
    int jump, pogo;             /* both together = shoot */
    int key;                    /* last key pressed this frame (ASCII upper-case, 13 enter, 27 esc, 0 none) */
    uint8_t held[256];          /* held state by virtual key / ASCII code */
    int quit;                   /* window closed */
} Input;

int      plat_init(const char *title);
void     plat_shutdown(void);
int      plat_poll(Input *in);                   /* pump messages, fill input; returns 0 when the window was closed */
void     plat_present(const uint32_t *xrgb);     /* SCR_W x SCR_H pixels */
int64_t  plat_time_us(void);
void     plat_sleep_ms(int ms);
void     plat_play_wav(const uint8_t *wav, size_t len);   /* asynchronous, replaces the sound that is playing */
const char *plat_exe_dir(void);
void     plat_error(const char *msg);            /* message box / stderr */

/* ------------------------------------------------------------------ assets (data.c) */
typedef struct Sprite {
    int w, h;                   /* pixels */
    uint8_t *pix;               /* w*h palette indices, 255 = transparent */
    int hx1, hy1, hx2, hy2;     /* hitbox in 1/256 px relative to the sprite origin */
} Sprite;

typedef struct Assets {
    uint8_t tiles16[NUM_TILES][256];
    uint8_t font8[256][64];                  /* 8x8 tiles from the latch bank (ASCII font, HUD icons) */
    Sprite  spr[NUM_SPRITES];
    uint8_t *title;  int title_w, title_h;   /* TITLE bitmap, palette indices */
    uint8_t *snd[64]; size_t snd_len[64];    /* pre-rendered WAV files, index = sound id */
    int     snd_prio[64];
    int     snd_ticks[64];
    char    dir[512];
} Assets;

extern Assets A;
int  assets_load(const char *dir);           /* returns 0 on success, prints the reason on failure */
void assets_free(void);

typedef struct Level {
    int w, h, num;
    uint16_t *p0, *p1;          /* tile plane, object plane */
} Level;
int  level_load(Level *lv, int num);
void level_free(Level *lv);

/* codecs (codecs.c) */
size_t rlew_decode(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_cap);
size_t lzw_decode(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_cap);
uint8_t *file_read(const char *path, size_t *len);

/* ------------------------------------------------------------------ game (game.c) */
enum { COL_LEFT = 1, COL_FLOOR = 2, COL_RIGHT = 4, COL_CEIL = 8 };

enum ObjType { OT_NONE = 0, OT_PLAYER = 1, OT_YORP = 2, OT_GARG = 3, OT_VORT = 4, OT_CAN = 5, OT_TANK = 6,
               OT_CHAIN = 8, OT_KEENSHOT = 0x0A, OT_TANKSHOT = 0x0B, OT_ICECHUNK = 0x0C, OT_SPLASH = 0x0D,
               OT_ICESHOT = 0x0F, OT_CANNON = 0x20 };

typedef struct Obj {
    int type, active;
    int32_t x, y;               /* sprite origin, 1/256 px */
    int32_t x1, y1, x2, y2;     /* hitbox */
    int dx, dy, xvel, yvel;
    int hits, sprite, timer, aux1, aux2, aux3;
    int state;
    int facing;                 /* 1 right, -1 left */
} Obj;

typedef struct Save {           /* persists between levels (DS:0xAA94 block of the original) */
    int part[4];                /* joystick, battery, vacuum, everclear */
    int key[4];                 /* yellow, red, green, blue (cleared at every level start) */
    int has_pogo, ammo, lives;
    int level_done[16];
    uint32_t score;
    uint32_t next_life;
} Save;

typedef struct Game {
    Assets *a;
    Save s;
    Level lv;
    Obj obj[MAX_OBJECTS];
    int level_result;           /* 0 running, 1 exit, 2 teleporter tile, 3 unused */
    int cam_x, cam_y;           /* pixels */
    int god;
    int frame;
    uint32_t rng;
    uint32_t pix[SCR_W * SCR_H];
    uint8_t  idx[SCR_W * SCR_H];
    Input in, prev;
} Game;

int  game_run(const char *datadir);

#endif
