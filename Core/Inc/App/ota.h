/*
 * ota.h
 *
 *  Created on: Jun 26, 2023
 *      Author: xuanthodo
 */

#ifndef INC_APP_OTA_H_
#define INC_APP_OTA_H_

#include "stdio.h"
#include "stdbool.h"

#define BOOTLOADER_ADDRESS			0x08000000			//	0x08000000 - 0x08006000: 24kB
#define APPLICATION_1_ADDRESS		0x08006000			// 	0x08006000 - 0x0801F000: 100 KB
#define APPLICATION_2_ADDRESS		0x0801F000			//  0x0801F000 - 0x0803FF00: 100 KB
#define FIRMWARE_CHOOSEN_ADDRESS	0x0803F000			//	0x0803F000 - 0x0803F004: 4B
#define OTA_REQUESTED_ADDRESS		0x0803F800			// 	0x0803F800 - 0x0803F804: 4B

enum {
	BOOTLOADER_CHOOSEN,
	APPLICATION_1_CHOOSEN,
	APPLICATION_2_CHOOSEN
};

uint8_t OTA_get_firmware_choosen();
uint16_t OTA_get_ota_requested();
void OTA_set_ota_requested(bool enable);
void OTA_jump_to_bootloader();
void OTA_jump_to_application_1();
void OTA_jump_to_application_2();
void OTA_test();

#endif /* INC_APP_OTA_H_ */
