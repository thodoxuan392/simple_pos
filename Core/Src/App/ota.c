/*
 * ota.c
 *
 *  Created on: Jun 26, 2023
 *      Author: xuanthodo
 */


#include "main.h"
#include "App/ota.h"
#include "Hal/flash.h"
#include "Lib/utils/utils_logger.h"

uint8_t OTA_get_firmware_choosen(){
	return FLASH_read_int(FIRMWARE_CHOOSEN_ADDRESS);
}

uint16_t OTA_get_ota_requested(){
	return FLASH_read_int(OTA_REQUESTED_ADDRESS);
}

void OTA_set_ota_requested(bool enable){
	return FLASH_write_int(OTA_REQUESTED_ADDRESS, (uint16_t)enable);
}

void OTA_jump_to_bootloader(){
	FLASH_write_int(FIRMWARE_CHOOSEN_ADDRESS, BOOTLOADER_CHOOSEN);
	NVIC_SystemReset();
}

void OTA_jump_to_application_1(){
	FLASH_write_int(FIRMWARE_CHOOSEN_ADDRESS, APPLICATION_1_CHOOSEN);
	NVIC_SystemReset();
}

void OTA_jump_to_application_2(){
	FLASH_write_int(FIRMWARE_CHOOSEN_ADDRESS, APPLICATION_2_CHOOSEN);
	NVIC_SystemReset();
}

void OTA_test(){
	uint8_t firmware_choosen = OTA_get_firmware_choosen();
	utils_log_debug("OTA_get_firmware_choosen: %d\r\n", firmware_choosen);
	utils_log_debug("OTA_jump_to_bootloader\r\n");
	OTA_jump_to_bootloader();
}
