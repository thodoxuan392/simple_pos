/*
 * statemachine.h
 *
 *  Created on: May 10, 2023
 *      Author: xuanthodo
 */

#ifndef INC_APP_STATEMACHINE_H_
#define INC_APP_STATEMACHINE_H_

#include "stdio.h"
#include "stdbool.h"

#define SM_INIT_DURATION	3000	// 3s
#define SM_BILLACCEPTOR_DURATION	2000 // 2s
#define SM_TAKING_CARD_TIMEOUT	30000 // 30s
#define SM_UPDATE_DURATION			30000 // 60s
#define SM_CHECK_CARD_STATUS_INTERVAL	5000 // 2s
#define SM_KEYPAD_BUF		128

bool STATEMACHINE_init();
bool STATEMACHINE_run();

#endif /* INC_APP_STATEMACHINE_H_ */
