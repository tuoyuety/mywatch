#include "ui.h"
#include "ui_helpers.h"
#include "ui_SportPage.h"
#include "PageManager.h"
#include "HWDataAccess.h"
#include <stdio.h>

Page_t Page_Sport = {ui_SportPage_screen_init, ui_SportPage_screen_deinit, &ui_SportPage};

lv_obj_t * ui_SportPage;
static lv_obj_t * s_title_label;
static lv_obj_t * s_sub_label;
static lv_obj_t * s_step_label;
static lv_obj_t * s_hint_label;
static lv_timer_t * s_sport_timer;

static void sport_timer_cb(lv_timer_t * t)
{
    LV_UNUSED(t);
    Page_t * now = Page_Get_NowPage();
    if(now != &Page_Sport) {
        return;
    }

    char buf[16];
#if HW_USE_IMU
    if(HWInterface.IMU.ConnectionError) {
        lv_label_set_text(s_step_label, "--");
        lv_label_set_text(s_hint_label, "\xe8\xae\xa1\xe6\xad\xa5\xe4\xb8\x8d\xe5\x8f\xaf\xe7\x94\xa8");
    }
    else {
        sprintf(buf, "%u", (unsigned)HWInterface.IMU.Steps);
        lv_label_set_text(s_step_label, buf);
        lv_label_set_text(s_hint_label, "");
    }
#else
    lv_label_set_text(s_step_label, "--");
    lv_label_set_text(s_hint_label, "\xe6\x9c\xaa\xe5\x90\xaf\xe7\x94\xa8 IMU");
#endif
}

static void sport_page_event(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_GESTURE) {
        if(lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT) {
            Page_Back();
        }
    }
}

void ui_SportPage_screen_init(void)
{
    ui_SportPage = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_SportPage, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(ui_SportPage, sport_page_event, LV_EVENT_ALL, NULL);

    s_title_label = lv_label_create(ui_SportPage);
    lv_label_set_text(s_title_label, "\xe8\xbf\x90\xe5\x8a\xa8");
    lv_obj_set_style_text_font(s_title_label, &ui_font_Cuyuan20, 0);
    lv_obj_set_style_text_color(s_title_label, lv_color_hex(0x3278FF), 0);
    lv_obj_align(s_title_label, LV_ALIGN_TOP_MID, 0, 12);

    s_sub_label = lv_label_create(ui_SportPage);
    lv_label_set_text(s_sub_label, "\xe4\xbb\x8a\xe6\x97\xa5\xe6\xad\xa5\xe6\x95\xb0");
    lv_obj_set_style_text_font(s_sub_label, &ui_font_Cuyuan18, 0);
    lv_obj_align(s_sub_label, LV_ALIGN_CENTER, 0, -50);

    s_step_label = lv_label_create(ui_SportPage);
    lv_obj_set_style_text_font(s_step_label, &ui_font_Cuyuan48, 0);
    lv_obj_align(s_step_label, LV_ALIGN_CENTER, 0, 0);

    s_hint_label = lv_label_create(ui_SportPage);
    lv_obj_set_width(s_hint_label, 220);
    lv_label_set_long_mode(s_hint_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(s_hint_label, &ui_font_Cuyuan18, 0);
    lv_obj_set_style_text_color(s_hint_label, lv_color_hex(0xA0A0A0), 0);
    lv_obj_align(s_hint_label, LV_ALIGN_BOTTOM_MID, 0, -24);

    sport_timer_cb(NULL);

    if(s_sport_timer != NULL) {
        lv_timer_del(s_sport_timer);
        s_sport_timer = NULL;
    }
    s_sport_timer = lv_timer_create(sport_timer_cb, 500, NULL);
}

void ui_SportPage_screen_deinit(void)
{
    if(s_sport_timer != NULL) {
        lv_timer_del(s_sport_timer);
        s_sport_timer = NULL;
    }
}
