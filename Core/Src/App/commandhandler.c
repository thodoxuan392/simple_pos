/*
 * commandhandler.c
 *
 *  Created on: Jun 3, 2023
 *      Author: xuanthodo
 */

#include "main.h"
#include "config.h"
#include "App/commandhandler.h"
#include "App/mqtt.h"
#include "Lib/jsmn/jsmn.h"
#include "Lib/utils/utils_logger.h"

enum {
	COMMANDHANDLE_IDLE,
	COMMANDHANDLE_EXECUTE
};

enum {
	COMMAND_RESET,
	COMMAND_DELETE_TOTAL_CARD,
	COMMAND_DELETE_TOTAL_AMOUNT
};

static uint8_t state = COMMANDHANDLE_IDLE;
static MQTT_message_t message;

static void COMMANDHANDLER_handle_config(uint8_t * payload, size_t payload_len);
static void COMMANDHANDLER_handle_command(uint8_t * payload, size_t payload_len);
static bool COMMANDHANDLER_parse_config(uint8_t *payload, size_t payload_len, CONFIG_t *config);
static bool COMMANDHANDLER_parse_command(uint8_t *payload, size_t payload_len, uint8_t *command);


bool COMMANDHANDLER_init(){

}

bool COMMANDHANDLER_run(){
	if(MQTT_receive_message(&message)){
		switch (message.topic_id) {
			case SUBTOPIC_CONFIG:
				COMMANDHANDLER_handle_config(message.payload, strlen(message.payload));
				break;
			case SUBTOPIC_COMMAND:
				COMMANDHANDLER_handle_command(message.payload, strlen(message.payload));
				break;
			default:
				break;
		}
	}
}

static void COMMANDHANDLER_handle_config(uint8_t * payload, size_t payload_len){
	CONFIG_t * config = CONFIG_get();
	if(COMMANDHANDLER_parse_config(payload, payload_len, config)){
		CONFIG_set(config);
	}
}

static void COMMANDHANDLER_handle_command(uint8_t * payload, size_t payload_len){
	// Parse command
	utils_log_info("Payload: %s\r\n", (char*)payload);
	uint8_t command;
	CONFIG_t *config;
	if(COMMANDHANDLER_parse_command(payload, payload_len, &command)){
		switch (command) {
			case COMMAND_RESET:
				utils_log_info("COMMAND_RESET\r\n");
				NVIC_SystemReset();
				break;
			case COMMAND_DELETE_TOTAL_CARD:
				utils_log_info("COMMAND_DELETE_TOTAL_CARD\r\n");
				config = CONFIG_get();
				config->total_card = 0;
				config->total_card_by_day = 0;
				config->total_card_by_month = 0;
				CONFIG_set(config);
				break;
			case COMMAND_DELETE_TOTAL_AMOUNT:
				utils_log_info("COMMAND_DELETE_TOTAL_AMOUNT\r\n");
				config = CONFIG_get();
				config->total_amount = 0;
				CONFIG_set(config);
				break;
			default:
				break;
		}
	}
}

static bool COMMANDHANDLER_parse_config(uint8_t *payload, size_t payload_len, CONFIG_t *config){
	jsmn_parser p;
	jsmntok_t t[PAYLOAD_MAX_LEN]; /* We expect no more than 128 tokens */

	jsmn_init(&p);
	uint32_t r = jsmn_parse(&p, payload, payload_len, t,
				 sizeof(t) / sizeof(t[0]));
	if (r < 0) {
		utils_log_debug("Failed to parse JSON: %d\r\n", r);
		return false;
	}

	/* Assume the top-level element is an object */
	if (r < 1 || t[0].type != JSMN_OBJECT) {
		utils_log_debug("Object expected\r\n");
		return false;
	}

	/* Loop over all keys of the root object */
	for (uint32_t i = 1; i < r; i++) {
		if (jsmn_streq(payload, &t[i], "pwd") == 0) {
			memset(config->password, 0 , sizeof(config->password));
			strncpy(config->password, payload + t[i + 1].start, t[i + 1].end - t[i + 1].start);
			utils_log_debug("- Password: %s\r\n", config->password);
			i++;
		} else if (jsmn_streq(payload, &t[i], "cp") == 0) {
			/* We may additionally check if the value is either "true" or "false" */
			config->card_price = utils_string_to_int(payload + t[i + 1].start, t[i + 1].end - t[i + 1].start);
			utils_log_debug("- Card Price: %d\r\n", config->card_price);
			i++;
		}
	}
	return true;
}


static bool COMMANDHANDLER_parse_command(uint8_t *payload, size_t payload_len, uint8_t *command){
	jsmn_parser p;
	jsmntok_t t[PAYLOAD_MAX_LEN]; /* We expect no more than 128 tokens */

	jsmn_init(&p);
	uint32_t r = jsmn_parse(&p, payload, payload_len, t,
				 sizeof(t) / sizeof(t[0]));
	if (r < 0) {
		utils_log_debug("Failed to parse JSON: %d\r\n", r);
		return false;
	}

	/* Assume the top-level element is an object */
	if (r < 1 || t[0].type != JSMN_OBJECT) {
		utils_log_debug("Object expected\r\n");
		return false;
	}

	/* Loop over all keys of the root object */
	for (uint32_t i = 1; i < r; i++) {
		if (jsmn_streq(payload, &t[i], "cmd") == 0) {
			* command = (uint8_t)utils_string_to_int(payload + t[i + 1].start, t[i + 1].end - t[i + 1].start);
			utils_log_debug("- Command: %d\r\n", * command);
			i++;
		}
	}
	return true;
}


