#include "ui.h"
#include "ui_helpers.h"
#include "ui_HomePage.h"
#include "ui_MenuPage.h"
#include "ui_SportPage.h"
#include "ui_HRPage.h"
#include "ui_SetPage.h"
#include "HWDataAccess.h"

#define HOME_STEP_GOAL    8000U

///////////////////// VARIABLES ////////////////////
//home page
lv_obj_t * ui_HomePage;
lv_obj_t * ui_TimeHourLabel;
lv_obj_t * ui_TimeColonLabel;
lv_obj_t * ui_TimeMinuteLabel;
lv_obj_t * ui_BaticonLabel;
lv_obj_t * ui_BatNumLabel;
lv_obj_t * ui_DateLabel;
lv_obj_t * ui_DayLabel;
lv_obj_t * ui_StepiconLabel;
lv_obj_t * ui_StepCnLabel;
lv_obj_t * ui_StepNumLabel;
lv_obj_t * ui_StepNumBar;
lv_obj_t * ui_StepTapPanel = NULL;
lv_obj_t * ui_TempiconLabel;
lv_obj_t * ui_TempNumLabel;
lv_obj_t * ui_HumiiconLabel;
lv_obj_t * ui_HumiNumLabel;
lv_obj_t * ui_HRiconLabel;
lv_obj_t * ui_HRNumLabel;
//drop down
lv_obj_t * ui_DropDownPanel;
lv_obj_t * ui_UpBGPanel;
lv_obj_t * ui_NFCButton;
lv_obj_t * ui_NFCLabel;
lv_obj_t * ui_BLEButton;
lv_obj_t * ui_BLELabel;
lv_obj_t * ui_PowerButton;
lv_obj_t * ui_PowerLabel;
lv_obj_t * ui_SetButton;
lv_obj_t * ui_SetLabel;
lv_obj_t * ui_LightSlider;
lv_obj_t * ui_LightLabel;
lv_obj_t * ui_DownBGPanel;
//power slider
lv_obj_t * ui_PowerPage;
lv_obj_t * ui_PowerSlider;
lv_obj_t * ui_PowerDownLabel;

lv_timer_t * ui_HomePageTimer;

// variables
uint8_t ui_TimeHourValue = 11;
uint8_t ui_TimeMinuteValue = 59;
const char * ui_Days[7] = {"Mon.", "Tue.", "Wed.", "Thu.", "Fri.", "Sat.", "Sun."};
uint8_t ui_DateMonthValue = 11;
uint8_t ui_DateDayValue = 8;
uint8_t ui_DataWeekdayValue = 2;

uint8_t ui_BatArcValue = 70;
uint16_t ui_StepNumValue = 0;
uint8_t ui_LightSliderValue = 50;

uint8_t ui_HomePageBLEEN = 0;
uint8_t ui_HomePageNFCEN = 0;

int8_t ui_HomePageTempValue = 25;
int8_t ui_HomePageHumiValue = 67;


///////////////////// Page Manager //////////////////
Page_t Page_Home = {ui_HomePage_screen_init, ui_HomePage_screen_deinit, &ui_HomePage};
Page_t Page_Power = {ui_PowerPage_screen_init, ui_PowerPage_screen_deinit, &ui_PowerPage};

