#include "power.h"
#include "adc.h"
#include "delay.h"

/* 出厂 VREFINT 校准字地址（勿命名为 VREFINT_CAL_ADDR，会与 LL 驱动头文件宏冲突） */
#define POWER_VREFINT_CAL_PTR ((const uint16_t *)0x1FFF7A2AU)

static float s_adc_vdda = 3.3f;
/* 电量换算前低通，减轻负载/充电脉冲导致的百分比乱跳 */
static float s_batvolt_lpf = -1.0f;
/* 充电中显示 SOC 单调不减：端电压受负载拉低时避免「越充越少」；0xFFFF=未初始化 */
static uint16_t s_soc_last_shown = 0xFFFFu;
static uint8_t s_was_charging;

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

#if POWER_USE_BAT_FULL_PIN
  /* 充满检测脚：常见为 TP4056 #STDBY 开漏，空闲为高，充满拉低 */
  GPIO_InitStruct.Pin = BAT_FULL_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
#if POWER_BAT_FULL_PIN_ACTIVE_LOW
  GPIO_InitStruct.Pull = GPIO_PULLUP;
#else
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
#endif
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
#if defined(GPIOB)
  if (BAT_FULL_PORT == GPIOB) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
  }
#endif
#if defined(GPIOC)
  if (BAT_FULL_PORT == GPIOC) {
    __HAL_RCC_GPIOC_CLK_ENABLE();
  }
#endif
#if defined(GPIOD)
  if (BAT_FULL_PORT == GPIOD) {
    __HAL_RCC_GPIOD_CLK_ENABLE();
  }
#endif
  HAL_GPIO_Init(BAT_FULL_PORT, &GPIO_InitStruct);
#endif
	
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

uint8_t FullChargeCheck(void)
{
#if !POWER_USE_BAT_FULL_PIN
	return 0U;
#else
	GPIO_PinState s = HAL_GPIO_ReadPin(BAT_FULL_PORT, BAT_FULL_PIN);
#if POWER_BAT_FULL_PIN_ACTIVE_LOW
	return (s == GPIO_PIN_RESET) ? 1U : 0U;
#else
	return (s == GPIO_PIN_SET) ? 1U : 0U;
#endif
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

/*
 * 与原阶梯电压区间一致，拐点间线性插值，SOC 连续；再量化为 0.01%（厘百分）。
 * 例如 3.98~4.06V 间由 80% 平滑过渡到 90%，不再整段钉死在 80%。
 */
static float PowerVoltageToSocPercent(float v)
{
	const float v_full = BAT_FULL_VOLTAGE_V;
	static const float kv[10] = {
		3.45f, 3.68f, 3.74f, 3.77f, 3.79f,
		3.82f, 3.87f, 3.92f, 3.98f, 4.06f
	};
	static const float sv[10] = {
		5.f, 10.f, 20.f, 30.f, 40.f,
		50.f, 60.f, 70.f, 80.f, 90.f
	};

	if (v < kv[0]) {
		return 0.f;
	}
	if (v >= v_full) {
		return 100.f;
	}
	{
		int i;
		for (i = 0; i < 9; i++) {
			if (v < kv[i + 1]) {
				float v0 = kv[i];
				float v1 = kv[i + 1];
				float s0 = sv[i];
				float s1 = sv[i + 1];
				float t = (v - v0) / (v1 - v0);
				return s0 + t * (s1 - s0);
			}
		}
	}
	{
		float v0 = kv[9];
		float t = (v - v0) / (v_full - v0);
		if (t < 0.f) {
			t = 0.f;
		} else if (t > 1.f) {
			t = 1.f;
		}
		return sv[9] + t * (100.f - sv[9]);
	}
}

static uint16_t PowerSocPercentToCenti(float p)
{
	if (p <= 0.f) {
		return 0U;
	}
	if (p >= 100.f) {
		return 10000U;
	}
	return (uint16_t)(p * 100.f + 0.5f);
}

uint16_t PowerCalculate(void)
{
	float raw;
	uint8_t chg;
	uint8_t was_chg;
	float voltage;
	uint16_t raw_cent;

	raw = BatCheck_8times();
	chg = ChargeCheck();
	was_chg = s_was_charging;
	s_was_charging = chg;

	if (s_batvolt_lpf < 0.0f)
	{
		s_batvolt_lpf = raw;
	}
	else if (chg && !was_chg)
	{
		/* 刚插上电：与未充电同一套电压→电量表，先把滤波器拉到当前端压 */
		s_batvolt_lpf = raw;
	}
	else if (chg)
	{
		s_batvolt_lpf = 0.88f * s_batvolt_lpf + 0.12f * raw;
	}
	else
	{
		s_batvolt_lpf = 0.78f * s_batvolt_lpf + 0.22f * raw;
	}

	voltage = s_batvolt_lpf;

	if (BAT_CHARGE_IR_COMPENSATION_V > 0.0f && chg)
	{
		voltage -= BAT_CHARGE_IR_COMPENSATION_V;
	}

	raw_cent = PowerSocPercentToCenti(PowerVoltageToSocPercent(voltage));

	{
		uint8_t full = FullChargeCheck();
		uint16_t power_cent;

		if (full != 0U)
		{
			raw_cent = 10000U;
			power_cent = 10000U;
		}
		else if (chg != 0U)
		{
			/* 充电且未满：每调用最多 +1.00%（100 厘百分），向电压目标靠拢 */
			if (s_soc_last_shown == 0xFFFFu)
			{
				power_cent = raw_cent;
			}
			else
			{
				uint16_t target = raw_cent;
				uint32_t cap = (uint32_t)s_soc_last_shown + 100U;
				if (cap > 10000U)
				{
					cap = 10000U;
				}
				if ((uint32_t)target > cap)
				{
					power_cent = (uint16_t)cap;
				}
				else if (target < s_soc_last_shown)
				{
					power_cent = s_soc_last_shown;
				}
				else
				{
					power_cent = target;
				}
			}
		}
		else
		{
			power_cent = raw_cent;
		}

		s_soc_last_shown = power_cent;
		return power_cent;
	}
}

void Power_Init(void)
{
	Power_Pins_Init();
	Power_Enable();
	/* 用芯片内 VREFINT 反推实际 VDDA，避免写死 3.3V 导致电量整体偏低（如实测 3.9V 仍显示 5%） */
	Power_UpdateAdcVddaCalibration();
}


