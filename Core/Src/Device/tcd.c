/*
 * tcd.c
 *
 *  Created on: May 14, 2023
 *      Author: xuanthodo
 */


#include "main.h"
#include "Device/tcd.h"
#include "Hal/gpio.h"

enum {
	TCD_PAYOUT1_IO,
	TCD_RESET1_IO,
	TCD_CALLBACK1_IO,
	TCD_OUTOK1_IO,
	TCD_LOWER1_IO,
	TCD_ERROR1_IO,
	TCD_EMPTY1_IO,
	TCD_PAYOUT2_IO,
	TCD_RESET2_IO,
	TCD_CALLBACK2_IO,
	TCD_OUTOK2_IO,
	TCD_LOWER2_IO,
	TCD_ERROR2_IO,
	TCD_EMPTY2_IO,
};

static GPIO_info_t gpio_table[] = {
		[TCD_PAYOUT1_IO] = 		{GPIOE, { GPIO_PIN_8, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_MEDIUM}},
		[TCD_RESET1_IO] = 		{GPIOE,{ GPIO_PIN_9, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_MEDIUM }},
		[TCD_CALLBACK1_IO] = 	{GPIOE,{ GPIO_PIN_10, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_MEDIUM }},
		[TCD_OUTOK1_IO] = 		{GPIOB,{ GPIO_PIN_9, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_MEDIUM }},
		[TCD_LOWER1_IO] = 		{GPIOE,{ GPIO_PIN_0, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_MEDIUM }},
		[TCD_ERROR1_IO] = 		{GPIOE,{ GPIO_PIN_1, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_MEDIUM }},
		[TCD_EMPTY1_IO] = 		{GPIOE,{ GPIO_PIN_2, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_MEDIUM }},
		[TCD_PAYOUT2_IO] = 		{GPIOE,{ GPIO_PIN_12, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_MEDIUM }},
		[TCD_RESET2_IO] = 		{GPIOE,{ GPIO_PIN_13, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_MEDIUM }},
		[TCD_CALLBACK2_IO] = 	{GPIOE,{ GPIO_PIN_14, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_MEDIUM }},
		[TCD_OUTOK2_IO] = 		{GPIOE,{ GPIO_PIN_3, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_MEDIUM }},
		[TCD_LOWER2_IO] = 		{GPIOE,{ GPIO_PIN_4, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_MEDIUM }},
		[TCD_ERROR2_IO] = 		{GPIOE,{ GPIO_PIN_5, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_MEDIUM }},
		[TCD_EMPTY2_IO] = 		{GPIOE,{ GPIO_PIN_6, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_MEDIUM }},
};

bool TCD_init(){
	int nb_io = sizeof(gpio_table)/sizeof(GPIO_info_t);
	for (uint8_t var = 0; var < nb_io; ++var) {
		HAL_GPIO_Init(gpio_table[var].port, &gpio_table[var].init_info);
	}
	 TCD_reset(TCD_1, SET);
	 TCD_reset(TCD_2, SET);
	 HAL_Delay(200);
	 TCD_reset(TCD_1, RESET);
	 TCD_reset(TCD_2, RESET);
	 TCD_payout_card(TCD_1, false);
	 TCD_payout_card(TCD_2, false);
	 TCD_callback(TCD_1, false);
	 TCD_callback(TCD_2, false);
	return true;
}
bool TCD_loop(){
	return true;
}
bool TCD_payout_card(TCD_id_t id ,bool enable){
	uint8_t io_index = (id == TCD_1)? TCD_PAYOUT1_IO : TCD_PAYOUT2_IO;
	if(enable){
		HAL_GPIO_WritePin(gpio_table[io_index].port, gpio_table[io_index].init_info.Pin, SET);
	}else{
		HAL_GPIO_WritePin(gpio_table[io_index].port, gpio_table[io_index].init_info.Pin, RESET);
	}
}
bool TCD_reset(TCD_id_t id ,bool enable){
	uint8_t io_index = (id == TCD_1)? TCD_RESET1_IO : TCD_RESET2_IO;
	if(enable){
		HAL_GPIO_WritePin(gpio_table[io_index].port, gpio_table[io_index].init_info.Pin, SET);
	}else{
		HAL_GPIO_WritePin(gpio_table[io_index].port, gpio_table[io_index].init_info.Pin, RESET);
	}
}

bool TCD_callback(TCD_id_t id , bool enable){
	uint8_t io_index = (id == TCD_1)? TCD_CALLBACK1_IO : TCD_CALLBACK2_IO;
	if(enable){
		HAL_GPIO_WritePin(gpio_table[io_index].port, gpio_table[io_index].init_info.Pin, SET);
	}else{
		HAL_GPIO_WritePin(gpio_table[io_index].port, gpio_table[io_index].init_info.Pin, RESET);
	}
}


bool TCD_is_out_ok(TCD_id_t id ){
	uint8_t io_index = (id == TCD_1)? TCD_OUTOK1_IO : TCD_OUTOK2_IO;
	return !HAL_GPIO_ReadPin(gpio_table[io_index].port, gpio_table[io_index].init_info.Pin);
}

bool TCD_is_error(TCD_id_t id ){
	uint8_t io_index = (id == TCD_1)? TCD_ERROR1_IO : TCD_ERROR2_IO;
	return !HAL_GPIO_ReadPin(gpio_table[io_index].port, gpio_table[io_index].init_info.Pin);
}

bool TCD_is_lower(TCD_id_t id ){
	uint8_t io_index = (id == TCD_1)? TCD_LOWER1_IO : TCD_LOWER2_IO;
	return !HAL_GPIO_ReadPin(gpio_table[io_index].port, gpio_table[io_index].init_info.Pin);
}

bool TCD_is_empty(TCD_id_t id ){
	uint8_t io_index = (id == TCD_1)? TCD_EMPTY1_IO : TCD_EMPTY2_IO;
	return !HAL_GPIO_ReadPin(gpio_table[io_index].port, gpio_table[io_index].init_info.Pin);
}



bool TCD_test(){
//	static bool set = false;
//	set = !set;
	// Payout
//	TCD_payout_card(TCD_1, true);
//	TCD_payout_card(TCD_2, true);
//	TCD_reset(TCD_1, true);
//	TCD_reset(TCD_2, true);
//	TCD_callback(TCD_1, true);
//	TCD_callback(TCD_2, true);
//	HAL_Delay(1000);
//	TCD_payout_card(TCD_2, false);
//	TCD_payout_card(TCD_1, false);
//	HAL_Delay(1000);

//	TCD_reset(TCD_1, set);
//	TCD_callback(TCD_1, set);
//	TCD_payout_card(TCD_2, SET);
//	HAL_Delay(200);
//	TCD_payout_card(TCD_2, RESET);
//	TCD_reset(TCD_2, set);
//	TCD_callback(TCD_2, set);
}
