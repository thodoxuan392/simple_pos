/*
 * card_scanner.h
 *
 *  Created on: May 10, 2023
 *      Author: xuanthodo
 */

#ifndef INC_DEVICE_TCD_H_
#define INC_DEVICE_TCD_H_

#include "stdio.h"
#include "stdbool.h"

#define CARD_ID_LEN        15

typedef enum {
	TCD_1,
	TCD_2
}TCD_id_t;

bool TCD_init();
bool TCD_loop();
bool TCD_payout_card(TCD_id_t id , bool enable);
bool TCD_callback(TCD_id_t id, bool enable);
bool TCD_reset(TCD_id_t id, bool enable);
bool TCD_is_out_ok(TCD_id_t id);
bool TCD_is_error(TCD_id_t id);
bool TCD_is_lower(TCD_id_t id);
bool TCD_is_empty(TCD_id_t id);
// For test IO
bool TCD_test();

#endif /* INC_DEVICE_TCD_H_ */
