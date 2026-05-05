#include "ui.h"
#include "ui_helpers.h"
#include "ui_ComputerPage.h"
#include "StrCalculate.h"
#include <stdio.h>
#include <string.h>
///////////////////// Page Manager //////////////////
Page_t Page_Computer = {ui_ComputerPage_screen_init, ui_ComputerPage_screen_deinit, &ui_Computerpage};

///////////////////// VARIABLES ////////////////////

StrStack_t CalStr;
NumStack_t NumStack;
SymStack_t SymStack;
lv_obj_t * ui_Computerpage;
lv_obj_t * ui_CompageBtnM;
lv_obj_t * ui_CompageTextarea;
lv_obj_t * ui_CompageBackBtn;

/*
 * 按键显示：ui_font_Cuyuan24 仅含 ＋－×÷＝ 与 ASCII 数字等，无 ASCII 的 +* /=。
 * 用 UTF-8（十六进制）做标签；表达式内部仍用 ASCII 存 CalStr，显示由 calstr 映射刷新。
 */
static const char * ui_ComPageBtnmap[] = {
    "1", "2", "3", "\xEF\xBC\x8B", "\n", /* ＋ */
    "4", "5", "6", "\xEF\xBC\x8D", "\n", /* － */
    "7", "8", "9", "\xC3\x97", "\n",     /* × */
    ".", "0", "\xEF\xBC\x9D", "\xC3\xB7", "" /* ＝ ÷ */
};

/* 将 CalStr 中 ASCII 运算符转为字体内可见的 UTF-8 */
static void computer_append_disp_char(char * buf, size_t buf_sz, char c)
{
    size_t len = strlen(buf);
    if(len >= buf_sz - 1U) {
        return;
    }
    const char * u8 = NULL;
    switch(c) {
        case '+':
            u8 = "\xEF\xBC\x8B";
            break; /* ＋ */
        case '-':
            u8 = "\xEF\xBC\x8D";
            break; /* － */
        case '*':
            u8 = "\xC3\x97";
            break; /* × */
        case '/':
            u8 = "\xC3\xB7";
            break; /* ÷ */
        default:
            buf[len] = c;
            buf[len + 1U] = '\0';
            return;
    }
    while(*u8 != '\0' && len < buf_sz - 1U) {
        buf[len++] = *u8++;
        buf[len] = '\0';
    }
}

static void computer_textarea_from_calstr(void)
{
    if(ui_CompageTextarea == NULL) {
        return;
    }
    char buf[48];
    buf[0] = '\0';
    for(uint8_t i = 0; i < CalStr.Top_Point; i++) {
        computer_append_disp_char(buf, sizeof(buf), CalStr.strque[i]);
    }
    lv_textarea_set_text(ui_CompageTextarea, buf);
}