/////////////////// private timer ///////////////////
// need to be destroyed when the page is destroyed
static void HomePage_timer_cb(lv_timer_t * timer)
{
  char value_strbuf[10];
  if(Page_Get_NowPage()->page_obj  == &ui_HomePage)
	{
			/*
			lv_obj_set_style_text_opa(ui_TimeColonLabel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
			osDelay(500);
			lv_obj_set_style_text_opa(ui_TimeColonLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
			*/

			HW_DateTimeTypeDef DateTime;
      HWInterface.RealTimeClock.GetTimeDate(&DateTime);

			if(ui_TimeMinuteValue != DateTime.Minutes)
			{
				ui_TimeMinuteValue = DateTime.Minutes;
				sprintf(value_strbuf, "%02d",ui_TimeMinuteValue);
				lv_label_set_text(ui_TimeMinuteLabel, value_strbuf);
			}

			if(ui_TimeHourValue != DateTime.Hours)
			{
				ui_TimeHourValue = DateTime.Hours;
				sprintf(value_strbuf, "%2d",ui_TimeHourValue);
				lv_label_set_text(ui_TimeHourLabel, value_strbuf);
			}

			if(ui_DateDayValue != DateTime.Date)
			{
				ui_DateDayValue = DateTime.Date;
				ui_DataWeekdayValue = DateTime.WeekDay;
				sprintf(value_strbuf, "%2d-%02d",ui_DateMonthValue,ui_DateDayValue);
				lv_label_set_text(ui_DateLabel, value_strbuf);
				lv_label_set_text(ui_DayLabel, ui_Days[ui_DataWeekdayValue-1]);

			}
			if(ui_DateMonthValue != DateTime.Month)
			{
				ui_DateMonthValue = DateTime.Month;
				ui_DateDayValue = DateTime.Date;
				ui_DataWeekdayValue = DateTime.WeekDay;
				sprintf(value_strbuf, "%2d-%02d",ui_DateMonthValue,ui_DateDayValue);
				lv_label_set_text(ui_DateLabel, value_strbuf);
				lv_label_set_text(ui_DayLabel, ui_Days[ui_DataWeekdayValue-1]);
			}

      if(ui_BatArcValue != HWInterface.Power.power_remain)
      {
        ui_BatArcValue = HWInterface.Power.power_remain;
				sprintf(value_strbuf, "%d%%",ui_BatArcValue);
				lv_label_set_text(ui_BatNumLabel, value_strbuf);
      }

      if(ui_StepNumValue != HWInterface.IMU.Steps)
      {
        ui_StepNumValue = HWInterface.IMU.Steps;
        sprintf(value_strbuf, "%d",ui_StepNumValue);
				lv_label_set_text(ui_StepNumLabel, value_strbuf);
        {
          uint32_t g = HOME_STEP_GOAL;
          int pct = (int)((ui_StepNumValue * 100U) / (g ? g : 1U));
          if(pct > 100) {
            pct = 100;
          }
          lv_bar_set_value(ui_StepNumBar, pct, LV_ANIM_OFF);
        }
      }

      if( ui_HomePageTempValue != (int8_t)HWInterface.AHT21.temperature )
      {
        ui_HomePageTempValue = (int8_t)HWInterface.AHT21.temperature;
        sprintf(value_strbuf, "%d",ui_HomePageTempValue);
				lv_label_set_text(ui_TempNumLabel, value_strbuf);
      }

      if( ui_HomePageHumiValue != (int8_t)HWInterface.AHT21.humidity )
      {
        ui_HomePageHumiValue = (int8_t)HWInterface.AHT21.humidity;
				sprintf(value_strbuf, "%d",ui_HomePageHumiValue);
				lv_label_set_text(ui_HumiNumLabel, value_strbuf);
      }

      {
        uint16_t hr = HWInterface.HR_meter.HrRate;
        static uint16_t s_last_hr = 0xFFFFu;
        if(hr != s_last_hr) {
          s_last_hr = hr;
          sprintf(value_strbuf, "%u", (unsigned)hr);
          lv_label_set_text(ui_HRNumLabel, value_strbuf);
        }
      }

	}
}


///////////////////// FUNCTIONS ////////////////////
void ui_event_StepTapPanel(lv_event_t * e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
#if SPORT_PAGE_EN
    Page_Load(&Page_Sport);
#endif
}

void ui_event_HomePage(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);

    if(event_code == LV_EVENT_GESTURE)
    {
			if(lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
      {
				Page_Load(&Page_Menu);
      }
		}
}

