/* Private includes -----------------------------------------------------------*/
//includes
#include "user_TasksInit.h"
#include "user_ScrRenewTask.h"
#include "user_RunModeTasks.h"
#include "ui_HomePage.h"
#include "ui_ChargPage.h"
#include "main.h"
#include "HWDataAccess.h"
#include "stm32f4xx_it.h"
#include "power.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static volatile uint8_t s_charge_ui_resync;

/* Private function prototypes -----------------------------------------------*/

void ChargeUi_NotifyResync(void)
{
	s_charge_ui_resync = 1U;
}

/*
 * 三次采样多数表决；若仍不一致则退回单次 ChargeCheck()，避免 PA2 抖动导致永远 0xFF 而不进充电页。
 */
static uint8_t charge_read_stable(void)
{
	uint8_t a = ChargeCheck();
	osDelay(6);
	uint8_t b = ChargeCheck();
	osDelay(6);
	uint8_t c = ChargeCheck();
	if (a == b || a == c)
	{
		return a;
	}
	if (b == c)
	{
		return b;
	}
	return ChargeCheck();
}

static void charge_apply_ui(uint8_t chg)
{
	if (chg == 1U && Page_Get_NowPage()->page_obj != &ui_ChargPage)
	{
		Page_Load(&Page_Charg);
	}
	else if (chg == 0U && Page_Get_NowPage()->page_obj == &ui_ChargPage)
	{
		Page_Back();
	}
}

/**
  * @brief  charg page enter task
  * @param  argument: Not used
  * @retval None
  * @note   轮询 PA2 充电状态；EXTI 仅用于清标志。检测极性见 power.h 中 POWER_CHARGING_PIN_ACTIVE_HIGH。
  */
void ChargPageEnterTask(void *argument)
{
	uint8_t last_chg = 0xFFu;

	while (1)
	{
		if (HardInt_Charg_flag)
		{
			IdleTimerCount = 0;
			HardInt_Charg_flag = 0;
			/* 边沿事件后重新与硬件对齐，避免仅依赖「状态变化」而漏掉亮屏后应显示的充电页 */
			s_charge_ui_resync = 1U;
		}

		if (s_charge_ui_resync)
		{
			s_charge_ui_resync = 0U;
			last_chg = 0xFFu;
		}

		{
			uint8_t chg = charge_read_stable();
			if (last_chg == 0xFFu)
			{
				last_chg = chg;
				charge_apply_ui(chg);
			}
			else if (chg != last_chg)
			{
				last_chg = chg;
				charge_apply_ui(chg);
			}
		}

		osDelay(200);
	}
}
