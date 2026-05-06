/* 我：上电先显示题目姓名学号和导师，字模是 GB2312 16x16 */

#include "thesis_boot_lcd.h"
#include "lcd.h"
#include "lcd_init.h"
#include "thesis_meta.h"

/* GB2312，0x00 结尾 */
static const uint8_t boot_title_gb2312[] = {
    0xb5, 0xcd, 0xb9, 0xa6, 0xba, 0xc4, 0xd6, 0xc7, 0xc4, 0xdc, 0xca, 0xd6, 0xb1, 0xed, 0xb5, 0xc4,
    0xc9, 0xe8, 0xbc, 0xc6, 0xd3, 0xeb, 0xca, 0xb5, 0xcf, 0xd6, 0x00,
};

static const uint8_t boot_name_gb2312[] = {
    0xd0, 0xbb, 0xd6, 0xbe, 0xbc, 0xe1, 0x00,
};

static const uint8_t boot_advisor_gb2312[] = {
    0xd6, 0xb8, 0xb5, 0xbc, 0xc0, 0xcf, 0xca, 0xa6, 0xa3, 0xba, 0xd6, 0xd3, 0xd0, 0xf1, 0x00,
};

void thesis_boot_show_on_lcd(void)
{
    const u8 row_step = 22;
    const u8 n_rows = 4;
    const u16 block_h = row_step * (n_rows - 1) + 16;
    u16 y0 = (LCD_H > block_h) ? (u16)((LCD_H - block_h) / 2) : 0;
    u16 x;
    u8 id_len = 0;
    const char *id = THESIS_AUTHOR_STUDENT_ID;

    LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);

    x = (LCD_W > 13 * 16) ? (u16)((LCD_W - 13 * 16) / 2) : 0;
    LCD_ShowChinese(x, y0, (u8 *)boot_title_gb2312, WHITE, BLACK, 16, 0);

    x = (LCD_W > 3 * 16) ? (u16)((LCD_W - 3 * 16) / 2) : 0;
    LCD_ShowChinese(x, (u16)(y0 + row_step), (u8 *)boot_name_gb2312, WHITE, BLACK, 16, 0);

    while (id[id_len] != '\0') {
        id_len++;
    }
    x = (LCD_W > id_len * 8) ? (u16)((LCD_W - id_len * 8) / 2) : 0;
    LCD_ShowString(x, (u16)(y0 + row_step * 2), (const u8 *)id, WHITE, BLACK, 16, 0);

    x = (LCD_W > 7 * 16) ? (u16)((LCD_W - 7 * 16) / 2) : 0;
    LCD_ShowChinese(x, (u16)(y0 + row_step * 3), (u8 *)boot_advisor_gb2312, WHITE, BLACK, 16, 0);
}
