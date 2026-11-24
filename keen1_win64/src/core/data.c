/* Loads the original data files (EGAHEAD/EGALATCH/EGASPRIT/SOUNDS/LEVELnn .CK1) from a directory.
 * File layout: keen1_re README "File formats". */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ck1.h"

Assets A;

static uint32_t rd32(const uint8_t *p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24); }
static uint16_t rd16(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }

static const uint8_t EGA_PAL[16][3] = {
    {0,0,0},{0,0,170},{0,170,0},{0,170,170},{170,0,0},{170,0,170},{170,85,0},{170,170,170},
    {85,85,85},{85,85,255},{85,255,85},{85,255,255},{255,85,85},{255,85,255},{255,255,85},{255,255,255}};

uint32_t ega_color(int i) { return ((uint32_t)EGA_PAL[i & 15][0] << 16) | ((uint32_t)EGA_PAL[i & 15][1] << 8) | EGA_PAL[i & 15][2]; }

static void path_join(char *out, size_t cap, const char *dir, const char *name)
{
    size_t n = strlen(dir);
    if (n && (dir[n - 1] == '/' || dir[n - 1] == '\\')) snprintf(out, cap, "%s%s", dir, name);
    else snprintf(out, cap, "%s/%s", dir, name);
}

static uint8_t *load_file(const char *name, size_t *len)
{
    char p[1024];
    uint8_t *b;
    path_join(p, sizeof p, A.dir, name);
    b = file_read(p, len);
    if (!b) fprintf(stderr, "cannot read %s\n", p);
    return b;
}

/* 1 bit per plane per pixel, MSB = leftmost; planes are B,G,R,I (bit 0..3 of the colour index) */
static int planar_px(const uint8_t *const *pl, int np, long off, int bit)
{
    int v = 0, p;
    for (p = 0; p < np && p < 4; p++) v |= ((pl[p][off] >> bit) & 1) << p;
    return v;
}

static void render_sounds(void)
{
    size_t len = 0;
    uint8_t *b = load_file("SOUNDS.CK1", &len);
    int id;
    const int rate = 22050;
    if (!b) return;
    for (id = 1; id < 64; id++) {
        size_t rec = (size_t)id * 16;
        int off, n = 0, i, k;
        static uint16_t divs[8192];
        if (rec + 16 > len) break;
        off = rd16(b + rec);
        A.snd_prio[id] = b[rec + 2];
        if (off == 0 || (size_t)off + 2 > len) continue;
        while ((size_t)off + 2 * (size_t)n + 2 <= len && n < 8192) {
            uint16_t d = rd16(b + off + 2 * n);
            if (d == 0xFFFF) break;
            divs[n++] = d;
        }
        if (n == 0) continue;
        {
            double spt = rate / TICK_HZ;                    /* samples per timer tick */
            size_t ns = (size_t)(spt * n) + 1, o = 44, s;
            uint8_t *w = (uint8_t *)malloc(44 + ns * 2);
            double phase = 0;
            int16_t *pcm;
            if (!w) continue;
            memset(w, 0, 44);
            pcm = (int16_t *)(w + 44);
            for (s = 0; s < ns; s++) {
                int t = (int)((double)s / spt);
                int d;
                if (t >= n) t = n - 1;
                d = divs[t];
                if (d == 0) { pcm[s] = 0; phase = 0; continue; }
                phase += (1193182.0 / d) / rate;
                phase -= (int)phase;
                pcm[s] = (int16_t)(phase < 0.5 ? 5000 : -5000);
            }
            (void)o; (void)i; (void)k;
            memcpy(w, "RIFF", 4);
            { uint32_t v = (uint32_t)(36 + ns * 2); memcpy(w + 4, &v, 4); }
            memcpy(w + 8, "WAVEfmt ", 8);
            { uint32_t v = 16; memcpy(w + 16, &v, 4); }
            { uint16_t v = 1; memcpy(w + 20, &v, 2); memcpy(w + 22, &v, 2); }
            { uint32_t v = rate; memcpy(w + 24, &v, 4); v = rate * 2; memcpy(w + 28, &v, 4); }
            { uint16_t v = 2; memcpy(w + 32, &v, 2); v = 16; memcpy(w + 34, &v, 2); }
            memcpy(w + 36, "data", 4);
            { uint32_t v = (uint32_t)(ns * 2); memcpy(w + 40, &v, 4); }
            A.snd[id] = w;
            A.snd_len[id] = 44 + ns * 2;
            A.snd_ticks[id] = n;
        }
    }
    free(b);
}

