/* 320x200 indexed-colour software renderer (the EGA mode 0Dh screen of the original). */
#include <string.h>
#include "game_int.h"

void gfx_clear(Game *g, int c) { memset(g->idx, c & 15, sizeof g->idx); }

void gfx_rect(Game *g, int x, int y, int w, int h, int c)
{
    int xx, yy;
    for (yy = y; yy < y + h; yy++) {
        if (yy < 0 || yy >= SCR_H) continue;
        for (xx = x; xx < x + w; xx++)
            if (xx >= 0 && xx < SCR_W) g->idx[yy * SCR_W + xx] = (uint8_t)c;
    }
}

void gfx_tile16(Game *g, int tile, int x, int y)
{
    int yy, xx;
    const uint8_t *t;
    if (tile < 0 || tile >= NUM_TILES) return;
    t = A.tiles16[tile];
    for (yy = 0; yy < 16; yy++) {
        int sy = y + yy;
        if (sy < 0 || sy >= SCR_H) continue;
        for (xx = 0; xx < 16; xx++) {
            int sx = x + xx;
            if (sx >= 0 && sx < SCR_W) g->idx[sy * SCR_W + sx] = t[yy * 16 + xx];
        }
    }
}

void gfx_sprite(Game *g, int spr, int x, int y)
{
    int yy, xx;
    const Sprite *s;
    if (spr < 0 || spr >= NUM_SPRITES) return;
    s = &A.spr[spr];
    if (!s->pix) return;
    for (yy = 0; yy < s->h; yy++) {
        int sy = y + yy;
        if (sy < 0 || sy >= SCR_H) continue;
        for (xx = 0; xx < s->w; xx++) {
            uint8_t c = s->pix[yy * s->w + xx];
            int sx = x + xx;
            if (c != 255 && sx >= 0 && sx < SCR_W) g->idx[sy * SCR_W + sx] = c;
        }
    }
}

void gfx_bitmap(Game *g, const uint8_t *pix, int w, int h, int x, int y)
{
    int yy, xx;
    if (!pix) return;
    for (yy = 0; yy < h; yy++) {
        int sy = y + yy;
        if (sy < 0 || sy >= SCR_H) continue;
        for (xx = 0; xx < w; xx++) {
            int sx = x + xx;
            if (sx >= 0 && sx < SCR_W) g->idx[sy * SCR_W + sx] = pix[yy * w + xx];
        }
    }
}

/* The original draws text with 8x8 tiles whose number is the ASCII code. */
int gfx_text_bg(void) { return A.font8[0x20][0]; }

void gfx_text(Game *g, int x, int y, const char *s)
{
    for (; *s; s++, x += 8) {
        int yy, xx;
        const uint8_t *t = A.font8[(unsigned char)*s];
        for (yy = 0; yy < 8; yy++) {
            int sy = y + yy;
            if (sy < 0 || sy >= SCR_H) continue;
            for (xx = 0; xx < 8; xx++) {
                int sx = x + xx;
                if (sx >= 0 && sx < SCR_W) g->idx[sy * SCR_W + sx] = t[yy * 8 + xx];
            }
        }
    }
}

/* Dialog box: filled with the font's background colour and framed in black. */
void gfx_box(Game *g, int x, int y, int w, int h)
{
    int bg = gfx_text_bg();
    gfx_rect(g, x - 2, y - 2, w + 4, h + 4, 0);
    gfx_rect(g, x, y, w, h, bg);
    gfx_rect(g, x + 2, y + 2, w - 4, 1, 0);
    gfx_rect(g, x + 2, y + h - 3, w - 4, 1, 0);
    gfx_rect(g, x + 2, y + 2, 1, h - 4, 0);
    gfx_rect(g, x + w - 3, y + 2, 1, h - 4, 0);
}

void gfx_flip(Game *g)
{
    int i;
    uint32_t pal[16];
    for (i = 0; i < 16; i++) pal[i] = ega_color(i);
    for (i = 0; i < SCR_W * SCR_H; i++) g->pix[i] = pal[g->idx[i] & 15];
    plat_present(g->pix);
}