void ui_event_NFCButton(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if(event_code == LV_EVENT_VALUE_CHANGED &&  lv_obj_has_state(target, LV_STATE_CHECKED))
    {
        //checked
        ui_HomePageNFCEN=1;
				//ICcard_Select(1);
    }
    if(event_code == LV_EVENT_VALUE_CHANGED &&  !lv_obj_has_state(target, LV_STATE_CHECKED))
    {
        //released
        ui_HomePageNFCEN=0;
				//ICcard_Select(0);
    }
}

void ui_event_BLEButton(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if(event_code == LV_EVENT_VALUE_CHANGED &&  lv_obj_has_state(target, LV_STATE_CHECKED))
    {
        //checked
        HWInterface.BLE.Enable();
        ui_HomePageBLEEN=1;

    }
    if(event_code == LV_EVENT_VALUE_CHANGED &&  !lv_obj_has_state(target, LV_STATE_CHECKED))
    {
        //released
				HWInterface.BLE.Disable();
        ui_HomePageBLEEN=0;
    }
}

void ui_event_PowerButton(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if(event_code == LV_EVENT_CLICKED)
    {
			//power slider
      Page_Load(&Page_Power);
			// ui_PowerPage_screen_init();
			// lv_scr_load_anim(ui_PowerPage,LV_SCR_LOAD_ANIM_MOVE_RIGHT,0,0,true);
			// user_Stack_Push(&ScrRenewStack,(long long int)&ui_PowerPage);
    }
}

void ui_event_PowerSlider(lv_event_t * e)
{
		lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if(event_code == LV_EVENT_VALUE_CHANGED)
    {

    }
		if(event_code == LV_EVENT_CLICKED)
		{
			//power down if slider value >= 90
			if(lv_slider_get_value(ui_PowerSlider) >=90)
			{
        HWInterface.Power.Shutdown();
			}
		}
}

void ui_event_SetButton(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if(event_code == LV_EVENT_CLICKED)
    {
			Page_Load(&Page_Set);
    }
}

void ui_event_LightSlider(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if(event_code == LV_EVENT_VALUE_CHANGED)
    {
        ui_LightSliderValue = lv_slider_get_value(ui_LightSlider);
        HWInterface.LCD.SetLight(ui_LightSliderValue);
    }
}

