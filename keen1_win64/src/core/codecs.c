/* RLEW (levels) and LZW (EGALATCH / EGASPRIT) decoders.  Formats as verified in keen1_re (STEP9/STEP12):
 * the LZW streams are MSB-first, 9-bit start, no clear code in the shipped files, the code width grows when the
 * next free entry reaches (1<<bits)-1, and the entry counter keeps running after the 4096-entry table is full. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ck1.h"

static uint32_t rd32(const uint8_t *p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24); }
static uint16_t rd16(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }

uint8_t *file_read(const char *path, size_t *len)
{
    FILE *f = fopen(path, "rb");
    uint8_t *b;
    long n;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (n < 0) { fclose(f); return NULL; }
    b = (uint8_t *)malloc((size_t)n + 16);
    if (!b) { fclose(f); return NULL; }
    if (fread(b, 1, (size_t)n, f) != (size_t)n) { fclose(f); free(b); return NULL; }
    memset(b + n, 0, 16);
    fclose(f);
    if (len) *len = (size_t)n;
    return b;
}

/* u32 size, then words; 0xFEFE count value = run of `count` copies of `value`. */
size_t rlew_decode(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_cap)
{
    size_t size, i = 4, o = 0;
    if (in_len < 4) return 0;
    size = rd32(in);
    if (size > out_cap) size = out_cap;
    while (o + 1 < size && i + 2 <= in_len) {
        uint16_t w = rd16(in + i);
        i += 2;
        if (w == 0xFEFE) {
            uint16_t c, v;
            if (i + 4 > in_len) break;
            c = rd16(in + i); v = rd16(in + i + 2);
            i += 4;
            while (c-- && o + 1 < size) { out[o++] = (uint8_t)(v & 255); out[o++] = (uint8_t)(v >> 8); }
        } else {
            out[o++] = (uint8_t)(w & 255);
            out[o++] = (uint8_t)(w >> 8);
        }
    }
    return o;
}

static uint16_t g_prefix[4096];
static uint8_t  g_suffix[4096];
static uint8_t  g_stack[4097];

/* u32 size, u16 max_bits, then the code stream. */
size_t lzw_decode(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_cap)
{
    size_t size, p = 6, o = 0;
    int maxbits, nb = 9, nxt = 0x102, prev = -1;
    uint32_t bb = 0;
    int bc = 0;
    uint8_t first = 0;
    if (in_len < 6) return 0;
    size = rd32(in);
    maxbits = rd16(in + 4);
    if (maxbits < 9 || maxbits > 12) maxbits = 12;
    if (size > out_cap) size = out_cap;
    while (o < size) {
        int c, cur, sp = 0;
        uint8_t f;
        while (bc < nb) {
            bb = (bb << 8) | (p < in_len ? in[p] : 0);
            p++;
            bc += 8;
            if (p > in_len + 8) return o;       /* ran off the stream */
        }
        c = (int)((bb >> (bc - nb)) & ((1u << nb) - 1));
        bc -= nb;
        bb &= (1u << bc) - 1;
        if (c == 0x101) break;
        if (c == 0x100) { nb = 9; nxt = 0x102; prev = -1; continue; }
        cur = c;
        if (prev >= 0 && c >= nxt) { g_stack[sp++] = first; cur = prev; }
        while (cur >= 0x102 && sp < 4096) { g_stack[sp++] = g_suffix[cur]; cur = g_prefix[cur]; }
        g_stack[sp++] = (uint8_t)cur;
        f = g_stack[sp - 1];
        if (prev >= 0) {
            if (nxt < 4096) { g_prefix[nxt] = (uint16_t)prev; g_suffix[nxt] = f; }
            nxt++;
            if (nxt >= (1 << nb) - 1 && nb < maxbits) nb++;
        }
        while (sp && o < size) out[o++] = g_stack[--sp];
        first = f;
        prev = c;
    }
    return o;
}
