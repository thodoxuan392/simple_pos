/*
 * statusreporter.c
 *
 *  Created on: Jun 3, 2023
 *      Author: xuanthodo
 */

#include "config.h"
#include <App/mqtt.h>
#include "App/statusreporter.h"
#include "DeviceManager/billacceptormanager.h"
#include "DeviceManager/tcdmanager.h"
#include "Lib/scheduler/scheduler.h"

#define STATUSREPORT_INTERVAL		30 * 1000 	// 5 minutes

static bool timeout_flag = true;

// Private function
static void STATUSREPORTER_report_status();
static void STATUSREPORTER_build_status_topic(char * buf, char * device_id);
static void STATUSREPORTER_build_status_payload(char * buf,
													CONFIG_t* config,
													TCDMNG_Status_t tcd_status,
													uint8_t billacepptor_status);
static void STATUSREPORTER_build_bill_accepted_topic(char * buf, char * device_id);
static void STATUSREPORTER_build_bill_accepted_payload(char * buf, uint32_t bill_value);
static void STATUSREPORTER_timeout();

bool STATUSREPORTER_init(){

}

bool STATUSREPORTER_run(){
	if(timeout_flag){
		timeout_flag = false;
		// Publish status
		STATUSREPORTER_report_status();
		SCH_Add_Task(STATUSREPORTER_timeout, STATUSREPORT_INTERVAL, 0);
	}
}

void STATUSREPORTER_report_billaccepted(uint32_t bill_value){
	CONFIG_t *config = CONFIG_get();
	MQTT_message_t message = {
		.qos = 1,
		.retain = 0
	};
	// Build Topic
	STATUSREPORTER_build_bill_accepted_topic(message.topic, config->device_id);
	STATUSREPORTER_build_bill_accepted_payload(message.payload, bill_value);
	// Send message
	MQTT_sent_message(&message);
}

static void STATUSREPORTER_report_status(){
	CONFIG_t *config = CONFIG_get();
	MQTT_message_t message = {
		.qos = 1,
		.retain = 1
	};

	// Build Topic
	STATUSREPORTER_build_status_topic(message.topic, config->device_id);
	// Build Payload
	TCDMNG_Status_t tcd_status = TCDMNG_get_status();
	uint8_t billacceptor_status = BILLACCEPTORMNG_get_status();

	STATUSREPORTER_build_status_payload(message.payload, config, tcd_status, billacceptor_status);
	// Send message
	MQTT_sent_message(&message);
}
static void STATUSREPORTER_build_status_topic(char * buf, char * device_id){
	snprintf(buf,
			TOPIC_MAX_LEN,
			"%s/rp/status",
			device_id);
}

static void STATUSREPORTER_build_status_payload(char * buf,
													CONFIG_t* config,
													TCDMNG_Status_t tcd_status,
													uint8_t billacepptor_status){
	snprintf(buf,
				PAYLOAD_MAX_LEN,
				"{"
					"\"v\":\"%s\","
					"\"pwd\":\"%s\","
					"\"cp\":%d,"
					"\"amt\":%d,"
					"\"to_amt\":%d,"
					"\"to_ca\":%d,"
					"\"to_ca_d\":%d,"
					"\"to_ca_m\":%d,"
					"\"tcd_1\":[%d,%d,%d],"
					"\"tcd_2\":[%d,%d,%d],"
					"\"bill\": %d"
				"}",
					config->version,
					config->password,
					config->card_price,
					config->amount,
					config->total_amount,
					config->total_card,
					config->total_card_by_day,
					config->total_card_by_month,
					tcd_status.TCD_1.is_empty,
					tcd_status.TCD_1.is_error,
					tcd_status.TCD_1.is_lower,
					tcd_status.TCD_2.is_empty,
					tcd_status.TCD_2.is_error,
					tcd_status.TCD_2.is_lower,
					billacepptor_status);
}

static void STATUSREPORTER_build_bill_accepted_topic(char * buf, char * device_id){
	snprintf(buf,
			TOPIC_MAX_LEN,
			"%s/rp/bill_accepted",
			device_id);
}

static void STATUSREPORTER_build_bill_accepted_payload(char * buf, uint32_t bill_value){
	snprintf(buf,
				PAYLOAD_MAX_LEN,
				"{"
					"\"value\":%d"
				"}",
				bill_value);
}

static void STATUSREPORTER_timeout(){
	timeout_flag = true;
}