///////////////////// FUNCTIONS ////////////////////
void ui_CompageBtnM_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_DRAW_PART_BEGIN)
    {
        lv_obj_draw_part_dsc_t * dsc = lv_event_get_param(e);
        if(dsc->id == 3 || dsc->id == 7 || dsc->id == 11 || dsc->id == 14 || dsc->id == 15)
        {
            /* LV_RADIUS_CIRCLE 在窄格子里会把全角运算符裁切变形，改用适度圆角 */
            if(dsc->draw_area != NULL && dsc->rect_dsc != NULL) {
                lv_coord_t cw = lv_area_get_width(dsc->draw_area);
                lv_coord_t ch = lv_area_get_height(dsc->draw_area);
                lv_coord_t m = (cw < ch) ? cw : ch;
                dsc->rect_dsc->radius = (m > 6) ? (m / 3) : 0;
            }
            if(lv_btnmatrix_get_selected_btn(obj) == dsc->id) {
                dsc->rect_dsc->bg_color = lv_palette_darken(LV_PALETTE_BLUE, 3);
            }
            else {
                dsc->rect_dsc->bg_color = lv_palette_main(LV_PALETTE_BLUE);
            }
        }
    }
    if(code == LV_EVENT_DRAW_PART_END)
    {
        lv_obj_draw_part_dsc_t * dsc = lv_event_get_param(e);


    }
    if(code == LV_EVENT_VALUE_CHANGED)
    {
        /* 必须用事件参数里的按键索引；get_selected_btn 在部分时序下与本次触发不一致，会导致算完后无法响应下一次按键 */
        uint32_t * p_btn = lv_event_get_param(e);
        uint16_t btn_id = (p_btn != NULL) ? (uint16_t)(*p_btn) : lv_btnmatrix_get_selected_btn(obj);
        const char * txt = lv_btnmatrix_get_btn_text(obj, btn_id);

        if(btn_id == 14)
        {
            if(ui_CompageTextarea != NULL && CalStr.Top_Point > 0)
            {
                char expr_disp[48];
                expr_disp[0] = '\0';
                for(uint8_t i = 0; i < CalStr.Top_Point; i++) {
                    computer_append_disp_char(expr_disp, sizeof(expr_disp), CalStr.strque[i]);
                }

                char strout[16];
                if(StrCalculate(CalStr.strque, &NumStack, &SymStack)) {
                    strcpy(strout, "erro");
                }
                else {
                    if(isIntNumber(NumStack.data[NumStack.Top_Point - 1])) {
                        sprintf(strout, "%.0f", NumStack.data[NumStack.Top_Point - 1]);
                    }
                    else {
                        sprintf(strout, "%.4f", NumStack.data[NumStack.Top_Point - 1]);
                    }
                }

                char full[80];
                lv_snprintf(full, sizeof(full), "%s\xEF\xBC\x9D\n%s", expr_disp, strout);
                lv_textarea_set_text(ui_CompageTextarea, full);
                strclear(&CalStr);
                /* 清除矩阵内部选中态，避免 one_checked/选中残留导致后续触摸不触发 VALUE_CHANGED */
                lv_btnmatrix_set_selected_btn(obj, LV_BTNMATRIX_BTN_NONE);
            }
        }
        else if(txt != NULL && ui_CompageTextarea != NULL)
        {
            uint8_t max_expr = (uint8_t)(sizeof(CalStr.strque) - 1u);
            if(CalStr.Top_Point >= max_expr) {
                /* 已满 */
            }
            else {
                uint8_t ok = 0;
                switch(btn_id)
                {
                    case 0:
                        ok = (strput(&CalStr, '1') == 0);
                        break;
                    case 1:
                        ok = (strput(&CalStr, '2') == 0);
                        break;
                    case 2:
                        ok = (strput(&CalStr, '3') == 0);
                        break;
                    case 3:
                        ok = (strput(&CalStr, '+') == 0);
                        break;
                    case 4:
                        ok = (strput(&CalStr, '4') == 0);
                        break;
                    case 5:
                        ok = (strput(&CalStr, '5') == 0);
                        break;
                    case 6:
                        ok = (strput(&CalStr, '6') == 0);
                        break;
                    case 7:
                        ok = (strput(&CalStr, '-') == 0);
                        break;
                    case 8:
                        ok = (strput(&CalStr, '7') == 0);
                        break;
                    case 9:
                        ok = (strput(&CalStr, '8') == 0);
                        break;
                    case 10:
                        ok = (strput(&CalStr, '9') == 0);
                        break;
                    case 11:
                        ok = (strput(&CalStr, '*') == 0);
                        break;
                    case 12:
                        ok = (strput(&CalStr, '.') == 0);
                        break;
                    case 13:
                        ok = (strput(&CalStr, '0') == 0);
                        break;
                    case 15:
                        ok = (strput(&CalStr, '/') == 0);
                        break;
                    default:
                        break;
                }
                if(ok) {
                    computer_textarea_from_calstr();
                }
            }
        }
    }
}


static void ui_CompageBackBtn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(ui_CompageTextarea == NULL) {
        return;
    }
    /* 用 PRESSED：电阻屏/短滑易丢 CLICKED；长按仍用 LONG_PRESSED 清空 */
    if(code == LV_EVENT_PRESSED)
    {
        if(!strstack_isEmpty(&CalStr))
        {
            strdel(&CalStr);
            computer_textarea_from_calstr();
        }
        else
        {
            lv_textarea_set_text(ui_CompageTextarea, "");
        }
    }
    else if(code == LV_EVENT_LONG_PRESSED)
    {
        if(!strstack_isEmpty(&CalStr))
        {
            strclear(&CalStr);
        }
        lv_textarea_set_text(ui_CompageTextarea, "");
    }
}


