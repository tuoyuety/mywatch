# Generate 16x16 GB2312 glyph masks for thesis boot screen (PCtoLCD-style packing
# matching USER/drivers/LCD/lcd.c LCD_ShowChinese16x16: row-major, 2 bytes/row,
# bit j of byte = column (row_byte*8+j), LSB = left pixel within byte group).

from __future__ import annotations

from PIL import Image, ImageDraw, ImageFont

FONT = "C:/Windows/Fonts/msyh.ttc"
SIZE = 12  # px, tuned for 16x16 box（全角冒号也更大些）

CHARS = sorted(set("低功耗智能手表的设计与实现谢志坚指导老师钟旭："))


def glyph_bytes(ch: str) -> tuple[bytes, list[int]]:
    gb = ch.encode("gb2312")
    if len(gb) != 2:
        raise ValueError(f"{ch!r} not in gb2312")
    im = Image.new("1", (16, 16), 1)
    dr = ImageDraw.Draw(im)
    font = ImageFont.truetype(FONT, SIZE)
    bbox = dr.textbbox((0, 0), ch, font=font)
    w, h = bbox[2] - bbox[0], bbox[3] - bbox[1]
    ox = (16 - w) // 2 - bbox[0]
    oy = (16 - h) // 2 - bbox[1]
    dr.text((ox, oy), ch, font=font, fill=0)
    px = im.load()
    out: list[int] = []
    for row in range(16):
        for cx in range(2):
            b = 0
            for bit in range(8):
                col = cx * 8 + bit
                # 0 = ink (foreground)
                if px[col, row] == 0:
                    b |= 1 << bit
            out.append(b)
    return gb, out


def main() -> None:
    lines: list[str] = []
    for ch in CHARS:
        gb, msk = glyph_bytes(ch)
        hexes = ", ".join(f"0x{b:02X}" for b in msk)
        lines.append(
            f"\t{{ 0x{gb[0]:02X}, 0x{gb[1]:02X}, {hexes} }}, /* {ch} */\n"
        )
    path = "USER/drivers/LCD/thesis_boot_glyphs16.inc"
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.writelines(lines)
    print("Wrote", path, "entries:", len(lines))


if __name__ == "__main__":
    main()
