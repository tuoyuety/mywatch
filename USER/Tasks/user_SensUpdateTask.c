/* Private includes -----------------------------------------------------------*/
//includes
#include "user_TasksInit.h"
#include "user_ScrRenewTask.h"
#include "user_SensUpdateTask.h"
#include "ui_HomePage.h"
#include "ui_MenuPage.h"
#include "ui_SetPage.h"
#include "ui_HRPage.h"
#include "ui_ENVPage.h"
#include "ui_CompassPage.h"
#include "main.h"

#include "AHT21.h"
#include "LSM303.h"
#include "SPL06_001.h"
#include "em70x8.h"
#include "HrAlgorythm.h"

#include "HWDataAccess.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
uint32_t user_HR_timecount=0;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  MPU6050 Check the state
  * @param  argument: Not used
  * @retval None
  */
void MPUCheckTask(void *argument)
{
	while(1)
	{
		if(HWInterface.IMU.wrist_is_enabled)
		{
			if(MPU_isHorizontal())
			{
				HWInterface.IMU.wrist_state = WRIST_UP;
			}
			else
			{
				if(WRIST_UP == HWInterface.IMU.wrist_state)
				{
					HWInterface.IMU.wrist_state = WRIST_DOWN;
					if( Page_Get_NowPage()->page_obj == &ui_HomePage ||
						Page_Get_NowPage()->page_obj == &ui_MenuPage ||
						Page_Get_NowPage()->page_obj == &ui_SetPage )
					{
						uint8_t Stopstr;
						osMessageQueuePut(Stop_MessageQueue, &Stopstr, 0, 1);//sleep
					}
				}
				HWInterface.IMU.wrist_state = WRIST_DOWN;
			}
		}
		osDelay(300);
	}
}

/**
  * @brief  HR data renew task
  * @param  argument: Not used
  * @retval None
  */
void HRDataUpdateTask(void *argument)
{
	uint8_t IdleBreakstr=0;
	uint8_t hr_temp=0;
	while(1)
	{
		if(Page_Get_NowPage()->page_obj == &ui_HRPage)
		{
			osMessageQueuePut(IdleBreak_MessageQueue, &IdleBreakstr, 0, 1);
			//sensor wake up
			EM7028_hrs_Enable();
			//receive the sensor wakeup message, sensor wakeup
			if(!HWInterface.HR_meter.ConnectionError)
			{
				vTaskSuspendAll();
				hr_temp = HR_Calculate(EM7028_Get_HRS1(),user_HR_timecount);
				xTaskResumeAll();
				if(HWInterface.HR_meter.HrRate != hr_temp && hr_temp>50 && hr_temp<120)
				{
					HWInterface.HR_meter.HrRate = hr_temp;
				}
			}
		}
		osDelay(50);
	}
}


/**
  * @brief  Sensor data update task
  * @param  argument: Not used
  * @retval None
  */
void SensorDataUpdateTask(void *argument)
{
	uint8_t IdleBreakstr=0;
	while(1)
	{
		// Update the sens data showed in Home
		uint8_t HomeUpdataStr;
		if(osMessageQueueGet(HomeUpdata_MessageQueue, &HomeUpdataStr, NULL, 0)==osOK)
		{
			//bat（0~10000 = 0.00%~100.00%）
			HWInterface.Power.power_remain = HWInterface.Power.BatCalculate();
			if (HWInterface.Power.power_remain > 10000u)
			{
				HWInterface.Power.power_remain = 0u;
			}

			//steps
			if(!(HWInterface.IMU.ConnectionError))
			{
				HWInterface.IMU.Steps = HWInterface.IMU.GetSteps();
			}

			//temp and humi
			if(!(HWInterface.AHT21.ConnectionError))
			{
				//temp and humi messure
				float humi,temp;
				HWInterface.AHT21.GetHumiTemp(&humi,&temp);
				//check
				if(temp>-10 && temp<50 && humi>0 && humi<100)
				{
					// ui_EnvTempValue = (int8_t)temp;
					// ui_EnvHumiValue = (int8_t)humi;
					HWInterface.AHT21.humidity = humi;
					HWInterface.AHT21.temperature = temp;
				}
			}

			//send data save message queue
			uint8_t Datastr = 3;
			osMessageQueuePut(DataSave_MessageQueue, &Datastr, 0, 1);

		}


		if(Page_Get_NowPage()->page_obj == &ui_EnvPage)
		{
			osMessageQueuePut(IdleBreak_MessageQueue, &IdleBreakstr, 0, 1);
			//receive the sensor wakeup message, sensor wakeup
			if(!HWInterface.AHT21.ConnectionError)
			{
				//temp and humi messure
				float humi,temp;
				HWInterface.AHT21.GetHumiTemp(&humi,&temp);
				//check
				if(temp>-10 && temp<50 && humi>0 && humi<100)
				{
					HWInterface.AHT21.temperature = (int8_t)temp;
					HWInterface.AHT21.humidity = (int8_t)humi;
				}
			}

		}
		// Compass page
		else if(Page_Get_NowPage()->page_obj == &ui_CompassPage)
		{
			osMessageQueuePut(IdleBreak_MessageQueue, &IdleBreakstr, 0, 1);
			//receive the sensor wakeup message, sensor wakeup
			LSM303DLH_Wakeup();
			//SPL_Wakeup();
			//if the sensor is no problem
			if(!HWInterface.Ecompass.ConnectionError)
			{
				//messure
				int16_t Xa,Ya,Za,Xm,Ym,Zm;
				LSM303_ReadAcceleration(&Xa,&Ya,&Za);
				LSM303_ReadMagnetic(&Xm,&Ym,&Zm);
				float temp = Azimuth_Calculate(Xa,Ya,Za,Xm,Ym,Zm)+0;//0 offset
				if(temp<0)
				{temp+=360;}
				//check
				if(temp>=0 && temp<=360)
				{
					HWInterface.Ecompass.direction = (uint16_t)temp;
				}
			}
			//if the sensor is no problem
			if(!HWInterface.Barometer.ConnectionError)
			{
				//messure
				float alti = Altitude_Calculate();
				//check
				if(1)
				{
					HWInterface.Barometer.altitude = (int16_t)alti;
				}
			}
		}

		osDelay(500);
	}
}
