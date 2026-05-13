#!/usr/bin/env python3
"""
Regenerate USER/GUI_App/Fonts/ui_font_iconfont*.c from watch_iconfont_merged.ttf.

Requires: Python 3, Node.js (any recent), pip package fonttools for merge step.
"""

from __future__ import annotations

import os
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, "..", ".."))
FONT = os.path.join(HERE, "assets", "watch_iconfont_merged.ttf")
OUT_DIR = os.path.join(REPO, "USER", "GUI_App", "Fonts")
CLI = os.path.join(HERE, "lv_font_conv_pkg", "package", "lib", "cli.js")

JOBS = [
    (16, "ui_font_iconfont16.c", ["0xe68f"]),
    (24, "ui_font_iconfont24.c", ["0xE65C", "0xe676", "0xe68f"]),
    (
        28,
        "ui_font_iconfont28.c",
        ["0xE6E4", "0xE89B", "0xE673", "0xE69B", "0xE788", "0xE653"],
    ),
    (
        30,
        "ui_font_iconfont30.c",
        [
            "0xE602",
            "0xE601",
            "0xE61C",
            "0xE600",
            "0xE60B",
            "0xE659",
            "0xE635",
            "0xE7BC",
        ],
    ),
    (32, "ui_font_iconfont32.c", ["0xE60E", "0xE6BF", "0xE762"]),
    (
        34,
        "ui_font_iconfont34.c",
        [
            "0xE65C",
            "0xE65A",
            "0xE619",
            "0xE633",
            "0xE762",
            "0xE652",
            "0xE706",
            "0xE788",
            "0xE603",
            "0xE986",
            "0xE607",
            "0xE62C",
            "0xE600",
            "0xE67A",
            "0xE7A0",
        ],
    ),
    (45, "ui_font_iconfont45.c", ["0xE696", "0xE62A", "0xe60e", "0xED1D"]),
]


def find_node() -> str:
    p = shutil.which("node")
    if p:
        return p
    # Cursor-bundled Node (common on dev machines without Node on PATH)
    cursor = os.path.join(
        os.environ.get("LOCALAPPDATA", ""),
        "Programs",
        "cursor",
        "resources",
        "app",
        "resources",
        "helpers",
        "node.exe",
    )
    if os.path.isfile(cursor):
        return cursor
    print("node not found. Install Node.js or add it to PATH.", file=sys.stderr)
    sys.exit(1)


def main() -> None:
    merge_py = os.path.join(HERE, "merge_material_to_watch.py")
    subprocess.check_call([sys.executable, merge_py], cwd=HERE)

    if not os.path.isfile(FONT):
        print("Missing merged font:", FONT, file=sys.stderr)
        sys.exit(1)
    if not os.path.isfile(CLI):
        print(
            "Missing lv_font_conv. Extract npm package lv_font_conv to:",
            os.path.dirname(CLI),
            file=sys.stderr,
        )
        sys.exit(1)

    node = find_node()
    for size, out_name, ranges in JOBS:
        out_path = os.path.join(OUT_DIR, out_name)
        args = [
            node,
            CLI,
            "--bpp",
            "4",
            "--size",
            str(size),
            "--font",
            FONT,
            "-o",
            out_path,
            "--format",
            "lvgl",
            "--no-compress",
            "--no-prefilter",
        ]
        for r in ranges:
            args.extend(["-r", r])
        print(" ".join(args))
        subprocess.check_call(args)

        _patch_opts_comment(out_path, size, out_name, ranges)


def _patch_opts_comment(path: str, size: int, out_name: str, ranges: list[str]) -> None:
    with open(path, encoding="utf-8", errors="replace") as f:
        lines = f.readlines()
    rel_font = os.path.relpath(FONT, REPO).replace("\\", "/")
    rel_out = os.path.join("USER", "GUI_App", "Fonts", out_name).replace("\\", "/")
    rargs = " ".join(f"-r {r}" for r in ranges)
    new_opts = (
        f" * Opts: --bpp 4 --size {size} --font {rel_font} "
        f"-o {rel_out} --format lvgl {rargs} --no-compress --no-prefilter\n"
    )
    for i, line in enumerate(lines):
        if line.startswith(" * Opts:"):
            lines[i] = new_opts
            break
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.writelines(lines)


if __name__ == "__main__":
    main()