int assets_load(const char *dir)
{
    size_t hl = 0, ll = 0, sl = 0;
    uint8_t *head, *lz, *sp, *lat = NULL, *spr = NULL;
    size_t lat_n, spr_n, lat_plane, spr_plane;
    uint32_t tile16_off, sprite_table, bitmap_table, bitmap_off;
    int n_bitmaps, n_sprites, i, x, y;
    const uint8_t *lp[4], *sprp[5];

    memset(&A, 0, sizeof A);
    snprintf(A.dir, sizeof A.dir, "%s", dir);
    head = load_file("EGAHEAD.CK1", &hl);
    lz = load_file("EGALATCH.CK1", &ll);
    sp = load_file("EGASPRIT.CK1", &sl);
    if (!head || !lz || !sp || hl < 0x150) { free(head); free(lz); free(sp); return 1; }

    lat_plane = rd32(head);
    spr_plane = rd32(head + 4);
    bitmap_table = rd32(head + 8);
    sprite_table = rd32(head + 12);
    tile16_off = rd32(head + 0x1E);
    n_bitmaps = rd16(head + 0x22);
    bitmap_off = rd32(head + 0x24);
    n_sprites = rd16(head + 0x28);
    if (lat_plane == 0 || lat_plane > 0x20000 || spr_plane == 0 || spr_plane > 0x20000 || n_sprites > NUM_SPRITES) {
        fprintf(stderr, "EGAHEAD.CK1 header looks wrong (latch %u, sprite %u, %d sprites)\n", (unsigned)lat_plane, (unsigned)spr_plane, n_sprites);
        free(head); free(lz); free(sp);
        return 1;
    }

    lat = (uint8_t *)calloc(1, lat_plane * 4 + 16);
    spr = (uint8_t *)calloc(1, spr_plane * 5 + 16);
    lat_n = lzw_decode(lz, ll, lat, lat_plane * 4);
    spr_n = lzw_decode(sp, sl, spr, spr_plane * 5);
    if (lat_n < lat_plane * 4 || spr_n < spr_plane * 5) {
        fprintf(stderr, "EGALATCH/EGASPRIT decoded to %u/%u bytes, expected %u/%u\n", (unsigned)lat_n, (unsigned)spr_n,
                (unsigned)(lat_plane * 4), (unsigned)(spr_plane * 5));
        free(head); free(lz); free(sp); free(lat); free(spr);
        return 1;
    }
    for (i = 0; i < 4; i++) lp[i] = lat + lat_plane * i;
    for (i = 0; i < 5; i++) sprp[i] = spr + spr_plane * i;

    /* 8x8 bank: 256 tiles at plane offset 0, 8 bytes per tile per plane */
    for (i = 0; i < 256; i++)
        for (y = 0; y < 8; y++)
            for (x = 0; x < 8; x++)
                A.font8[i][y * 8 + x] = (uint8_t)planar_px(lp, 4, i * 8 + y, 7 - x);

    /* 16x16 tiles from plane offset tile16_off, 32 bytes per tile per plane */
    for (i = 0; i < NUM_TILES; i++) {
        long base = (long)tile16_off + i * 32;
        if ((size_t)base + 32 > lat_plane) break;
        for (y = 0; y < 16; y++)
            for (x = 0; x < 16; x++)
                A.tiles16[i][y * 16 + x] = (uint8_t)planar_px(lp, 4, base + y * 2 + x / 8, 7 - (x % 8));
    }

    /* sprites: shift-0 record of each sprite (record = sprite*4) */
    for (i = 0; i < n_sprites; i++) {
        size_t rec = sprite_table + 32 * (size_t)(i * 4);
        int w, h;
        long base;
        Sprite *s = &A.spr[i];
        if (rec + 32 > hl) break;
        w = rd16(head + rec);
        h = rd16(head + rec + 2);
        base = (long)rd16(head + rec + 4) + 16L * rd16(head + rec + 6);
        s->w = w * 8;
        s->h = h;
        s->hx1 = (int16_t)rd16(head + rec + 8);
        s->hy1 = (int16_t)rd16(head + rec + 10);
        s->hx2 = (int16_t)rd16(head + rec + 12);
        s->hy2 = (int16_t)rd16(head + rec + 14);
        if (w <= 0 || h <= 0 || w > 16 || h > 200 || (size_t)base + (size_t)w * h > spr_plane) { s->w = s->h = 0; continue; }
        s->pix = (uint8_t *)malloc((size_t)s->w * s->h);
        for (y = 0; y < h; y++)
            for (x = 0; x < s->w; x++) {
                long o = base + (long)y * w + x / 8;
                int bit = 7 - (x % 8);
                s->pix[y * s->w + x] = ((sprp[4][o] >> bit) & 1) ? 255 : (uint8_t)planar_px(sprp, 4, o, bit);
            }
    }

    /* TITLE bitmap (bitmap table record 0) */
    for (i = 0; i < n_bitmaps; i++) {
        size_t rec = bitmap_table + 16 * (size_t)i;
        if (rec + 16 > hl) break;
        if (memcmp(head + rec + 8, "TITLE", 5) == 0) {
            int w = rd16(head + rec), h = rd16(head + rec + 2);
            long base = (long)bitmap_off + (long)rd32(head + rec + 4);
            if (w > 0 && h > 0 && (size_t)base + (size_t)w * h <= lat_plane) {
                A.title_w = w * 8;
                A.title_h = h;
                A.title = (uint8_t *)malloc((size_t)A.title_w * A.title_h);
                for (y = 0; y < h; y++)
                    for (x = 0; x < A.title_w; x++)
                        A.title[y * A.title_w + x] = (uint8_t)planar_px(lp, 4, base + (long)y * w + x / 8, 7 - (x % 8));
            }
        }
    }

    free(head); free(lz); free(sp); free(lat); free(spr);
    render_sounds();
    return 0;
}

