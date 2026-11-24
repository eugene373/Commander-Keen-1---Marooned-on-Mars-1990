#!/usr/bin/env python3
"""Writes a SYNTHETIC Keen-1-format data set (not the real game) for smoke-testing the engine without the original files.
Uses the verified formats from keen1_re. usage: make_test_data.py outdir"""
import struct, sys, os
sys.path.insert(0, sys.argv[2] if len(sys.argv) > 2 else '/home/claude/k/keen1_re/tools')
from ck1fmt import rlew_encode, lzw_encode
out = sys.argv[1]; os.makedirs(out, exist_ok=True)
LP, SP = 29920, 6320
def planes_put(pl, base, rows, w, colors):   # rows of colour indices (len w*8 each)
    for y, row in enumerate(rows):
        for x, c in enumerate(row):
            if c == 255: continue
            for p in range(4):
                if (c >> p) & 1: pl[p][base + y*w + x//8] |= 0x80 >> (x % 8)
lat = [bytearray(LP) for _ in range(4)]
# 8x8 font: space = white, other chars = black blocks pattern on white
for n in range(256):
    for y in range(8):
        for p in range(4): lat[p][n*8+y] = 0xFF
        if 0x21 <= n < 0x7F and 1 <= y <= 6:
            bits = ((n*37 + y*91) | 0x18) & 0x7E
            for p in range(4): lat[p][n*8+y] = (~bits) & 0xFF
# 16x16 tiles
for n in range(611):
    c = (n % 14) + 1
    for y in range(16):
        row = [c if (x+y) % 7 else 0 for x in range(16)] if n != 143 else [1]*16
        for p in range(4):
            for xb in range(2):
                v = 0
                for b in range(8):
                    if (row[xb*8+b] >> p) & 1: v |= 0x80 >> b
                lat[p][0x800 + n*32 + y*2 + xb] = v
for y in range(112):      # TITLE bitmap 33 bytes wide at 21600
    for p in range(4):
        for xb in range(33): lat[p][21600 + y*33 + xb] = 0xAA if (y//8) % 2 == 0 else 0x55
spr = [bytearray(SP) for _ in range(5)]
for i in range(119):
    h = 24 if i < 48 else 16
    for y in range(h):
        for xb in range(2):
            spr[4][i*48 + y*2 + xb] = 0xFF if (y < 2 or y >= h-1) else 0x00
            for p in range(4):
                spr[p][i*48 + y*2 + xb] = 0xFF if (i % 15 + 1) >> p & 1 else 0
head = bytearray(0x150 + 476*32)
struct.pack_into('<II', head, 0, LP, SP); struct.pack_into('<II', head, 8, 0x30, 0x150)
struct.pack_into('<H', head, 0x10, 256); struct.pack_into('<H', head, 0x1c, 611); struct.pack_into('<I', head, 0x1e, 0x800)
struct.pack_into('<H', head, 0x22, 1); struct.pack_into('<I', head, 0x24, 21600); struct.pack_into('<H', head, 0x28, 119)
struct.pack_into('<HHI', head, 0x30, 33, 112, 0); head[0x38:0x38+5] = b'TITLE'
for i in range(119):
    h = 24 if i < 48 else 16
    struct.pack_into('<HHHH', head, 0x150 + 32*(i*4), 2, h, i*48, 0)
    struct.pack_into('<hhhh', head, 0x150 + 32*(i*4) + 8, 1024, 256, 2816, (h-1)*256)
open(out+'/EGAHEAD.CK1','wb').write(head)
open(out+'/EGALATCH.CK1','wb').write(lzw_encode(b''.join(bytes(p) for p in lat)))
open(out+'/EGASPRIT.CK1','wb').write(lzw_encode(b''.join(bytes(p) for p in spr)))
# SOUNDS: 63 records, data after 64*16
snd = bytearray(64*16); data = bytearray()
for i in range(1, 64):
    snd[i*16:i*16+2] = struct.pack('<H', 64*16 + len(data)); snd[i*16+2] = 100 + i
    for k in range(6): data += struct.pack('<H', 3000 + i*40 + k*100)
    data += struct.pack('<H', 0xFFFF)
open(out+'/SOUNDS.CK1','wb').write(bytes(snd + data))
# levels: SOLID tile with all four collision flags (taken from the real tables), EMPTY = 143
import csv
rows = list(csv.DictReader(open('/home/claude/k/keen1_re/step5/tile_attrs.csv')))
solid = next(int(r['tile']) for r in rows if all(int(r[k]) == 1 for k in ('top','left','bottom','right')) and int(r['special']) == -1)
def level(name, w, h, tiles, objs):
    pb = (w*h*2 + 15)//16*16
    p0 = [143]*(w*h); p1 = [0]*(w*h)
    for (x, y, t) in tiles: p0[y*w+x] = t
    for (x, y, v) in objs: p1[y*w+x] = v
    body = struct.pack('<3H', w, h, 2) + b'\0'*(0x0E-6) + struct.pack('<H', pb) + b'\0'*(0x20-0x10)
    body += struct.pack('<%dH' % (w*h), *p0) + b'\0'*(pb-w*h*2) + struct.pack('<%dH' % (w*h), *p1) + b'\0'*(pb-w*h*2)
    open(out+'/'+name,'wb').write(rlew_encode(body))
W, H = 70, 30
t = [(x, y, solid) for x in range(W) for y in (0, 1, 27, 28, 29)] + [(x, y, solid) for y in range(H) for x in (0, 1, W-2, W-1)]
t += [(x, 23, solid) for x in range(30, 36)]                 # a ledge
t += [(12, 26, 201), (14, 26, 201), (16, 26, 159)]           # pizza, pizza, exit door tile (special 17)
t += [(20, 26, 169)]                                         # hazard
objs = [(5, 26, 255), (18, 26, 1), (26, 26, 2), (40, 25, 3), (50, 26, 4), (56, 26, 5)]
level('LEVEL01.CK1', W, H, t, objs)
level('LEVEL80.CK1', 40, 40, [], [(5, 5, 255), (10, 10, 1)])
print('solid tile', solid)
