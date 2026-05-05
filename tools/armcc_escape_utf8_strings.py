# -*- coding: utf-8 -*-
"""Escape non-ASCII characters in C string literals as \\xHH bytes (UTF-8) for Keil ARMCC 5."""
import re
import sys
from pathlib import Path
from typing import Optional

ROOT = Path(__file__).resolve().parents[1]


def decode_c_string_inner(inner: str) -> Optional[str]:
    """Decode C string contents (without surrounding quotes). Returns None if unsupported."""
    out: list[str] = []
    i = 0
    while i < len(inner):
        c = inner[i]
        if c != "\\":
            out.append(c)
            i += 1
            continue
        if i + 1 >= len(inner):
            return None
        n = inner[i + 1]
        i += 2
        if n == "\\":
            out.append("\\")
            continue
        if n == '"':
            out.append('"')
            continue
        if n == "'":
            out.append("'")
            continue
        if n == "n":
            out.append("\n")
            continue
        if n == "r":
            out.append("\r")
            continue
        if n == "t":
            out.append("\t")
            continue
        if n == "0":
            out.append("\0")
            continue
        if n == "a":
            out.append("\a")
            continue
        if n == "b":
            out.append("\b")
            continue
        if n == "f":
            out.append("\f")
            continue
        if n == "v":
            out.append("\v")
            continue
        if n == "x":
            m = re.match(r"([0-9a-fA-F]{1,2})", inner[i:])
            if not m:
                return None
            out.append(chr(int(m.group(1), 16)))
            i += m.end(1)
            continue
        if n.isdigit():
            # octal: up to 3 digits
            j = i
            digits = n
            while len(digits) < 3 and j < len(inner) and inner[j].isdigit():
                digits += inner[j]
                j += 1
            i = j
            try:
                v = int(digits, 8)
                if v > 0xFF:
                    return None
                out.append(chr(v))
            except ValueError:
                return None
            continue
        return None
    return "".join(out)


def utf8_hex_escape(s: str) -> str:
    return "".join("\\x%02x" % b for b in s.encode("utf-8"))


def transform_source(text: str) -> str:
    out: list[str] = []
    i = 0
    n = len(text)
    in_line = False
    in_block = False
    in_string = False
    in_char = False
    string_buf: list[str] = []

    def flush_string():
        nonlocal string_buf
        raw_inner = "".join(string_buf)
        decoded = decode_c_string_inner(raw_inner)
        out.append('"')
        if decoded is None:
            out.append(raw_inner)
        elif any(ord(ch) > 127 for ch in decoded):
            out.append(utf8_hex_escape(decoded))
        else:
            out.append(raw_inner)
        out.append('"')
        string_buf = []

    while i < n:
        c = text[i]
        nxt = text[i + 1] if i + 1 < n else ""

        if in_line:
            out.append(c)
            if c == "\n":
                in_line = False
            i += 1
            continue

        if in_block:
            out.append(c)
            if c == "*" and nxt == "/":
                out.append("/")
                in_block = False
                i += 2
            else:
                i += 1
            continue

        if in_string:
            if c == "\\":
                string_buf.append(c)
                if i + 1 < n:
                    string_buf.append(text[i + 1])
                    i += 2
                else:
                    i += 1
                continue
            if c == '"':
                flush_string()
                in_string = False
                i += 1
                continue
            string_buf.append(c)
            i += 1
            continue

        if in_char:
            out.append(c)
            if c == "\\":
                if i + 1 < n:
                    out.append(text[i + 1])
                    i += 2
                else:
                    i += 1
                continue
            if c == "'":
                in_char = False
            i += 1
            continue

        # not in special state
        if c == "/" and nxt == "/":
            out.append(c)
            out.append(nxt)
            in_line = True
            i += 2
            continue
        if c == "/" and nxt == "*":
            out.append(c)
            out.append(nxt)
            in_block = True
            i += 2
            continue
        if c == '"':
            in_string = True
            string_buf = []
            i += 1
            continue
        if c == "'":
            out.append(c)
            in_char = True
            i += 1
            continue

        out.append(c)
        i += 1

    return "".join(out)


def main() -> int:
    paths: list[Path] = []
    gui = ROOT / "USER" / "GUI_App"
    paths.extend(sorted((gui / "Screens").glob("*.c")))
    for name in ("ui.c", "ui_helpers.c"):
        p = gui / name
        if p.is_file():
            paths.append(p)

    changed = 0
    for p in paths:
        raw = p.read_bytes()
        try:
            text = raw.decode("utf-8")
        except UnicodeDecodeError:
            print("skip (not utf-8):", p, file=sys.stderr)
            continue
        new_text = transform_source(text)
        if new_text != text:
            p.write_text(new_text, encoding="utf-8", newline="\n")
            changed += 1
            print("updated:", p.relative_to(ROOT))
    print("files changed:", changed)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