///////////////////// SCREEN init ////////////////////
void ui_HomePage_screen_init(void)
{
		ui_MenuScrollY = 0;
    char value_strbuf[10];

    ui_HomePage = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(ui_HomePage, lv_color_hex(0x0F1419), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_HomePage, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(ui_HomePage, LV_OBJ_FLAG_SCROLLABLE);

    /* 顶栏：日期 / 星期 */
    ui_DateLabel = lv_label_create(ui_HomePage);
    lv_obj_set_width(ui_DateLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_DateLabel, LV_SIZE_CONTENT);
    sprintf(value_strbuf, "%2d-%02d", ui_DateMonthValue, ui_DateDayValue);
    lv_label_set_text(ui_DateLabel, value_strbuf);
    lv_obj_set_style_text_color(ui_DateLabel, lv_color_hex(0x9AA5B8), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_DateLabel, &ui_font_Cuyuan18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(ui_DateLabel, LV_ALIGN_TOP_LEFT, 10, 8);

    ui_DayLabel = lv_label_create(ui_HomePage);
    lv_obj_set_width(ui_DayLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_DayLabel, LV_SIZE_CONTENT);
    lv_label_set_text(ui_DayLabel, ui_Days[ui_DataWeekdayValue - 1]);
    lv_obj_set_style_text_color(ui_DayLabel, lv_color_hex(0x6B7585), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_DayLabel, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(ui_DayLabel, LV_ALIGN_TOP_LEFT, 10, 30);

    /* 电量：线性图标 + 百分比 */
    ui_BaticonLabel = lv_label_create(ui_HomePage);
    lv_obj_set_width(ui_BaticonLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_BaticonLabel, LV_SIZE_CONTENT);
    lv_label_set_text(ui_BaticonLabel, "\xee\x98\xac");
    lv_obj_set_style_text_color(ui_BaticonLabel, lv_color_hex(0x3DDC84), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_BaticonLabel, &ui_font_iconfont24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(ui_BaticonLabel, LV_ALIGN_TOP_RIGHT, -10, 10);

    ui_BatNumLabel = lv_label_create(ui_HomePage);
    lv_obj_set_width(ui_BatNumLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_BatNumLabel, LV_SIZE_CONTENT);
    sprintf(value_strbuf, "%d%%", ui_BatArcValue);
    lv_label_set_text(ui_BatNumLabel, value_strbuf);
    lv_obj_set_style_text_color(ui_BatNumLabel, lv_color_hex(0xC5D0E0), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_BatNumLabel, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(ui_BatNumLabel, LV_ALIGN_TOP_RIGHT, -10, 36);

    /* 主时间 */
    ui_TimeHourLabel = lv_label_create(ui_HomePage);
    lv_obj_set_width(ui_TimeHourLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_TimeHourLabel, LV_SIZE_CONTENT);
    sprintf(value_strbuf, "%2d", ui_TimeHourValue);
    lv_label_set_text(ui_TimeHourLabel, value_strbuf);
    lv_obj_set_style_text_color(ui_TimeHourLabel, lv_color_hex(0xE8ECF3), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_TimeHourLabel, &ui_font_Cuyuan48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(ui_TimeHourLabel, LV_ALIGN_CENTER, -52, -88);

    ui_TimeColonLabel = lv_label_create(ui_HomePage);
    lv_obj_set_width(ui_TimeColonLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_TimeColonLabel, LV_SIZE_CONTENT);
    lv_label_set_text(ui_TimeColonLabel, ":");
    lv_obj_set_style_text_color(ui_TimeColonLabel, lv_color_hex(0x5F6775), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_TimeColonLabel, &ui_font_Cuyuan48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(ui_TimeColonLabel, LV_ALIGN_CENTER, -22, -88);

    ui_TimeMinuteLabel = lv_label_create(ui_HomePage);
    lv_obj_set_width(ui_TimeMinuteLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_TimeMinuteLabel, LV_SIZE_CONTENT);
    sprintf(value_strbuf, "%02d", ui_TimeMinuteValue);
    lv_label_set_text(ui_TimeMinuteLabel, value_strbuf);
    lv_obj_set_style_text_color(ui_TimeMinuteLabel, lv_color_hex(0xE8ECF3), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_TimeMinuteLabel, &ui_font_Cuyuan48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(ui_TimeMinuteLabel, LV_ALIGN_CENTER, 42, -88);

    /* 步数区背景条 */
    {
        lv_obj_t * step_bg = lv_obj_create(ui_HomePage);
        lv_obj_set_size(step_bg, 224, 88);
        lv_obj_align(step_bg, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(step_bg, lv_color_hex(0x1B2333), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(step_bg, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(step_bg, 14, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(step_bg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_all(step_bg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_clear_flag(step_bg, LV_OBJ_FLAG_SCROLLABLE);
    }

    ui_StepiconLabel = lv_label_create(ui_HomePage);
    lv_obj_set_width(ui_StepiconLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_StepiconLabel, LV_SIZE_CONTENT);
    lv_label_set_text(ui_StepiconLabel, "\xee\x9e\xa0");
    lv_obj_set_style_text_color(ui_StepiconLabel, lv_color_hex(0x7EB7FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_StepiconLabel, &ui_font_iconfont34, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(ui_StepiconLabel, LV_ALIGN_CENTER, -92, 4);

    ui_StepCnLabel = lv_label_create(ui_HomePage);
    lv_obj_set_width(ui_StepCnLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_StepCnLabel, LV_SIZE_CONTENT);
    lv_label_set_text(ui_StepCnLabel, "\xe6\xb4\xbb\xe5\x8a\xa8\xe6\xad\xa5\xe6\x95\xb0");
    lv_obj_set_style_text_color(ui_StepCnLabel, lv_color_hex(0x8B98B0), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_StepCnLabel, &ui_font_Cuyuan18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(ui_StepCnLabel, LV_ALIGN_CENTER, -14, -12);

    ui_StepNumLabel = lv_label_create(ui_HomePage);
    lv_obj_set_width(ui_StepNumLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_StepNumLabel, LV_SIZE_CONTENT);
    sprintf(value_strbuf, "%d", ui_StepNumValue);
    lv_label_set_text(ui_StepNumLabel, value_strbuf);
    lv_obj_set_style_text_color(ui_StepNumLabel, lv_color_hex(0xB8D4FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_StepNumLabel, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(ui_StepNumLabel, LV_ALIGN_CENTER, -14, 14);

    ui_StepNumBar = lv_bar_create(ui_HomePage);
    lv_bar_set_range(ui_StepNumBar, 0, 100);
    {
        uint32_t pct = (ui_StepNumValue * 100U) / (HOME_STEP_GOAL ? HOME_STEP_GOAL : 1U);
        if(pct > 100U) {
            pct = 100U;
        }
        lv_bar_set_value(ui_StepNumBar, (int32_t)pct, LV_ANIM_OFF);
    }
    lv_obj_set_size(ui_StepNumBar, 196, 6);
    lv_obj_align(ui_StepNumBar, LV_ALIGN_CENTER, 0, 34);
    lv_obj_set_style_radius(ui_StepNumBar, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_StepNumBar, lv_color_hex(0x2A3346), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_StepNumBar, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_StepNumBar, 3, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_StepNumBar, lv_color_hex(0x5B8CFF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_StepNumBar, LV_OPA_COVER, LV_PART_INDICATOR | LV_STATE_DEFAULT);

#if SPORT_PAGE_EN
    ui_StepTapPanel = lv_obj_create(ui_HomePage);
    lv_obj_set_size(ui_StepTapPanel, 224, 88);
    lv_obj_align(ui_StepTapPanel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(ui_StepTapPanel, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_StepTapPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(ui_StepTapPanel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(ui_StepTapPanel, ui_event_StepTapPanel, LV_EVENT_CLICKED, NULL);
    lv_obj_move_foreground(ui_StepTapPanel);
#endif

    /* 底部三列：气温 / 湿度 / 心率（无弧形表盘） */
    ui_TempiconLabel = lv_label_create(ui_HomePage);
    lv_obj_set_width(ui_TempiconLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_TempiconLabel, LV_SIZE_CONTENT);
    /* U+E659：与 ui_font_iconfont30 字库一致；勿用 U+E706（未编入 30 号字库会空白） */
    lv_label_set_text(ui_TempiconLabel, "\xee\x99\x99");
    lv_obj_set_style_text_color(ui_TempiconLabel, lv_color_hex(0xFFB547), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_TempiconLabel, &ui_font_iconfont30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(ui_TempiconLabel, LV_ALIGN_BOTTOM_MID, -86, -50);

    ui_TempNumLabel = lv_label_create(ui_HomePage);
    lv_obj_set_width(ui_TempNumLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_TempNumLabel, LV_SIZE_CONTENT);
    sprintf(value_strbuf, "%d", ui_HomePageTempValue);
    lv_label_set_text(ui_TempNumLabel, value_strbuf);
    lv_obj_set_style_text_color(ui_TempNumLabel, lv_color_hex(0xE8ECF3), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_TempNumLabel, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(ui_TempNumLabel, LV_ALIGN_BOTTOM_MID, -86, -22);

    ui_HumiiconLabel = lv_label_create(ui_HomePage);
    lv_obj_set_width(ui_HumiiconLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_HumiiconLabel, LV_SIZE_CONTENT);
    /* U+E7BC：湿度图标，避免与温度共用 U+E659 */
    lv_label_set_text(ui_HumiiconLabel, "\xee\x9e\xbc");
    lv_obj_set_style_text_color(ui_HumiiconLabel, lv_color_hex(0x5ED4F0), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_HumiiconLabel, &ui_font_iconfont30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(ui_HumiiconLabel, LV_ALIGN_BOTTOM_MID, 0, -50);

    ui_HumiNumLabel = lv_label_create(ui_HomePage);
    lv_obj_set_width(ui_HumiNumLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_HumiNumLabel, LV_SIZE_CONTENT);
    sprintf(value_strbuf, "%d", ui_HomePageHumiValue);
    lv_label_set_text(ui_HumiNumLabel, value_strbuf);
    lv_obj_set_style_text_color(ui_HumiNumLabel, lv_color_hex(0xE8ECF3), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_HumiNumLabel, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(ui_HumiNumLabel, LV_ALIGN_BOTTOM_MID, 0, -22);

    ui_HRiconLabel = lv_label_create(ui_HomePage);
    lv_obj_set_width(ui_HRiconLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_HRiconLabel, LV_SIZE_CONTENT);
    lv_label_set_text(ui_HRiconLabel, "\xee\x99\x92");
    lv_obj_set_style_text_color(ui_HRiconLabel, lv_color_hex(0xFF6B7A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_HRiconLabel, &ui_font_iconfont34, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(ui_HRiconLabel, LV_ALIGN_BOTTOM_MID, 86, -50);

    ui_HRNumLabel = lv_label_create(ui_HomePage);
    lv_obj_set_width(ui_HRNumLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_HRNumLabel, LV_SIZE_CONTENT);
    sprintf(value_strbuf, "%u", (unsigned)HWInterface.HR_meter.HrRate);
    lv_label_set_text(ui_HRNumLabel, value_strbuf);
    lv_obj_set_style_text_color(ui_HRNumLabel, lv_color_hex(0xE8ECF3), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_HRNumLabel, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(ui_HRNumLabel, LV_ALIGN_BOTTOM_MID, 86, -22);


    ui_DropDownPanel = lv_obj_create(ui_HomePage);
    lv_obj_set_width(ui_DropDownPanel, 240);
    lv_obj_set_height(ui_DropDownPanel, 420);
    lv_obj_set_x(ui_DropDownPanel, 0);
    lv_obj_set_y(ui_DropDownPanel, -10);
    lv_obj_set_align(ui_DropDownPanel, LV_ALIGN_TOP_MID);
    lv_obj_scroll_by(ui_DropDownPanel,0,-130,LV_ANIM_OFF);
    lv_obj_clear_flag(ui_DropDownPanel, LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_CHAIN);      /// Flags
    //lv_obj_clear_flag(ui_DropDownPanel, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_CHAIN);      /// Flags
    lv_obj_set_scroll_dir(ui_DropDownPanel, LV_DIR_VER);
    lv_obj_set_style_bg_color(ui_DropDownPanel, lv_color_hex(0x323232), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_DropDownPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_DropDownPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_DropDownPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_DropDownPanel, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_DropDownPanel, 0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);

    ui_UpBGPanel = lv_obj_create(ui_DropDownPanel);
    lv_obj_set_width(ui_UpBGPanel, 240);
    lv_obj_set_height(ui_UpBGPanel, 130);
    lv_obj_set_x(ui_UpBGPanel, 0);
    lv_obj_set_y(ui_UpBGPanel, -10);
    lv_obj_set_align(ui_UpBGPanel, LV_ALIGN_TOP_MID);
    lv_obj_clear_flag(ui_UpBGPanel, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_UpBGPanel, lv_color_hex(0x323232), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_UpBGPanel, 200, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_NFCButton = lv_btn_create(ui_DropDownPanel);
    lv_obj_set_width(ui_NFCButton, 50);
    lv_obj_set_height(ui_NFCButton, 50);
		lv_obj_set_x(ui_NFCButton, 0);
    lv_obj_set_y(ui_NFCButton, 5);
    lv_obj_add_flag(ui_NFCButton, LV_OBJ_FLAG_CHECKABLE | LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_clear_flag(ui_NFCButton, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_NFCButton, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_NFCButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_NFCButton, lv_color_hex(0x3264C8), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(ui_NFCButton, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    if(ui_HomePageNFCEN)
    {lv_obj_add_state(ui_NFCButton, LV_STATE_CHECKED);}

    ui_NFCLabel = lv_label_create(ui_NFCButton);
    lv_obj_set_width(ui_NFCLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_NFCLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_NFCLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_NFCLabel, "\xee\x9e\x88");
    lv_obj_set_style_text_color(ui_NFCLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_NFCLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_NFCLabel, &ui_font_iconfont34, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_BLEButton = lv_btn_create(ui_DropDownPanel);
    lv_obj_set_width(ui_BLEButton, 50);
    lv_obj_set_height(ui_BLEButton, 50);
    lv_obj_set_x(ui_BLEButton, 0);
    lv_obj_set_y(ui_BLEButton, 65);
    lv_obj_add_flag(ui_BLEButton, LV_OBJ_FLAG_CHECKABLE | LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_clear_flag(ui_BLEButton, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_BLEButton, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_BLEButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_BLEButton, lv_color_hex(0x3264C8), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(ui_BLEButton, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    if(ui_HomePageBLEEN)
    {lv_obj_add_state(ui_BLEButton, LV_STATE_CHECKED);}

    ui_BLELabel = lv_label_create(ui_BLEButton);
    lv_obj_set_width(ui_BLELabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_BLELabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_BLELabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_BLELabel, "\xee\x99\x93");
    lv_obj_set_style_text_color(ui_BLELabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_BLELabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_BLELabel, &ui_font_iconfont34, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_PowerButton = lv_btn_create(ui_DropDownPanel);
    lv_obj_set_width(ui_PowerButton, 50);
    lv_obj_set_height(ui_PowerButton, 50);
	lv_obj_set_x(ui_PowerButton, 0);
    lv_obj_set_y(ui_PowerButton, 5);
    lv_obj_set_align(ui_PowerButton, LV_ALIGN_TOP_MID);
    lv_obj_add_flag(ui_PowerButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_clear_flag(ui_PowerButton, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_PowerButton, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_PowerButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_PowerLabel = lv_label_create(ui_PowerButton);
    lv_obj_set_width(ui_PowerLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_PowerLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_PowerLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_PowerLabel, "\xee\x98\xac");
    lv_obj_set_style_text_color(ui_PowerLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_PowerLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_PowerLabel, &ui_font_iconfont34, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_SetButton = lv_btn_create(ui_DropDownPanel);
    lv_obj_set_width(ui_SetButton, 50);
    lv_obj_set_height(ui_SetButton, 50);
    lv_obj_set_x(ui_SetButton, 0);
    lv_obj_set_y(ui_SetButton, 65);
    lv_obj_set_align(ui_SetButton, LV_ALIGN_TOP_MID);
    lv_obj_add_flag(ui_SetButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_clear_flag(ui_SetButton, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_SetButton, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_SetButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_SetLabel = lv_label_create(ui_SetButton);
    lv_obj_set_width(ui_SetLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_SetLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_SetLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_SetLabel, "\xee\x98\x80");
    lv_obj_set_style_text_color(ui_SetLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_SetLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_SetLabel, &ui_font_iconfont34, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_LightSlider = lv_slider_create(ui_DropDownPanel);
    lv_slider_set_value(ui_LightSlider, ui_LightSliderValue, LV_ANIM_OFF);
    if(lv_slider_get_mode(ui_LightSlider) == LV_SLIDER_MODE_RANGE) lv_slider_set_left_value(ui_LightSlider, 0, LV_ANIM_OFF);
    lv_obj_set_width(ui_LightSlider, 50);
    lv_obj_set_height(ui_LightSlider, 110);
	lv_obj_set_x(ui_LightSlider, 0);
    lv_obj_set_y(ui_LightSlider, 5);
    lv_obj_set_align(ui_LightSlider, LV_ALIGN_TOP_RIGHT);
    lv_obj_set_style_radius(ui_LightSlider, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_LightSlider, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_LightSlider, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_radius(ui_LightSlider, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_LightSlider, lv_color_hex(0x3264C8), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_LightSlider, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_LightSlider, lv_color_hex(0x000000), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_LightSlider, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    ui_LightLabel = lv_label_create(ui_LightSlider);
    lv_obj_set_width(ui_LightLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_LightLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_LightLabel, 0);
    lv_obj_set_y(ui_LightLabel, 30);
    lv_obj_set_align(ui_LightLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_LightLabel, "\xee\xa6\x86");
    lv_obj_set_style_text_color(ui_LightLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_LightLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_LightLabel, &ui_font_iconfont34, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_DownBGPanel = lv_obj_create(ui_DropDownPanel);
    lv_obj_set_width(ui_DownBGPanel, 240);
    lv_obj_set_height(ui_DownBGPanel, 130);
    lv_obj_set_x(ui_DownBGPanel, 0);
    lv_obj_set_y(ui_DownBGPanel, 420);
    lv_obj_set_align(ui_DownBGPanel, LV_ALIGN_TOP_MID);
    lv_obj_clear_flag(ui_DownBGPanel, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_DownBGPanel, lv_color_hex(0x323232), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_DownBGPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_DownBGPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_DownBGPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

		//events
		lv_obj_add_event_cb(ui_HomePage, ui_event_HomePage, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_NFCButton, ui_event_NFCButton, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_BLEButton, ui_event_BLEButton, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_PowerButton, ui_event_PowerButton, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_SetButton, ui_event_SetButton, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_LightSlider,ui_event_LightSlider, LV_EVENT_ALL, NULL);

    //timer
    ui_HomePageTimer = lv_timer_create(HomePage_timer_cb, 500,  NULL);

}


void ui_PowerPage_screen_init(void)
{
    ui_PowerPage = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_PowerPage, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_PowerSlider = lv_slider_create(ui_PowerPage);
    lv_obj_set_width(ui_PowerSlider, 220);
    lv_obj_set_height(ui_PowerSlider, 50);
    lv_obj_set_align(ui_PowerSlider, LV_ALIGN_CENTER);
    lv_obj_set_style_bg_color(ui_PowerSlider, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_PowerSlider, 128, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_PowerSlider, lv_color_hex(0x800000), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_PowerSlider, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_PowerSlider, lv_color_hex(0x800000), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_PowerSlider, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    ui_PowerDownLabel = lv_label_create(ui_PowerSlider);
    lv_obj_set_width(ui_PowerDownLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_PowerDownLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_PowerDownLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_PowerDownLabel, "\xe6\xbb\x91\xe5\x8a\xa8\xe5\x85\xb3\xe6\x9c\xba");
    lv_obj_set_style_text_color(ui_PowerDownLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_PowerDownLabel, 200, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_PowerDownLabel, &ui_font_Cuyuan20, LV_PART_MAIN | LV_STATE_DEFAULT);

		//events
		lv_obj_add_event_cb(ui_PowerSlider, ui_event_PowerSlider, LV_EVENT_ALL, NULL);

}

/////////////////// SCREEN deinit ////////////////////
void ui_HomePage_screen_deinit(void)
{
  lv_timer_del(ui_HomePageTimer);
}

void ui_PowerPage_screen_deinit(void)
{}
