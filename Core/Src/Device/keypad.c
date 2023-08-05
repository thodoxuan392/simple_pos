/*
 * keypad.c
 *
 *  Created on: May 14, 2023
 *      Author: xuanthodo
 */


#include "main.h"
#include "Device/keypad.h"
#include "Hal/gpio.h"

enum {
	KEYPAD_R1,
	KEYPAD_R2,
	KEYPAD_R3,
	KEYPAD_R4,
	KEYPAD_C1,
	KEYPAD_C2,
	KEYPAD_C3,
	KEYPAD_LEDP,	// LED+
};

static GPIO_info_t gpio_table[] = {
		[KEYPAD_C1] = 		{GPIOD, { GPIO_PIN_0, GPIO_MODE_INPUT, GPIO_PULLDOWN, GPIO_SPEED_FREQ_HIGH}},
		[KEYPAD_C2] = 		{GPIOD,{ GPIO_PIN_1, GPIO_MODE_INPUT, GPIO_PULLDOWN, GPIO_SPEED_FREQ_HIGH }},
		[KEYPAD_C3] = 		{GPIOD,{ GPIO_PIN_2, GPIO_MODE_INPUT, GPIO_PULLDOWN, GPIO_SPEED_FREQ_HIGH }},
		[KEYPAD_R1] = 		{GPIOD,{ GPIO_PIN_3, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH }},
		[KEYPAD_R2] = 		{GPIOD,{ GPIO_PIN_4, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH }},
		[KEYPAD_R3] = 		{GPIOD,{ GPIO_PIN_5, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH }},
		[KEYPAD_R4] = 		{GPIOD,{ GPIO_PIN_6, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH }},
		[KEYPAD_LEDP] = 	{GPIOD,{ GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH }}
};

// Internal function
static uint8_t KEYPAD_calculate_key(uint8_t row, uint8_t col);

bool KEYPAD_init(){
	// Init GPIO
	int nb_io = sizeof(gpio_table)/sizeof(GPIO_info_t);
	for (uint8_t var = 0; var < nb_io; ++var) {
		HAL_GPIO_Init(gpio_table[var].port, &gpio_table[var].init_info);
	}
	// Enable Keypad
	HAL_GPIO_WritePin(gpio_table[KEYPAD_LEDP].port, gpio_table[KEYPAD_LEDP].init_info.Pin, RESET);
	return true;
}

uint16_t KEYPAD_get_status(){
	uint16_t keypad_status = 0x0000;
	for (uint16_t row_id = KEYPAD_R1; row_id <= KEYPAD_R4; row_id++) {
		HAL_GPIO_WritePin(gpio_table[row_id].port, gpio_table[row_id].init_info.Pin, SET);
		for (uint8_t col_id = KEYPAD_C1; col_id <= KEYPAD_C3; col_id++) {
			if(HAL_GPIO_ReadPin(gpio_table[col_id].port, gpio_table[col_id].init_info.Pin)){
				keypad_status |= 1 << KEYPAD_calculate_key(row_id, col_id);
			}
		}
		HAL_GPIO_WritePin(gpio_table[row_id].port, gpio_table[row_id].init_info.Pin, RESET);
	}
	return keypad_status;
}

static uint8_t KEYPAD_calculate_key(uint8_t row, uint8_t col){
	return (col - KEYPAD_C1) * 4 + (row - KEYPAD_R1);
}

bool KEYPAD_test(){
	uint8_t keypad_status = KEYPAD_get_status();
	uint8_t a = 1;
}