void assets_free(void)
{
    int i;
    for (i = 0; i < NUM_SPRITES; i++) free(A.spr[i].pix);
    for (i = 0; i < 64; i++) free(A.snd[i]);
    free(A.title);
    memset(&A, 0, sizeof A);
}

int level_load(Level *lv, int num)
{
    char name[32];
    size_t len = 0, n, cap = 0x40000;
    uint8_t *f, *body;
    int pb;
    snprintf(name, sizeof name, "LEVEL%02d.CK1", num);
    f = load_file(name, &len);
    if (!f) return 1;
    body = (uint8_t *)calloc(1, cap);
    n = rlew_decode(f, len, body, cap);
    free(f);
    if (n < 0x20) { free(body); return 1; }
    lv->w = rd16(body);
    lv->h = rd16(body + 2);
    pb = rd16(body + 0x0E);
    lv->num = num;
    if (lv->w <= 0 || lv->h <= 0 || lv->w * lv->h * 2 > pb || (size_t)0x20 + 2 * (size_t)pb > n) {
        fprintf(stderr, "%s: bad header (%d x %d, plane %d, decoded %u)\n", name, lv->w, lv->h, pb, (unsigned)n);
        free(body);
        return 1;
    }
    lv->p0 = (uint16_t *)malloc((size_t)lv->w * lv->h * 2);
    lv->p1 = (uint16_t *)malloc((size_t)lv->w * lv->h * 2);
    memcpy(lv->p0, body + 0x20, (size_t)lv->w * lv->h * 2);
    memcpy(lv->p1, body + 0x20 + pb, (size_t)lv->w * lv->h * 2);
    free(body);
    return 0;
}

void level_free(Level *lv)
{
    free(lv->p0);
    free(lv->p1);
    lv->p0 = lv->p1 = NULL;
}
