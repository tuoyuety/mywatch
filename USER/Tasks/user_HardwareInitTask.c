/* user_HardwareInitTask.c
 * 上电后第一个「干重活」的任务：把外设摸一遍，能用的用起来，用不了的别卡死整机。
 * 顺序大致按「先供电/时钟相关 → 传感器 → 存储 → 人机界面」走，和论文里系统初始化章节一一对应即可。
 */

#include "usart.h"
#include "tim.h"
#include "stm32f4xx_it.h"
#include "delay.h"

#include "user_TasksInit.h"
#include "HWDataAccess.h"

#include "key.h"
#include "lcd.h"
#include "lcd_init.h"
#include "CST816.h"
#include "DataSave.h"

#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "ui.h"

#include "ui_DateTimeSetPage.h"
#include "thesis_boot_lcd.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/**
 * @brief 硬件一次性初始化任务（只跑一轮，然后自删）
 * @note  外面包了 vTaskSuspendAll，意思是：这段里别指望别的任务帮你擦屁股，代码自己得是非阻塞的。
 */
void HardwareInitTask(void *argument)
{
	while(1)
	{
    vTaskSuspendAll();

    /* RTC 周期性唤醒：低功耗场景用得上；这里先配好，后面睡眠策略改起来省事 */
    if(HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 2000, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK)
    {
      Error_Handler();
    }

    /* 串口 DMA 收包 + IDLE 帧中断：给蓝牙/调试指令留入口 */
    HAL_UART_Receive_DMA(&huart1, (uint8_t *)HardInt_receive_str, 25);
    __HAL_UART_ENABLE_IT(&huart1,UART_IT_IDLE);

    /* 背光 PWM：先 Start，亮度具体多少等屏点亮后再 LCD_Set_Light */
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_3);

    delay_init();

    /* 电源保持脚拉高：电池方案里常见，不然 MCU 一睡板子就断电 */
    HWInterface.Power.Init();

    Key_Port_Init();

    /* 传感器：每类最多试 3 次，失败就标记 ConnectionError，别 while(1) 傻等 */
    uint8_t num = 3;
    while(num && HWInterface.AHT21.ConnectionError)
    {
      num--;
      HWInterface.AHT21.ConnectionError = HWInterface.AHT21.Init();
    }

    num = 3;
    while(num && HWInterface.Ecompass.ConnectionError)
    {
      num--;
      HWInterface.Ecompass.ConnectionError = HWInterface.Ecompass.Init();
    }
    if(!HWInterface.Ecompass.ConnectionError)
      HWInterface.Ecompass.Sleep();

    num = 3;
    while(num && HWInterface.Barometer.ConnectionError)
    {
      num--;
      HWInterface.Barometer.ConnectionError = HWInterface.Barometer.Init();
    }

    num = 3;
    while(num && HWInterface.IMU.ConnectionError)
    {
      num--;
      HWInterface.IMU.ConnectionError = HWInterface.IMU.Init();
    }

    num = 3;
    while(num && HWInterface.HR_meter.ConnectionError)
    {
      num--;
      HWInterface.HR_meter.ConnectionError = HWInterface.HR_meter.Init();
    }
    if(!HWInterface.HR_meter.ConnectionError)
      HWInterface.HR_meter.Sleep();


    /* EEPROM：存点用户偏好、步数缓存之类；读失败就走默认策略 */
    EEPROM_Init();
    if(!EEPROM_Check())
    {
      uint8_t recbuf[3];
      SettingGet(recbuf,0x10,2);
      if((recbuf[0]!=0 && recbuf[0]!=1) || (recbuf[1]!=0 && recbuf[1]!=1))
      {
        HWInterface.IMU.wrist_is_enabled = 0;
        ui_APPSy_EN = 0;
      }
      else
      {
        HWInterface.IMU.wrist_is_enabled = recbuf[0];
        ui_APPSy_EN = recbuf[1];
      }

      RTC_DateTypeDef nowdate;
      HAL_RTC_GetDate(&hrtc,&nowdate,RTC_FORMAT_BIN);

      SettingGet(recbuf,0x20,3);
      if(recbuf[0] == nowdate.Date)
      {
        uint16_t steps=0;
        steps = recbuf[1]&0x00ff;
        steps = steps<<8 | recbuf[2];
        if(!HWInterface.IMU.ConnectionError)
          dmp_set_pedometer_step_count((unsigned long)steps);
      }
    }


    /* 蓝牙模组：只做了 GPIO/关断，具体透传协议可在毕设「通信层」展开写 */
    HWInterface.BLE.Init();
    HWInterface.BLE.Disable();

    /* 触摸：I2C 软总线 + 复位脚，电容屏能摸之前必须先过这关 */
    CST816_GPIO_Init();
    CST816_RESET();

    /* 显示链路：GPIO 模拟命令 + SPI 刷像素；背光 PWM 前面已开 */
    LCD_Init();
    LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);
    delay_ms(10);
    LCD_Set_Light(50);
    thesis_boot_show_on_lcd();
    delay_ms(2500);
    LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);


    /* LVGL：图形栈初始化完再 ui_init，否则对象树没根 */
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
    ui_init();

    xTaskResumeAll();
		vTaskDelete(NULL);
		osDelay(500);
	}
}

