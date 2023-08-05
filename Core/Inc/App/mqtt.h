#ifndef MQTT_H
#define MQTT_H

#include "stdint.h"
#include "stdbool.h"


#define NETWORK_RESET_WAIT_TIME		10000	// 10000ms
#define COMMAND_INTERVAL	2000		// 1000ms
#define CLIENTID_MAX_LEN	64
#define TOPIC_MAX_LEN       64
#define PAYLOAD_MAX_LEN     512


enum {
	SUBTOPIC_CONFIG,
	SUBTOPIC_COMMAND,
};

typedef struct {
	uint8_t topic_id;
    char topic[TOPIC_MAX_LEN];
    char payload[PAYLOAD_MAX_LEN];
    uint8_t qos;
    uint8_t retain;
}MQTT_message_t;

void MQTT_init();
void MQTT_run();


bool MQTT_is_ready();
bool MQTT_sent_message(MQTT_message_t * message);
bool MQTT_receive_message(MQTT_message_t * message);


#endif //MQTT_H
