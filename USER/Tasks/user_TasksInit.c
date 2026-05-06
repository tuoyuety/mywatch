/* 我：任务、队列、定时器在这里建 */

/* Private includes -----------------------------------------------------------*/
#include "user_TasksInit.h"
//sys
#include "sys.h"
#include "stdio.h"
#include "lcd.h"
#include "WDOG.h"
//gui
#include "lvgl.h"
#include "ui_TimerPage.h"
//tasks
#include "user_HardwareInitTask.h"
#include "user_RunModeTasks.h"
#include "user_KeyTask.h"
#include "user_ScrRenewTask.h"
#include "user_SensUpdateTask.h"
#include "user_ChargCheckTask.h"
#include "user_MessageSendTask.h"
#include "user_DataSaveTask.h"

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/


/* Timers --------------------------------------------------------------------*/
osTimerId_t IdleTimerHandle;


/* Tasks ---------------------------------------------------------------------*/
// Hardwares initialization
osThreadId_t HardwareInitTaskHandle;
const osThreadAttr_t HardwareInitTask_attributes = {
  .name = "HardwareInitTask",
  .stack_size = 128 * 10,
  .priority = (osPriority_t) osPriorityHigh3,
};

//LVGL Handler task
osThreadId_t LvHandlerTaskHandle;
const osThreadAttr_t LvHandlerTask_attributes = {
  .name = "LvHandlerTask",
  .stack_size = 128 * 24,
  .priority = (osPriority_t) osPriorityLow,
};

//WDOG Feed task
osThreadId_t WDOGFeedTaskHandle;
const osThreadAttr_t WDOGFeedTask_attributes = {
  .name = "WDOGFeedTask",
  .stack_size = 128 * 1,
  .priority = (osPriority_t) osPriorityHigh2,
};

//Idle Enter Task
osThreadId_t IdleEnterTaskHandle;
const osThreadAttr_t IdleEnterTask_attributes = {
  .name = "IdleEnterTask",
  .stack_size = 128 * 1,
  .priority = (osPriority_t) osPriorityHigh,
};

//Stop Enter Task
osThreadId_t StopEnterTaskHandle;
const osThreadAttr_t StopEnterTask_attributes = {
  .name = "StopEnterTask",
  .stack_size = 128 * 16,
  .priority = (osPriority_t) osPriorityHigh1,
};

//Key task
osThreadId_t KeyTaskHandle;
const osThreadAttr_t KeyTask_attributes = {
  .name = "KeyTask",
  .stack_size = 128 * 1,
  .priority = (osPriority_t) osPriorityNormal,
};

//ScrRenew task
osThreadId_t ScrRenewTaskHandle;
const osThreadAttr_t ScrRenewTask_attributes = {
  .name = "ScrRenewTask",
  .stack_size = 128 * 10,
  .priority = (osPriority_t) osPriorityLow1,
};

//SensorDataRenew task
osThreadId_t SensorDataTaskHandle;
const osThreadAttr_t SensorDataTask_attributes = {
  .name = "SensorDataTask",
  .stack_size = 128 * 5,
  .priority = (osPriority_t) osPriorityLow1,
};

//HRDataRenew task
osThreadId_t HRDataTaskHandle;
const osThreadAttr_t HRDataTask_attributes = {
  .name = "HRDataTask",
  .stack_size = 128 * 5,
  .priority = (osPriority_t) osPriorityLow1,
};

//ChargPageEnterTask
osThreadId_t ChargPageEnterTaskHandle;
const osThreadAttr_t ChargPageEnterTask_attributes = {
  .name = "ChargPageEnterTask",
  .stack_size = 128 * 10,
  .priority = (osPriority_t) osPriorityLow1,
};

//messagesendtask
osThreadId_t MessageSendTaskHandle;
const osThreadAttr_t MessageSendTask_attributes = {
  .name = "MessageSendTask",
  .stack_size = 128 * 5,
  .priority = (osPriority_t) osPriorityLow1,
};

//MPUCheckTask
osThreadId_t MPUCheckTaskHandle;
const osThreadAttr_t MPUCheckTask_attributes = {
  .name = "MPUCheckTask",
  .stack_size = 128 * 3,
  .priority = (osPriority_t) osPriorityLow2,
};

//DataSaveTask
osThreadId_t DataSaveTaskHandle;
const osThreadAttr_t DataSaveTask_attributes = {
  .name = "DataSaveTask",
  .stack_size = 128 * 5,
  .priority = (osPriority_t) osPriorityLow2,
};


/* Message queues ------------------------------------------------------------*/
//Key message
osMessageQueueId_t Key_MessageQueue;
osMessageQueueId_t Idle_MessageQueue;
osMessageQueueId_t Stop_MessageQueue;
osMessageQueueId_t IdleBreak_MessageQueue;
osMessageQueueId_t HomeUpdata_MessageQueue;
osMessageQueueId_t DataSave_MessageQueue;

/* Private function prototypes -----------------------------------------------*/
void LvHandlerTask(void *argument);
void WDOGFeedTask(void *argument);

