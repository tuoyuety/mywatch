#include "power.h"
#include "adc.h"
#include "delay.h"

/* 出厂 VREFINT 校准字地址（勿命名为 VREFINT_CAL_ADDR，会与 LL 驱动头文件宏冲突） */
#define POWER_VREFINT_CAL_PTR ((const uint16_t *)0x1FFF7A2AU)

static float s_adc_vdda = 3.3f;
/* 电量换算前低通，减轻负载/充电脉冲导致的百分比乱跳 */
static float s_batvolt_lpf = -1.0f;
/* 充电中显示 SOC 单调不减：端电压受负载拉低时避免「越充越少」 */
static uint8_t s_soc_last_shown = 0xFFu;

static void power_adc_restore_battery_channel(void)
{
  ADC_ChannelConfTypeDef ch = {0};
  ch.Channel = ADC_CHANNEL_1;
  ch.Rank = 1;
  ch.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  (void)HAL_ADC_ConfigChannel(&hadc1, &ch);
}

void Power_UpdateAdcVddaCalibration(void)
{
  ADC_ChannelConfTypeDef ch = {0};
  uint32_t vref_sum = 0U;
  uint8_t n;

  ch.Channel = ADC_CHANNEL_VREFINT;
  ch.Rank = 1;
  ch.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &ch) != HAL_OK)
  {
    return;
  }

  /* 多采几次取平均，避免切通道后首样异常；校准字低 12 位有效 */
  for (n = 0U; n < 4U; n++)
  {
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100U) != HAL_OK)
    {
      HAL_ADC_Stop(&hadc1);
      power_adc_restore_battery_channel();
      return;
    }
    vref_sum += HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    delay_ms(1);
  }

  {
    uint32_t vref_adc = vref_sum >> 2U;
    uint32_t cal = (uint32_t)(*POWER_VREFINT_CAL_PTR) & 0xFFFU;

    if (vref_adc != 0U && cal >= 400U && cal <= 4095U)
    {
      float vdda = 3.3f * (float)cal / (float)vref_adc;
      /* 勿把上限卡死在 3.6V：USB/外部 LDO 供电时 VDDA 可能略高于 3.6，卡死会整体算低电量 */
      if (vdda < 2.5f)
      {
        vdda = 2.5f;
      }
      else if (vdda > 3.75f)
      {
        vdda = 3.75f;
      }
      s_adc_vdda = vdda;
    }
  }

  power_adc_restore_battery_channel();

  /* 从 VREFINT 切回 PA1 后丢弃一次转换，再采电池 */
  {
    uint16_t dummy;
    HAL_ADC_Start(&hadc1);
    (void)HAL_ADC_PollForConversion(&hadc1, 100U);
    dummy = (uint16_t)HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    (void)dummy;
  }
}

void Power_Pins_Init()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(POWER_PORT, POWER_PIN, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA3 */
  GPIO_InitStruct.Pin = POWER_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(POWER_PORT, &GPIO_InitStruct);

  /*Configure GPIO pin : PA2 — 与 OV-Watch 原版一致：无内部上下拉，由板级 CHRG/反相电路决定电平 */
  GPIO_InitStruct.Pin = CHARGE_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(CHARGE_PORT, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);
	
}

void Power_Enable()
{
	HAL_GPIO_WritePin(POWER_PORT,POWER_PIN,GPIO_PIN_SET);
}

void Power_DisEnable()
{
	HAL_GPIO_WritePin(POWER_PORT,POWER_PIN,GPIO_PIN_RESET);
}

uint8_t ChargeCheck(void) /* 1=充电中 */
{
  GPIO_PinState s = HAL_GPIO_ReadPin(CHARGE_PORT, CHARGE_PIN);
#if POWER_CHARGING_PIN_ACTIVE_HIGH
  return (s == GPIO_PIN_SET) ? 1U : 0U;
#else
  return (s == GPIO_PIN_RESET) ? 1U : 0U;
#endif
}

float BatCheck()
{
	uint16_t dat;
	float BatVoltage;
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1,5);
	dat = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	BatVoltage = (float)dat * BAT_VOLTAGE_DIVIDER_RATIO * s_adc_vdda / 4096.0f;
	BatVoltage *= BAT_VOLTAGE_TRIM_GAIN;
	return BatVoltage;
}

float BatCheck_8times()
{
	uint32_t dat = 0U;
	uint8_t i;
	float BatVoltage;

	for (i = 0U; i < 16U; i++)
	{
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, 5);
		dat += HAL_ADC_GetValue(&hadc1);
		HAL_ADC_Stop(&hadc1);
		delay_ms(1);
	}
	dat >>= 4U;
	BatVoltage = (float)dat * BAT_VOLTAGE_DIVIDER_RATIO * s_adc_vdda / 4096.0f;
	BatVoltage *= BAT_VOLTAGE_TRIM_GAIN;
	return BatVoltage;
}

uint8_t PowerCalculate(void)
{
	uint8_t power = 0U;
	float voltage;
	float raw;

	raw = BatCheck_8times();
	if (s_batvolt_lpf < 0.0f)
	{
		s_batvolt_lpf = raw;
	}
	else
	{
		s_batvolt_lpf = 0.78f * s_batvolt_lpf + 0.22f * raw;
	}
	voltage = s_batvolt_lpf;

	if (ChargeCheck())
	{
		voltage -= BAT_CHARGE_IR_COMPENSATION_V;
	}

	if (voltage >= 4.2f)
	{
		power = 100U;
	}
	else if (voltage >= 4.06f && voltage < 4.2f)
	{
		power = 90U;
	}
	else if (voltage >= 3.98f && voltage < 4.06f)
	{
		power = 80U;
	}
	else if (voltage >= 3.92f && voltage < 3.98f)
	{
		power = 70U;
	}
	else if (voltage >= 3.87f && voltage < 3.92f)
	{
		power = 60U;
	}
	else if (voltage >= 3.82f && voltage < 3.87f)
	{
		power = 50U;
	}
	else if (voltage >= 3.79f && voltage < 3.82f)
	{
		power = 40U;
	}
	else if (voltage >= 3.77f && voltage < 3.79f)
	{
		power = 30U;
	}
	else if (voltage >= 3.74f && voltage < 3.77f)
	{
		power = 20U;
	}
	else if (voltage >= 3.68f && voltage < 3.74f)
	{
		power = 10U;
	}
	else if (voltage >= 3.45f && voltage < 3.68f)
	{
		power = 5U;
	}
	else
	{
		/* 原逻辑：低于 3.45V 未赋值，为未定义行为；显式归为 0% */
		power = 0U;
	}

	/* 充电时：负载电流会在分压点上造成瞬时压降，换算 SOC 会误判下降 → 显示不低于上一拍 */
	if (ChargeCheck())
	{
		if (s_soc_last_shown != 0xFFu && power < s_soc_last_shown)
		{
			power = s_soc_last_shown;
		}
	}
	s_soc_last_shown = power;
	return power;
}

void Power_Init(void)
{
	Power_Pins_Init();
	Power_Enable();
	/* 用芯片内 VREFINT 反推实际 VDDA，避免写死 3.3V 导致电量整体偏低（如实测 3.9V 仍显示 5%） */
	Power_UpdateAdcVddaCalibration();
}


