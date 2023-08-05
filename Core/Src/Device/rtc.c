/*
 * rtc.c
 *
 *  Created on: Jun 10, 2023
 *      Author: xuanthodo
 */

#ifndef SRC_HAL_RTC_C_
#define SRC_HAL_RTC_C_

#include "main.h"
#include "Device/rtc.h"
#include "Hal/i2c.h"

#define DS1307_ADDRESS	0x68


static uint8_t RTC_dec_to_bcd(uint8_t num);
static uint8_t RTC_bcd_to_dec(uint8_t num);

void RTC_init(){

}

RTC_t RTC_get_time(){
	RTC_t rtc_time;

	uint8_t write_data[] = { 0x00 };
	// seconds, minutes, hours, date, date of week, month, year
	uint8_t read_data[7] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	I2C_write_and_read(DS1307_ADDRESS << 1, write_data, sizeof(write_data), read_data, sizeof(read_data));
	rtc_time.second = RTC_bcd_to_dec(read_data[0] & 0x7F);
	rtc_time.minute = RTC_bcd_to_dec(read_data[1]);
	rtc_time.hour = RTC_bcd_to_dec(read_data[2] & 0x3F);
	rtc_time.date = RTC_bcd_to_dec(read_data[4]);
	rtc_time.month = RTC_bcd_to_dec(read_data[5]);
	rtc_time.year = RTC_bcd_to_dec(read_data[6]) + 2000;
	return rtc_time;
}

void RTC_set_time(RTC_t *rtc){
	uint8_t write_data[8];
	// Set time
	write_data[0] = 0x00;
	write_data[1] = 0x80;
	write_data[2] = RTC_dec_to_bcd(rtc->minute);
	write_data[3] = RTC_dec_to_bcd(rtc->hour);
	write_data[4] = RTC_dec_to_bcd(6);
	write_data[5] = RTC_dec_to_bcd(rtc->date);
	write_data[6] = RTC_dec_to_bcd(rtc->month);
	write_data[7] = RTC_dec_to_bcd(rtc->year - 2000);
	I2C_write(DS1307_ADDRESS  << 1, write_data, 8);
	// Reset timer
	write_data[0] = 0x00;
	write_data[1] = RTC_dec_to_bcd(rtc->second);
	I2C_write(DS1307_ADDRESS  << 1, write_data, 2);
}
void RTC_test(){
	RTC_t rtc = {
		.second = 20,
		.minute = 20,
		.hour = 10,
		.date = 10,
		.month = 1,
		.year = 2001
	};
	RTC_set_time(&rtc);

	RTC_t new_rtc;
	RTC_get_time(&new_rtc);
}


static uint8_t RTC_dec_to_bcd(uint8_t num){
	return ((num/10 * 16) + (num % 10));
}

static uint8_t RTC_bcd_to_dec(uint8_t num){
	return ((num/16 * 10) + (num % 16));
}

#endif /* SRC_HAL_RTC_C_ */
