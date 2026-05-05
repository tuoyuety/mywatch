#include "ui.h"
#include "ui_helpers.h"
#include "ui_AboutPage.h"
#include "version.h" /* 内含 thesis_meta：产品名、署名别硬编码散落各处 */
///////////////////// VARIABLES ////////////////////
lv_obj_t * ui_AboutPage;

///////////////////// Page Manager //////////////////
Page_t Page_About = {ui_AboutPage_screen_init, ui_AboutPage_screen_deinit, &ui_AboutPage};

///////////////////// FUNCTIONS ////////////////////


///////////////////// SCREEN init ////////////////////
void ui_AboutPage_screen_init(void)
{
    lv_obj_t * ui_NameLabel;
    lv_obj_t * ui_NameTextLabel;
    lv_obj_t * ui_VersionLabel;
    lv_obj_t * ui_VersionTextLabel;
    lv_obj_t * ui_MCULabel;
    lv_obj_t * ui_MCUTextLabel;
    lv_obj_t * ui_OSLabel;
    lv_obj_t * ui_OSTextLabel;
    lv_obj_t * ui_GUILabel;
    lv_obj_t * ui_GUITextLabel;
    lv_obj_t * ui_AuthorSLabel;
    lv_obj_t * ui_AuthorSTextLabel;
		lv_obj_t * ui_AuthorGLabel;
    lv_obj_t * ui_AuthorGTextLabel;

    ui_AboutPage = lv_obj_create(NULL);
    /* 内容总高度大于屏高时若不滚动，底部几行会被父对象裁掉 */
    lv_obj_set_size(ui_AboutPage, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_add_flag(ui_AboutPage, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(ui_AboutPage, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(ui_AboutPage, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_bottom(ui_AboutPage, 28, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_NameLabel = lv_label_create(ui_AboutPage);
    lv_obj_set_width(ui_NameLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_NameLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_NameLabel, 20);
    lv_obj_set_y(ui_NameLabel, 15);
    lv_label_set_text(ui_NameLabel, "\xe6\x89\x8b\xe8\xa1\xa8\xe5\x9e\x8b\xe5\x8f\xb7");
    lv_obj_set_style_text_color(ui_NameLabel, lv_color_hex(0x1980E1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_NameLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_NameLabel, &ui_font_Cuyuan20, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_NameTextLabel = lv_label_create(ui_AboutPage);
    lv_obj_set_width(ui_NameTextLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_NameTextLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_NameTextLabel, 25);
    lv_obj_set_y(ui_NameTextLabel, 40);
    lv_label_set_text(ui_NameTextLabel, THESIS_WATCH_MODEL);
    lv_obj_set_width(ui_NameTextLabel, 210);
    lv_label_set_long_mode(ui_NameTextLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(ui_NameTextLabel, &ui_font_thesis_about20, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_VersionLabel = lv_label_create(ui_AboutPage);
    lv_obj_set_width(ui_VersionLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_VersionLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_VersionLabel, 20);
    lv_obj_set_y(ui_VersionLabel, 65);
    lv_label_set_text(ui_VersionLabel, "\xe5\x9b\xba\xe4\xbb\xb6\xe5\x9e\x8b\xe5\x8f\xb7");
    lv_obj_set_style_text_color(ui_VersionLabel, lv_color_hex(0x1980E1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_VersionLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_VersionLabel, &ui_font_Cuyuan20, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_VersionTextLabel = lv_label_create(ui_AboutPage);
    lv_obj_set_width(ui_VersionTextLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_VersionTextLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_VersionTextLabel, 25);
    lv_obj_set_y(ui_VersionTextLabel, 90);
    char str_buf[16];
    sprintf(str_buf, "V%d.%d.%d", watch_version_major(), watch_version_minor(), watch_version_patch());
    lv_label_set_text(ui_VersionTextLabel, str_buf);
    lv_obj_set_style_text_font(ui_VersionTextLabel, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_MCULabel = lv_label_create(ui_AboutPage);
    lv_obj_set_width(ui_MCULabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_MCULabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_MCULabel, 20);
    lv_obj_set_y(ui_MCULabel, 115);
    lv_label_set_text(ui_MCULabel, "\xe4\xb8\xbb\xe6\x8e\xa7\xe8\x8a\xaf\xe7\x89\x87");
    lv_obj_set_style_text_color(ui_MCULabel, lv_color_hex(0x1980E1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_MCULabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_MCULabel, &ui_font_Cuyuan20, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_MCUTextLabel = lv_label_create(ui_AboutPage);
    lv_obj_set_width(ui_MCUTextLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_MCUTextLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_MCUTextLabel, 25);
    lv_obj_set_y(ui_MCUTextLabel, 140);
    lv_label_set_text(ui_MCUTextLabel, "STM32F411CEU6");
    lv_obj_set_style_text_font(ui_MCUTextLabel, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_OSLabel = lv_label_create(ui_AboutPage);
    lv_obj_set_width(ui_OSLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_OSLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_OSLabel, 20);
    lv_obj_set_y(ui_OSLabel, 165);
    lv_label_set_text(ui_OSLabel, "\xe6\x93\x8d\xe4\xbd\x9c\xe7\xb3\xbb\xe7\xbb\x9f");
    lv_obj_set_style_text_color(ui_OSLabel, lv_color_hex(0x1980E1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_OSLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_OSLabel, &ui_font_Cuyuan20, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_OSTextLabel = lv_label_create(ui_AboutPage);
    lv_obj_set_width(ui_OSTextLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_OSTextLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_OSTextLabel, 25);
    lv_obj_set_y(ui_OSTextLabel, 190);
    lv_label_set_text(ui_OSTextLabel, "FreeRTOS");
    lv_obj_set_style_text_font(ui_OSTextLabel, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_GUILabel = lv_label_create(ui_AboutPage);
    lv_obj_set_width(ui_GUILabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_GUILabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_GUILabel, 20);
    lv_obj_set_y(ui_GUILabel, 215);
    lv_label_set_text(ui_GUILabel, "\xe5\x9b\xbe\xe5\xbd\xa2\xe7\x95\x8c\xe9\x9d\xa2");
    lv_obj_set_style_text_color(ui_GUILabel, lv_color_hex(0x1980E1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_GUILabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_GUILabel, &ui_font_Cuyuan20, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_GUITextLabel = lv_label_create(ui_AboutPage);
    lv_obj_set_width(ui_GUITextLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_GUITextLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_GUITextLabel, 25);
    lv_obj_set_y(ui_GUITextLabel, 240);
    lv_label_set_text(ui_GUITextLabel, "LVGL");
    lv_obj_set_style_text_font(ui_GUITextLabel, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_AuthorSLabel = lv_label_create(ui_AboutPage);
    lv_obj_set_width(ui_AuthorSLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_AuthorSLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_AuthorSLabel, 20);
    lv_obj_set_y(ui_AuthorSLabel, 265);
    lv_label_set_text(ui_AuthorSLabel, "\xe8\xbd\xaf\xe4\xbb\xb6\xe5\xbc\x80\xe5\x8f\x91\xe8\x80\x85");
    lv_obj_set_style_text_color(ui_AuthorSLabel, lv_color_hex(0x1980E1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_AuthorSLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_AuthorSLabel, &ui_font_Cuyuan20, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_AuthorSTextLabel = lv_label_create(ui_AboutPage);
    lv_obj_set_width(ui_AuthorSTextLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_AuthorSTextLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_AuthorSTextLabel, 25);
    lv_obj_set_y(ui_AuthorSTextLabel, 290);
    lv_label_set_text(ui_AuthorSTextLabel, THESIS_CREDIT_SW);
    lv_obj_set_width(ui_AuthorSTextLabel, 210);
    lv_label_set_long_mode(ui_AuthorSTextLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(ui_AuthorSTextLabel, &ui_font_thesis_about20, LV_PART_MAIN | LV_STATE_DEFAULT);

		ui_AuthorGLabel = lv_label_create(ui_AboutPage);
    lv_obj_set_width(ui_AuthorGLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_AuthorGLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_AuthorGLabel, 20);
    lv_obj_set_y(ui_AuthorGLabel, 315);
    lv_label_set_text(ui_AuthorGLabel, "\xe7\x95\x8c\xe9\x9d\xa2\xe8\xae\xbe\xe8\xae\xa1\xe8\x80\x85");
    lv_obj_set_style_text_color(ui_AuthorGLabel, lv_color_hex(0x1980E1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_AuthorGLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_AuthorGLabel, &ui_font_Cuyuan20, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_AuthorGTextLabel = lv_label_create(ui_AboutPage);
    lv_obj_set_width(ui_AuthorGTextLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_AuthorGTextLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_AuthorGTextLabel, 25);
    lv_obj_set_y(ui_AuthorGTextLabel, 340);
    lv_label_set_text(ui_AuthorGTextLabel, THESIS_CREDIT_UI);
    lv_obj_set_width(ui_AuthorGTextLabel, 210);
    lv_label_set_long_mode(ui_AuthorGTextLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(ui_AuthorGTextLabel, &ui_font_thesis_about20, LV_PART_MAIN | LV_STATE_DEFAULT);

}

//////////////////// SCREEN Deinit ////////////////////
void ui_AboutPage_screen_deinit(void)
{

}
