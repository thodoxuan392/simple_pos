/*
 * timer.c
 *
 *  Created on: May 28, 2023
 *      Author: xuanthodo
 */

#ifndef SRC_HAL_TIMER_C_
#define SRC_HAL_TIMER_C_

#include "main.h"
#include "Hal/timer.h"
#include "utils/utils_logger.h"

#define TIMER_FN_MAX_SIZE	10

static TIMER_fn fn_table[TIMER_FN_MAX_SIZE];
static size_t fn_table_len = 0;
static uint32_t tick_us = 0;

TIM_HandleTypeDef htim3 = {
	.Instance = TIM3,
	.Init = {
		.Prescaler = 63,
		.CounterMode = TIM_COUNTERMODE_UP,
		.Period = 999,
		.ClockDivision = TIM_CLOCKDIVISION_DIV1,
		.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE
	}
};

bool TIMER_init(){
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
	{
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
	{
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_TIM_Base_Start_IT(&htim3) != HAL_OK)
	{
		Error_Handler();
	}
	return true;
}

uint32_t TIMER_get_tick_us(){
	return tick_us + __HAL_TIM_GET_COUNTER(&htim3);
}

bool TIMER_attach_intr_1ms(void (*fn)(void)){
	if(fn_table_len >= TIMER_FN_MAX_SIZE){
		return false;
	}
	fn_table[fn_table_len++] = fn;
	return true;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef * htim){
	tick_us += 1000;
	if(htim->Instance == htim3.Instance){
		for (int fn_idx = 0; fn_idx < fn_table_len; ++fn_idx) {
			fn_table[fn_idx]();
		}
	}
}

#endif /* SRC_HAL_TIMER_C_ */
