#!/usr/bin/env python3
"""
Generate a creative 256×256 GTASCII world and split into 64×64 chunk files.

Glyph palette (ASCII-friendly, single-cell):
  .  grass          ,  dirt           '  sparse grass
  =  road           -  sidewalk
  #  building wall  H  house body     +  closed door
  T  tree           Y  bush           *  flower
  ~  water          ≈  deep water (approx with ~)
  o  rock           ^  hill

Block types:
  0 normal, 1 solid, 2 water, 3 door (closed)
"""

from __future__ import annotations
import os
from pathlib import Path

W, H = 256, 256
CHUNK = 64
OUT = Path(__file__).resolve().parent.parent / "assets" / "map"

# glyph, color (0-8), type
G = lambda ch, c, t: (ch, c, t)

def make_empty():
    g = [['.' for _ in range(W)] for _ in range(H)]
    c = [[1 for _ in range(W)] for _ in range(H)]
    t = [[0 for _ in range(W)] for _ in range(H)]
    return g, c, t

def fill_rect(g, c, t, x0, y0, x1, y1, glyph, color, typ):
    for y in range(max(0, y0), min(H, y1)):
        for x in range(max(0, x0), min(W, x1)):
            g[y][x] = glyph
            c[y][x] = color
            t[y][x] = typ

def put(g, c, t, x, y, glyph, color, typ):
    if 0 <= x < W and 0 <= y < H:
        g[y][x] = glyph
        c[y][x] = color
        t[y][x] = typ

