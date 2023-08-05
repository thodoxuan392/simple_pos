/*
 * config.c
 *
 *  Created on: Jun 2, 2023
 *      Author: xuanthodo
 */


#include "config.h"
#include "Device/eeprom.h"
#include "Lib/utils/utils_logger.h"

#define EEPROM_CONFIG_ADDRESS	0x0000

static CONFIG_t config = {
	.version = VERSION,
	.device_id = DEVICE_ID_DEFAULT,
	.password = SETTING_MODE_PASSWORD,
	.card_price = 10000,
	.amount = 0,
	.total_amount = 0,
	.total_card = 0,
	.total_card_by_day = 0,
	.total_card_by_month = 0
};

static bool CONFIG_set_default(CONFIG_t * config,  CONFIG_t *config_temp);
static bool CONFIG_field_is_empty(uint8_t *data, size_t data_len);

bool CONFIG_init(){
	CONFIG_t temp;
	EEPROM_read(EEPROM_CONFIG_ADDRESS, (uint8_t*)&temp, sizeof(CONFIG_t));
	CONFIG_set_default(&config, &temp);
	utils_log_info("CONFIG init done\r\n");
	CONFIG_printf();
}

CONFIG_t * CONFIG_get(){
	return &config;
}

void CONFIG_set(CONFIG_t * _config){
	memcpy(&config, _config, sizeof(CONFIG_t));
	EEPROM_write(EEPROM_CONFIG_ADDRESS, (uint8_t*)&config, sizeof(CONFIG_t));
	CONFIG_printf();
}


void CONFIG_printf(){
	utils_log_info("Version: %s\r\n", config.version);
	utils_log_info("DeviceId: %s\r\n", config.device_id);
	utils_log_info("Password: %s\r\n", config.password);
	utils_log_info("CardPrice: %d\r\n", config.card_price);
	utils_log_info("Amount: %d\r\n", config.amount);
	utils_log_info("total_amount: %d\r\n", config.total_amount);
	utils_log_info("total_card: %d\r\n", config.total_card);
	utils_log_info("total_card_by_day: %d\r\n", config.total_card_by_day);
	utils_log_info("total_card_by_month: %d\r\n", config.total_card_by_month);
}

void CONFIG_clear(){
	memset(&config, 0xFF , sizeof(CONFIG_t));
	EEPROM_write(EEPROM_CONFIG_ADDRESS, &config, sizeof(CONFIG_t));
}

void CONFIG_test(){
	CONFIG_t * newConfig = CONFIG_get();
	newConfig->total_card = 5;
	newConfig->card_price = 25000;
	CONFIG_set(newConfig);
}

static bool CONFIG_set_default(CONFIG_t * _config,  CONFIG_t *config_temp){
	// Set version
	if(!CONFIG_field_is_empty((uint8_t*)config_temp->version, sizeof(config_temp->version))){
		memcpy(_config->version, config_temp->version, sizeof(_config->version));
	}
	// Set device_id
	if(!CONFIG_field_is_empty((uint8_t*)config_temp->device_id, sizeof(config_temp->device_id))){
		memcpy(_config->device_id, config_temp->device_id, sizeof(_config->device_id));
	}
	// Set password
	if(!CONFIG_field_is_empty((uint8_t*)config_temp->password, sizeof(config_temp->password))){
		memcpy(_config->password, config_temp->password, sizeof(_config->password));
	}
	// Set amount
	if(!CONFIG_field_is_empty((uint8_t*)(&config_temp->amount), sizeof(config_temp->amount))){
		memcpy(&_config->amount, &config_temp->amount, sizeof(_config->amount));
	}
	// Set card prices
	if(!CONFIG_field_is_empty((uint8_t*)(&config_temp->card_price), sizeof(config_temp->card_price))){
		memcpy(&_config->card_price, &config_temp->card_price, sizeof(_config->card_price));
	}
	// Set total_amount
	if(!CONFIG_field_is_empty((uint8_t*)(&config_temp->total_amount), sizeof(config_temp->total_amount))){
		memcpy(&_config->total_amount, &config_temp->total_amount, sizeof(_config->total_amount));
	}
	if(!CONFIG_field_is_empty((uint8_t*)(&config_temp->total_card), sizeof(config_temp->total_card))){
		memcpy(&_config->total_card, &config_temp->total_card, sizeof(_config->total_card));
	}
	if(!CONFIG_field_is_empty((uint8_t*)(&config_temp->total_card_by_day), sizeof(config_temp->total_card_by_day))){
		memcpy(&_config->total_card_by_day, &config_temp->total_card_by_day, sizeof(_config->total_card_by_day));
	}
	// Set card prices
	if(!CONFIG_field_is_empty((uint8_t*)(&config_temp->total_card_by_month), sizeof(config_temp->total_card_by_month))){
		memcpy(&_config->total_card_by_month, &config_temp->total_card_by_month, sizeof(_config->total_card_by_month));
	}
}

static bool CONFIG_field_is_empty(uint8_t *data, size_t data_len){
	for (int var = 0; var < data_len; ++var) {
		if(data[var] != 0xFF){
			return false;
		}
	}
	return true;
}
