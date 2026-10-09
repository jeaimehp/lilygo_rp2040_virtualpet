#!/usr/bin/env python3
"""Render the real sketch and GFX rasterizer. Requires clang++ and Pillow."""
from array import array
from pathlib import Path
import subprocess
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / 'build'
BUILD.mkdir(exist_ok=True)
subprocess.run([
    'clang++', '-std=c++17', '-DARDUINO=10800',
    '-I' + str(ROOT / 'tools/host'),
    '-I' + str(ROOT / 'dependencies/Adafruit_GFX_Library'),
    str(ROOT / 'tools/render.cpp'),
    str(ROOT / 'dependencies/Adafruit_GFX_Library/Adafruit_GFX.cpp'),
    '-o', str(BUILD / 'render-scenes')], check=True)
subprocess.run([str(BUILD / 'render-scenes'), str(BUILD)], check=True)

screens = ROOT / 'assets/screenshots'
animations = ROOT / 'assets/animations'
screens.mkdir(parents=True, exist_ok=True)
animations.mkdir(parents=True, exist_ok=True)
names = ['meadow', 'picnic', 'play', 'sleep', 'digging', 'grooming']
for name in names:
    raw = (BUILD / (name + '.rgb565')).read_bytes()
    assert len(raw) == 80 * 240 * 135 * 2
    pixels = array('H', raw)
    # Renderer writes native uint16_t. Expand 5/6-bit channels by bit replication.
    frames = []
    for i in range(80):
        rgb = bytearray()
        for c in pixels[i*32400:(i+1)*32400]:
            r, g, b = (c >> 11) & 31, (c >> 5) & 63, c & 31
            rgb.extend(((r << 3) | (r >> 2), (g << 2) | (g >> 4), (b << 3) | (b >> 2)))
        frames.append(Image.frombytes('RGB', (240, 135), bytes(rgb)))
    png = screens / (name + '.png')
    frames[8].save(png)
    assert Image.open(png).tobytes() == frames[8].tobytes()

    # Build a palette containing every used color, so GIF encoding loses none.
    colors = sorted({color for f in frames for count, color in f.getcolors(256)})
    assert len(colors) <= 256
    palette = Image.new('P', (1, 1))
    palette.putpalette([v for color in colors for v in color] + [0] * (768-3*len(colors)))
    indexed = [f.quantize(palette=palette, dither=Image.Dither.NONE) for f in frames]
    assert all(a.convert('RGB').tobytes() == b.tobytes() for a, b in zip(indexed, frames))
    # GIF time units are 10 ms. Alternating 60/70 ms preserves the 65 ms average.
    durations = [60 if i % 2 == 0 else 70 for i in range(80)]
    gif = animations / (name + '.gif')
    indexed[0].save(gif, save_all=True, append_images=indexed[1:],
                    duration=durations, loop=0, disposal=2, optimize=False)
    # Encoders can merge identical frames; validate displayed pixels by timeline.
    decoded = Image.open(gif)
    elapsed = 0
    for i in range(decoded.n_frames):
        decoded.seek(i)
        duration = decoded.info['duration']
        for j in range(80):
            start = sum(durations[:j])
            if elapsed <= start < elapsed + duration:
                assert decoded.convert('RGB').tobytes() == frames[j].tobytes()
        elapsed += duration
    assert elapsed == sum(durations)
    print(f'{name}: native PNG + {decoded.n_frames}-frame GIF, {len(colors)} exact colors')

# A larger contact sheet is only for inspection; individual assets stay native.
sheet = Image.new('RGB', (720, 270), 'white')
for i, name in enumerate(names):
    sheet.paste(Image.open(screens / (name + '.png')), ((i % 3)*240, (i // 3)*135))
sheet.save(BUILD / 'contact-sheet.png')