///////////////////// SCREEN init ////////////////////
void ui_ComputerPage_screen_init(void)
{
		strclear(&CalStr);
		NumStackClear(&NumStack);
		SymStackClear(&SymStack);
    ui_Computerpage = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Computerpage,LV_OBJ_FLAG_SCROLLABLE);
    ui_CompageBtnM = lv_btnmatrix_create(ui_Computerpage);
    lv_btnmatrix_set_map(ui_CompageBtnM, ui_ComPageBtnmap);

    lv_obj_set_style_text_font(ui_CompageBtnM, &ui_font_Cuyuan24, 0);
    /* 计算器非单选控件；one_checked 会在内部维护 CHECKED 状态，易与连续点击冲突 */
    lv_btnmatrix_set_one_checked(ui_CompageBtnM, false);
    int i = 0;
    for (i = 0; i < 16; i++)
    {
        lv_btnmatrix_set_btn_ctrl(ui_CompageBtnM, i, LV_BTNMATRIX_CTRL_NO_REPEAT); // 长按按钮时禁用重复
    }
    /* 允许点击键盘获得焦点，避免焦点一直停在 textarea 上影响部分输入设备/ group's 行为 */
    lv_obj_add_flag(ui_CompageBtnM, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_set_style_border_width(ui_CompageBtnM,0,0);
    lv_obj_set_style_bg_opa(ui_CompageBtnM,0,0);
    lv_obj_set_style_pad_all(ui_CompageBtnM, 2, LV_PART_ITEMS);
    lv_obj_set_size(ui_CompageBtnM, 240, 220);
    lv_obj_set_align(ui_CompageBtnM, LV_ALIGN_BOTTOM_MID);

    ui_CompageTextarea = lv_textarea_create(ui_Computerpage);
    lv_textarea_set_one_line(ui_CompageTextarea, false); // 将文本区域配置为一行
    //lv_textarea_set_password_mode(obj_text_area, true); // 将文本区域配置为密码模式
    /* 含 UTF-8 运算符时字节数大于 CalStr 字符数 */
    lv_textarea_set_max_length(ui_CompageTextarea, 96);
    lv_obj_add_state(ui_CompageTextarea, LV_STATE_FOCUSED); /* 光标 */
    lv_obj_clear_flag(ui_CompageTextarea, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /* 避免抢滚动/焦点链导致后续点击异常 */
    lv_obj_set_style_radius(ui_CompageTextarea, 0, 0); // 设置样式的圆角弧度
    lv_obj_set_style_border_width(ui_CompageTextarea, 0, 0); //设置边框宽度
    lv_obj_set_style_bg_color(ui_CompageTextarea, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_CompageTextarea, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    /*
     * 退格在 layer_top 右上角约 52px 宽；textarea 若过宽，右对齐时文字会从退格下方起笔。
     * 宽度 = 屏宽 - 退格宽度 - 间隙（与 ui_CompageBackBtn 的 52 同步修改）。
     */
    lv_obj_set_size(ui_CompageTextarea, 240 - 52 - 8, 60);
    lv_obj_align(ui_CompageTextarea, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_clear_flag(ui_CompageTextarea,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_text_font(ui_CompageTextarea, &ui_font_Cuyuan24, 0);
    lv_textarea_set_align(ui_CompageTextarea, LV_TEXT_ALIGN_RIGHT);

    /*
     * 退格放在 lv_layer_top()，保证永远在普通屏幕之上，避免与键盘「＋」等争触摸顺序。
     * 离开页面时必须在 deinit 里删除，否则会残留在其它界面。
     */
    if(ui_CompageBackBtn != NULL)
    {
        lv_obj_del(ui_CompageBackBtn);
        ui_CompageBackBtn = NULL;
    }
    ui_CompageBackBtn = lv_btn_create(lv_layer_top());
    lv_obj_set_size(ui_CompageBackBtn, 52, 52);
    lv_obj_align(ui_CompageBackBtn, LV_ALIGN_TOP_RIGHT, -4, 4);
    lv_obj_set_style_bg_opa(ui_CompageBackBtn, LV_OPA_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_CompageBackBtn, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_CompageBackBtn, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(ui_CompageBackBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    lv_obj_t * btnlabel = lv_label_create(ui_CompageBackBtn);
    lv_label_set_text(btnlabel, LV_SYMBOL_BACKSPACE);
    lv_obj_set_style_text_font(btnlabel, &lv_font_montserrat_24, 0);
    lv_obj_center(btnlabel);

    lv_obj_add_event_cb(ui_CompageBtnM, ui_CompageBtnM_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_CompageBackBtn, ui_CompageBackBtn_event_cb, LV_EVENT_ALL, NULL);
}

/////////////////// SCREEN deinit ////////////////////
void ui_ComputerPage_screen_deinit(void)
{
    if(ui_CompageBackBtn != NULL)
    {
        lv_obj_del(ui_CompageBackBtn);
        ui_CompageBackBtn = NULL;
    }
}
