#ifndef _POWER_H_
#define _POWER_H_

#include "stm32f4xx_hal.h"

/*
 * 充电检测脚有效电平（须与原理图一致）：
 * OV-Watch 原版 power.c 为「读脚直接当结果」，注释 1=充电，对应 MCU 上该脚在充电中为高电平 → 保持为 1。
 * 若你板子 TP4056 的 #CHRG 直接进 MCU（充电中脚为低），再改为 0。
 */
#ifndef POWER_CHARGING_PIN_ACTIVE_HIGH
#define POWER_CHARGING_PIN_ACTIVE_HIGH 1
#endif

/*
 * 电池经电阻分压后送到 PA1：Vbat = V(PA1) * BAT_VOLTAGE_DIVIDER_RATIO。
 * 对半分压（两电阻相等）时为 2；若你板子不是 1:1 分压，按原理图改成 (R上+R下)/R下。
 */
#ifndef BAT_VOLTAGE_DIVIDER_RATIO
#define BAT_VOLTAGE_DIVIDER_RATIO (2.0f)
#endif

/*
 * 总增益微调：万用表测电池端电压 ÷ BatCheck_8times() 返回值。默认 1.0；
 * 若整体偏低再在工程选项里加 -DBAT_VOLTAGE_TRIM_GAIN=1.06 等微调（勿过大，否则 3.9V 会顶到 100%）。
 */
#ifndef BAT_VOLTAGE_TRIM_GAIN
#define BAT_VOLTAGE_TRIM_GAIN (1.0f)
#endif

/*
 * 充电时从采样电压里减去的「等效 IR 压降」，用于抵消恒流阶段端电压虚高；过大会把电量算得越来越低。
 * 若仍觉得充电时百分比掉，可改为 0.03f 或 0。
 */
#ifndef BAT_CHARGE_IR_COMPENSATION_V
#define BAT_CHARGE_IR_COMPENSATION_V (0.06f)
#endif

#define BAT_CHECK_PORT	GPIOA
#define BAT_CHECK_PIN		GPIO_PIN_1

#define CHARGE_PORT			GPIOA
#define CHARGE_PIN			GPIO_PIN_2

#define POWER_PORT			GPIOA
#define POWER_PIN				GPIO_PIN_3

void Power_Pins_Init(void);
void Power_Enable(void);
void Power_DisEnable(void);
float BatCheck(void);
float BatCheck_8times(void);
uint8_t ChargeCheck(void);
uint8_t PowerCalculate(void);
void Power_Init(void);
void Power_UpdateAdcVddaCalibration(void);

#endif
