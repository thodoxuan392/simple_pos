/*
 * eeprom.c
 *
 *  Created on: May 14, 2023
 *      Author: xuanthodo
 */


#include "main.h"
#include "string.h"
#include "Device/eeprom.h"
#include "Hal/i2c.h"

#define EEPROM_ADDRESS	0xA0
#define EEPROM_ADDRESS_SIZE	4
#define PAGE_SIZE	32	// 32 bytes

enum {
	EEPROM_READ_OP,
	EEPROM_WRITE_OP,
	EEPROM_ERASE_OP
};

static uint8_t i2c_buffer_wr[PAGE_SIZE];
static uint8_t i2c_buffer_rd[PAGE_SIZE];


bool EEPROM_init(){
	return true;
}

bool EEPROM_read(uint16_t _address , uint8_t * data, size_t data_len){
	I2C_mem_read(EEPROM_ADDRESS, _address, EEPROM_ADDRESS_SIZE, data, data_len);
	return true;
}

bool EEPROM_write(uint16_t _address, uint8_t * data, size_t data_len){
	uint16_t address;
	size_t remain_size = data_len;
	size_t write_size;
	for(;remain_size > 0;){
		if(remain_size > PAGE_SIZE){
			write_size = PAGE_SIZE;
		}else{
			write_size = remain_size;
		}
		address = _address + data_len - remain_size;
		memcpy(i2c_buffer_wr , &data[data_len - remain_size], write_size);
		I2C_mem_write(EEPROM_ADDRESS, address, EEPROM_ADDRESS_SIZE,  i2c_buffer_wr, write_size);
		HAL_Delay(5);
		remain_size -= write_size;
	}
	return true;
}

bool EEPROM_test(){
	// Write
	uint8_t test_str[100];
	for (int var = 0; var < 100; ++var) {
		test_str[var] = var;
	}
	EEPROM_write(0x00, test_str, sizeof(test_str));
	// Read
	static uint8_t read_buf[100] = {0};
	EEPROM_read(0x00, read_buf, 100);
	return true;
}

