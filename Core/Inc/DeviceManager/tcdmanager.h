/*
 * tcd_manager.h
 *
 *  Created on: Jun 2, 2023
 *      Author: xuanthodo
 */

#ifndef INC_DEVICEMANAGER_TCDMANAGER_H_
#define INC_DEVICEMANAGER_TCDMANAGER_H_

#include "stdio.h"
#include "stdbool.h"
#include "Device/tcd.h"

typedef struct {
	bool is_error;
	bool is_lower;
	bool is_empty;
}TCD_status_t;

typedef struct {
	TCD_status_t TCD_1;
	TCD_status_t TCD_2;
}TCDMNG_Status_t;

typedef void (*TCDMNG_take_card_cb)(TCD_id_t id);
typedef void (*TCDMNG_callback_card_cb)(TCD_id_t id);

void TCDMNG_init();
void TCDMNG_set_take_card_cb(TCDMNG_take_card_cb callback);
void TCDMNG_set_callback_card_cb(TCDMNG_callback_card_cb callback);
void TCDMNG_run();
TCDMNG_Status_t TCDMNG_get_status();
uint8_t TCDMNG_get_state();
bool TCDMNG_is_in_idle();
bool TCDMNG_is_in_processing();
bool TCDMNG_is_in_error();
void TCDMNG_reset();
void TCDMNG_payout();
void TCDMNG_callback();
bool TCDMNG_is_error();
bool TCDMNG_is_lower();
bool TCDMNG_is_empty();
bool TCDMNG_is_available_for_use();



#endif /* INC_DEVICEMANAGER_TCDMANAGER_H_ */
