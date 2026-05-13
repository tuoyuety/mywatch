#!/usr/bin/env python3
"""
Merge Google Material Icons glyphs into watch private-use codepoints (E6xx / EDxx).

The UI uses fixed UTF-8 sequences; we only replace the TTF then re-run lv_font_conv.

License: Material Icons font is Apache 2.0 (see Google material-design-icons repo).

Usage:
  pip install fonttools
  python merge_material_to_watch.py
  # writes tools/watch_iconfont/assets/watch_iconfont_merged.ttf
"""

from __future__ import annotations

import os
import shutil
import sys
from urllib.request import urlretrieve

try:
    from fontTools.ttLib import TTFont
except ImportError:
    print("Install: pip install fonttools", file=sys.stderr)
    raise

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, "..", ".."))
ASSETS = os.path.join(REPO, "tools", "watch_iconfont", "assets")
MATERIAL_TTF = os.path.join(HERE, "MaterialIcons-Regular.ttf")
OUT_TTF = os.path.join(ASSETS, "watch_iconfont_merged.ttf")

# Watch codepoint -> Material Icons codepoint (same BMP / PUA as in MaterialIcons-Regular.codepoints)
WATCH_TO_MATERIAL: dict[int, int] = {
    0xE68F: 0xE1A3,  # charge battery
    0xE676: 0xE566,  # sport
    0xE6E4: 0xE51C,  # long off / night
    0xE89B: 0xEBBA,  # comm / tower
    0xE673: 0xE798,  # env humidity (small)
    0xE69B: 0xE564,  # compass altitude (terrain)
    0xE788: 0xE87A,  # explore (compass dir + NFC / wallet shared)
    0xE653: 0xE1A7,  # BLE
    0xE602: 0xEBCC,  # card calendar
    0xE601: 0xE935,  # menu calendar
    0xE61C: 0xE87B,  # subset spare
    0xE600: 0xE8B8,  # settings
    0xE60B: 0xE88E,  # about
    0xE659: 0xE1FF,  # home temp
    0xE635: 0xEA28,  # games menu
    0xE7BC: 0xF016,  # 2048 grid
    0xE60E: 0xE8B5,  # light time
    0xE6BF: 0xE429,  # date set spare
    0xE762: 0xEAA2,  # menu HR
    0xE65C: 0xE430,  # 下滑亮度：wb_sunny（小太阳）
    0xE65A: 0xE80C,  # campus card
    0xE619: 0xE570,  # transit card
    0xE633: 0xE425,  # timer / stopwatch menu
    0xE652: 0xEFE4,  # SpO2 / HR (bloodtype)
    0xE706: 0xEFD8,  # env menu (air)
    0xE603: 0xEAA2,  # HR page
    0xE986: 0xE518,  # brightness sun
    0xE607: 0xE887,  # subset spare
    0xE62C: 0xE1A4,  # battery home
    0xE67A: 0xF076,  # env page temp
    0xE7A0: 0xE536,  # steps + memory game (shared codepoint)
    0xE696: 0xE425,  # off timer
    0xE62A: 0xE334,  # wrist
    0xED1D: 0xE5CA,  # OK check
}


_MATERIAL_URL = (
    "https://raw.githubusercontent.com/google/material-design-icons/master/font/MaterialIcons-Regular.ttf"
)


def main() -> None:
    if not os.path.isfile(MATERIAL_TTF):
        print("Downloading Material Icons TTF …")
        os.makedirs(HERE, exist_ok=True)
        urlretrieve(_MATERIAL_URL, MATERIAL_TTF)

    os.makedirs(ASSETS, exist_ok=True)
    shutil.copy2(MATERIAL_TTF, OUT_TTF)
    font = TTFont(OUT_TTF)

    cmap = None
    for t in font["cmap"].tables:
        if t.isUnicode():
            cmap = t.cmap
            break
    if cmap is None:
        raise RuntimeError("No Unicode cmap in Material font")

    for dst, src in WATCH_TO_MATERIAL.items():
        gname = cmap.get(src)
        if not gname:
            raise KeyError(f"No glyph for material U+{src:04X} (watch U+{dst:04X})")
        cmap[int(dst)] = gname

    font.save(OUT_TTF)
    print("Wrote", OUT_TTF)


if __name__ == "__main__":
    main()
