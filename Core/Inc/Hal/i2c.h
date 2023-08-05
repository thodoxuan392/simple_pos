/*
 * i2c.h
 *
 *  Created on: May 13, 2023
 *      Author: xuanthodo
 */

#ifndef INC_HAL_I2C_H_
#define INC_HAL_I2C_H_

#include "stdio.h"
#include "stdbool.h"

#define I2C_TIMEOUT	200 	// 200ms

void I2C_init();
bool I2C_write(uint8_t address, uint8_t * data_w, size_t w_len);
bool I2C_write_and_read(uint8_t address, uint8_t * data_w, size_t w_len, uint8_t * data_r, size_t r_len);
bool I2C_read(uint8_t address, uint8_t * data_r, size_t r_len);
bool I2C_mem_write(uint8_t address, uint16_t mem_address, uint16_t mem_size, uint8_t * data_w, size_t w_len);

#endif /* INC_HAL_I2C_H_ */