/**
  * @brief 创建消息队列、软件定时器与各业务线程（调度器启动前调用一次即可）
  */
void User_Tasks_Init(void)
{
  /* 100 ms 周期的 idle 相关定时器：配合熄屏/唤醒策略，具体逻辑在回调里展开 */
	IdleTimerHandle = osTimerNew(IdleTimerCallback, osTimerPeriodic, NULL, NULL);
	osTimerStart(IdleTimerHandle,100);

  /* 队列一律短小精悍：单字节事件通知，避免大块数据在 ISR 和任务间乱传 */
	Key_MessageQueue  = osMessageQueueNew(1, 1, NULL);
	Idle_MessageQueue = osMessageQueueNew(1, 1, NULL);
	Stop_MessageQueue = osMessageQueueNew(1, 1, NULL);
	IdleBreak_MessageQueue = osMessageQueueNew(1, 1, NULL);
	HomeUpdata_MessageQueue = osMessageQueueNew(1, 1, NULL);
	DataSave_MessageQueue = osMessageQueueNew(2, 1, NULL);

	/* 线程优先级：初始化任务最高，喂狗次之，LVGL 与传感器刷新偏低，避免抢占重活 */
  HardwareInitTaskHandle  = osThreadNew(HardwareInitTask, NULL, &HardwareInitTask_attributes);
  LvHandlerTaskHandle  = osThreadNew(LvHandlerTask, NULL, &LvHandlerTask_attributes);
  WDOGFeedTaskHandle   = osThreadNew(WDOGFeedTask, NULL, &WDOGFeedTask_attributes);
	IdleEnterTaskHandle  = osThreadNew(IdleEnterTask, NULL, &IdleEnterTask_attributes);
	StopEnterTaskHandle  = osThreadNew(StopEnterTask, NULL, &StopEnterTask_attributes);
	KeyTaskHandle 			 = osThreadNew(KeyTask, NULL, &KeyTask_attributes);
	ScrRenewTaskHandle   = osThreadNew(ScrRenewTask, NULL, &ScrRenewTask_attributes);
	SensorDataTaskHandle = osThreadNew(SensorDataUpdateTask, NULL, &SensorDataTask_attributes);
	HRDataTaskHandle		 = osThreadNew(HRDataUpdateTask, NULL, &HRDataTask_attributes);
	ChargPageEnterTaskHandle = osThreadNew(ChargPageEnterTask, NULL, &ChargPageEnterTask_attributes);
  MessageSendTaskHandle = osThreadNew(MessageSendTask, NULL, &MessageSendTask_attributes);
	MPUCheckTaskHandle		= osThreadNew(MPUCheckTask, NULL, &MPUCheckTask_attributes);
	DataSaveTaskHandle		= osThreadNew(DataSaveTask, NULL, &DataSaveTask_attributes);

	/* 上电先踢一脚首页刷新队列，避免桌面卡在第一帧 */
	uint8_t HomeUpdataStr;
	osMessageQueuePut(HomeUpdata_MessageQueue, &HomeUpdataStr, 0, 1);

}


/**
  * @brief 系统节拍钩子：给 LVGL 喂时间，并顺带维护秒表页的软时钟。
  * @note 跑在 tick 中断里，别写阻塞逻辑；往队列里塞消息时 timeout 必须是 0。
  */
void TaskTickHook(void)
{
	lv_tick_inc(1);
	if(ui_TimerPageFlag)
	{
			ui_TimerPage_ms+=1;
			if(ui_TimerPage_ms>=10)
			{
				ui_TimerPage_ms=0;
				ui_TimerPage_10ms+=1;
			}
			if(ui_TimerPage_10ms>=100)
			{
					ui_TimerPage_10ms=0;
					ui_TimerPage_sec+=1;
					uint8_t IdleBreakstr = 0;
					osMessageQueuePut(IdleBreak_MessageQueue, &IdleBreakstr, 0, 0);
			}
			if(ui_TimerPage_sec>=60)
			{
					ui_TimerPage_sec=0;
					ui_TimerPage_min+=1;
			}
			if(ui_TimerPage_min>=60)
			{
					ui_TimerPage_min=0;
			}
	}
	user_HR_timecount+=1;
}


/**
  * @brief LVGL 专用线程：周期性 lv_task_handler，顺便根据「多久没摸屏」打断 idle 计时。
  */
void LvHandlerTask(void *argument)
{
	uint8_t IdleBreakstr=0;
  while(1)
  {
		if(lv_disp_get_inactive_time(NULL)<1000)
		{
			osMessageQueuePut(IdleBreak_MessageQueue, &IdleBreakstr, 0, 0);
		}
		lv_task_handler();
    osDelay(1);
	}
}


/**
  * @brief 外部看门狗喂狗任务：板子上若挂了专用 WDI 芯片，这里周期性翻转电平。
  */
void WDOGFeedTask(void *argument)
{
	WDOG_Port_Init();
  while(1)
  {
		WDOG_Feed();
		WDOG_Enable();
    osDelay(100);
  }
}