def road_h(g, c, t, y, x0, x1, width=3):
    for dy in range(-(width // 2), width // 2 + 1):
        fill_rect(g, c, t, x0, y + dy, x1, y + dy + 1, '=', 2, 0)

def road_v(g, c, t, x, y0, y1, width=3):
    for dx in range(-(width // 2), width // 2 + 1):
        fill_rect(g, c, t, x + dx, y0, x + dx + 1, y1, '=', 2, 0)

def building(g, c, t, x, y, w, h, door_side='s'):
    """Solid rectangle with a door on one side. Can cross chunk borders."""
    fill_rect(g, c, t, x, y, x + w, y + h, '#', 3, 1)
    # interior floor
    if w > 2 and h > 2:
        fill_rect(g, c, t, x + 1, y + 1, x + w - 1, y + h - 1, ',', 2, 0)
    # door
    if door_side == 's':
        dx, dy = x + w // 2, y + h - 1
    elif door_side == 'n':
        dx, dy = x + w // 2, y
    elif door_side == 'e':
        dx, dy = x + w - 1, y + h // 2
    else:
        dx, dy = x, y + h // 2
    put(g, c, t, dx, dy, '+', 5, 3)

def house(g, c, t, x, y, w=7, h=5):
    building(g, c, t, x, y, w, h, 's')
    # little roof hint
    for i in range(w):
        put(g, c, t, x + i, y, 'H', 3, 1)

def scatter_trees(g, c, t, x0, y0, x1, y1, density=0.08, seed=0):
    import random
    rng = random.Random(seed)
    for y in range(y0, y1):
        for x in range(x0, x1):
            if g[y][x] != '.':
                continue
            r = rng.random()
            if r < density * 0.4:
                put(g, c, t, x, y, 'T', 1, 1)  # solid trunk
            elif r < density:
                put(g, c, t, x, y, 'Y', 1, 0)  # bush, walkable
            elif r < density + 0.03:
                put(g, c, t, x, y, '*', 8, 0)  # flower

def water_blob(g, c, t, cx, cy, rx, ry):
    for y in range(max(0, cy - ry), min(H, cy + ry + 1)):
        for x in range(max(0, cx - rx), min(W, cx + rx + 1)):
            dx = (x - cx) / max(1, rx)
            dy = (y - cy) / max(1, ry)
            if dx * dx + dy * dy <= 1.0:
                put(g, c, t, x, y, '~', 4, 2)
            if dx * dx + dy * dy <= 0.45:
                put(g, c, t, x, y, '~', 7, 2)  # deeper look

def main():
    g, c, t = make_empty()

    # --- coast / sea on the south-east ---
    for y in range(H):
        for x in range(W):
            # diagonal coastline
            if x + y > 340:
                put(g, c, t, x, y, '~', 4, 2)
            if x + y > 380:
                put(g, c, t, x, y, '~', 7, 2)

    # lakes
    water_blob(g, c, t, 40, 200, 22, 14)
    water_blob(g, c, t, 180, 50, 18, 12)
    water_blob(g, c, t, 100, 100, 10, 8)

    # --- main road grid (spans multiple chunks) ---
    # horizontal arterials
    for y in (32, 96, 160, 224):
        road_h(g, c, t, y, 0, W, width=3)
    # vertical arterials
    for x in (32, 96, 160, 224):
        road_v(g, c, t, x, 0, H, width=3)

    # secondary streets
    for y in (64, 128, 192):
        road_h(g, c, t, y, 0, W, width=1)
    for x in (64, 128, 192):
        road_v(g, c, t, x, 0, H, width=1)

    # --- city blocks / buildings (some intentionally cross chunk borders) ---
    # NW residential
    for i, (bx, by) in enumerate([(8, 8), (16, 8), (8, 16), (48, 48), (55, 40),
                                   (70, 10), (80, 20), (12, 50)]):
        house(g, c, t, bx, by, 7 + (i % 3), 5 + (i % 2))

    # large warehouse spanning chunks 0_0 / 1_0 (x around 60-80)
    building(g, c, t, 55, 70, 40, 18, door_side='s')  # crosses x=64

    # downtown strip
    for i in range(6):
        building(g, c, t, 110 + i * 12, 110, 10, 14, door_side='s')
    # tall office crossing chunk border at y=128
    building(g, c, t, 140, 118, 16, 28, door_side='w')

    # east industrial (near water)
    building(g, c, t, 200, 140, 24, 20, door_side='w')
    building(g, c, t, 210, 180, 18, 12, door_side='n')

    # south market hall spanning y=192
    building(g, c, t, 80, 185, 30, 16, door_side='n')

    # --- parks ---
    scatter_trees(g, c, t, 0, 0, 60, 60, density=0.12, seed=1)
    scatter_trees(g, c, t, 170, 0, 256, 60, density=0.1, seed=2)
    scatter_trees(g, c, t, 0, 170, 80, 256, density=0.09, seed=3)
    scatter_trees(g, c, t, 100, 100, 150, 150, density=0.05, seed=4)

    # rocks / hills near coast
    import random
    rng = random.Random(42)
    for _ in range(80):
        x = rng.randint(180, 255)
        y = rng.randint(180, 255)
        if t[y][x] == 2:
            continue
        put(g, c, t, x, y, 'o', 2, 1)

    # clear a small spawn plaza
    fill_rect(g, c, t, 15, 15, 30, 30, '.', 1, 0)
    put(g, c, t, 22, 22, '*', 8, 0)

    # --- write chunks ---
    OUT.mkdir(parents=True, exist_ok=True)
    for cy in range(H // CHUNK):
        for cx in range(W // CHUNK):
            path = OUT / f"{cx}_{cy}.txt"
            lines = ["GLYPHS"]
            for ly in range(CHUNK):
                row = ''.join(g[cy * CHUNK + ly][cx * CHUNK: cx * CHUNK + CHUNK])
                lines.append(row)
            lines.append("COLORS")
            for ly in range(CHUNK):
                row = ''.join(str(c[cy * CHUNK + ly][cx * CHUNK + lx]) for lx in range(CHUNK))
                lines.append(row)
            lines.append("TYPES")
            for ly in range(CHUNK):
                row = ''.join(str(t[cy * CHUNK + ly][cx * CHUNK + lx]) for lx in range(CHUNK))
                lines.append(row)
            path.write_text('\n'.join(lines) + '\n', encoding='utf-8')
            print(f"wrote {path.name}")

    print(f"Done — world {W}x{H}, chunks {CHUNK}x{CHUNK}, grid {W//CHUNK}x{H//CHUNK}")

if __name__ == "__main__":
    main()
