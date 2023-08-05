/*
 * config.h
 *
 *  Created on: Jun 2, 2023
 *      Author: xuanthodo
 */

#ifndef INC_APP_CONFIG_H_
#define INC_APP_CONFIG_H_

#include "stdio.h"
#include "stdbool.h"

#define VERSION_MAX_LEN		8
#define VERSION	 			"1.0.0"

#define MANUAL_MODE_PASSWORD		"987654321"

#define SETTING_MODE_PASSWORD_MAX_LEN	10
#ifndef SETTING_MODE_PASSWORD
	#define SETTING_MODE_PASSWORD	"123"
#endif

#define DEVICE_ID_MAX_LEN				10
#ifndef DEVICE_ID_DEFAULT
	#define DEVICE_ID_DEFAULT		"123456"
#endif

typedef struct {
	char version[VERSION_MAX_LEN];
	char device_id[DEVICE_ID_MAX_LEN];
	char password[SETTING_MODE_PASSWORD_MAX_LEN];
	uint32_t card_price;
	uint32_t amount;
	uint32_t total_amount;
	uint32_t total_card;
	uint32_t total_card_by_day;
	uint32_t total_card_by_month;
}CONFIG_t;

bool CONFIG_init();
CONFIG_t * CONFIG_get();
void CONFIG_set(CONFIG_t *);
void CONFIG_clear();
void CONFIG_test();

#endif /* INC_APP_CONFIG_H_ */
