#!/usr/bin/env python3
"""
GTASCII map splitter
--------------------
Reads a large map.txt (GLYPHS + COLORS + optional TYPES) and
splits it into 64×64 chunk files that the game can load.

Input format (map.txt)
----------------------
GLYPHS
<H lines of W characters each>
COLORS
<H lines of W digit characters each (0-9)>
TYPES
<H lines of W digit characters each (0-9)>   # optional

Rules
-----
* Width  (W) and height (H) must be multiples of CHUNK_SIZE (64).
* Glyph / color / type lines must have identical dimensions.
* Output files:  <out_dir>/<cx>_<cy>.txt

Usage
-----
  python3 tools/map_to_chunks.py map.txt
  python3 tools/map_to_chunks.py map.txt -o assets/map
  python3 tools/map_to_chunks.py map.txt --pad
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

CHUNK_SIZE = 64


def parse_map(path: Path):
    text = path.read_text(encoding="utf-8")
    lines = [ln.rstrip("\n\r") for ln in text.splitlines()]

    glyphs: list[str] = []
    colors: list[str] = []
    types: list[str] = []
    mode = None

    for ln in lines:
        stripped = ln.strip()
        if not stripped:
            continue
        upper = stripped.upper()
        if upper == "GLYPHS":
            mode = "glyphs"
            continue
        if upper == "COLORS":
            mode = "colors"
            continue
        if upper == "TYPES":
            mode = "types"
            continue
        if mode == "glyphs":
            glyphs.append(ln)
        elif mode == "colors":
            colors.append(ln.strip())
        elif mode == "types":
            types.append(ln.strip())

    if not glyphs:
        sys.exit("ERROR: no GLYPHS section found")
    if not colors:
        sys.exit("ERROR: no COLORS section found")
    return glyphs, colors, types


def validate(glyphs, colors, types, pad: bool):
    h_g, h_c = len(glyphs), len(colors)
    if h_g != h_c:
        sys.exit(f"ERROR: glyph height ({h_g}) != color height ({h_c})")
    if types and len(types) != h_g:
        sys.exit(f"ERROR: types height ({len(types)}) != glyph height ({h_g})")

    w = len(glyphs[0])
    for i, row in enumerate(glyphs):
        if len(row) != w:
            sys.exit(f"ERROR: glyph row {i} width {len(row)} != {w}")
    for i, row in enumerate(colors):
        if len(row) != w:
            sys.exit(f"ERROR: color row {i} width {len(row)} != {w}")
    for i, row in enumerate(types):
        if len(row) != w:
            sys.exit(f"ERROR: type row {i} width {len(row)} != {w}")

    if not types:
        types = ['0' * w for _ in range(h_g)]

    if pad:
        nw = ((w + CHUNK_SIZE - 1) // CHUNK_SIZE) * CHUNK_SIZE
        nh = ((h_g + CHUNK_SIZE - 1) // CHUNK_SIZE) * CHUNK_SIZE
        glyphs = [row + ('.' * (nw - len(row))) for row in glyphs] + ['.' * nw for _ in range(nh - h_g)]
        colors = [row + ('1' * (nw - len(row))) for row in colors] + ['1' * nw for _ in range(nh - h_g)]
        types  = [row + ('0' * (nw - len(row))) for row in types]  + ['0' * nw for _ in range(nh - h_g)]
        w, h_g = nw, nh
    else:
        if w % CHUNK_SIZE or h_g % CHUNK_SIZE:
            sys.exit(f"ERROR: size {w}x{h_g} not multiple of {CHUNK_SIZE} (use --pad)")

    return glyphs, colors, types


def write_chunks(glyphs, colors, types, out_dir: Path):
    out_dir.mkdir(parents=True, exist_ok=True)
    h, w = len(glyphs), len(glyphs[0])
    for cy in range(h // CHUNK_SIZE):
        for cx in range(w // CHUNK_SIZE):
            path = out_dir / f"{cx}_{cy}.txt"
            parts = ["GLYPHS"]
            for ly in range(CHUNK_SIZE):
                y = cy * CHUNK_SIZE + ly
                parts.append(glyphs[y][cx * CHUNK_SIZE: (cx + 1) * CHUNK_SIZE])
            parts.append("COLORS")
            for ly in range(CHUNK_SIZE):
                y = cy * CHUNK_SIZE + ly
                parts.append(colors[y][cx * CHUNK_SIZE: (cx + 1) * CHUNK_SIZE])
            parts.append("TYPES")
            for ly in range(CHUNK_SIZE):
                y = cy * CHUNK_SIZE + ly
                parts.append(types[y][cx * CHUNK_SIZE: (cx + 1) * CHUNK_SIZE])
            path.write_text("\n".join(parts) + "\n", encoding="utf-8")
            print(f"wrote {path}")


def main():
    ap = argparse.ArgumentParser(description="Split large map into 64x64 chunks")
    ap.add_argument("map", type=Path, help="input map.txt")
    ap.add_argument("-o", "--out", type=Path, default=Path("assets/map"))
    ap.add_argument("--pad", action="store_true", help="pad to next multiple of 64")
    args = ap.parse_args()

    glyphs, colors, types = parse_map(args.map)
    glyphs, colors, types = validate(glyphs, colors, types, args.pad)
    write_chunks(glyphs, colors, types, args.out)
    print("Done.")


if __name__ == "__main__":
    main()
