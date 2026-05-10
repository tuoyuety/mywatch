/* 我上电后跑一遍硬件初始化，做完就结束这个任务 */

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

/* 这一段在 vTaskSuspendAll 里，我尽量别阻塞太久 */
void HardwareInitTask(void *argument)
{
	while(1)
	{
    vTaskSuspendAll();

    /* RTC 唤醒，后面睡眠要用 */
    if(HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 2000, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK)
    {
      Error_Handler();
    }

    /* 串口 DMA，蓝牙和调试命令从这里进 */
    HAL_UART_Receive_DMA(&huart1, (uint8_t *)HardInt_receive_str, 25);
    __HAL_UART_ENABLE_IT(&huart1,UART_IT_IDLE);

    /* 背光 PWM 先开起来 */
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_3);

    delay_init();

    /* 电源保持 */
    HWInterface.Power.Init();
    /* 先读一次电量，避免首页仍用 SquareLine 占位 70% 直到传感器任务才改 */
    {
      uint8_t pr = HWInterface.Power.BatCalculate();
      if (pr > 0u && pr <= 100u) {
        HWInterface.Power.power_remain = pr;
      } else {
        HWInterface.Power.power_remain = 0u;
      }
    }

    Key_Port_Init();

    /* 传感器逐个试，失败就记 ConnectionError */
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


    /* EEPROM：设置和步数 */
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


    /* 蓝牙：我先初始化再关掉省电 */
    HWInterface.BLE.Init();
    HWInterface.BLE.Disable();

    /* 触摸芯片复位 */
    CST816_GPIO_Init();
    CST816_RESET();

    /* 屏和开机动画 */
    LCD_Init();
    LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);
    delay_ms(10);
    LCD_Set_Light(50);
    thesis_boot_show_on_lcd();
    delay_ms(2500);
    LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);


    /* LVGL 再拉界面 */
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
    ui_init();

    xTaskResumeAll();
		vTaskDelete(NULL);
		osDelay(500);
	}
}

