/*
 * bill_acceptor.c
 *
 *  Created on: May 14, 2023
 *      Author: xuanthodo
 */

#include <Device/billacceptor.h>
#include "main.h"
#include "string.h"
#include "Hal/timer.h"
#include "Hal/uart.h"

#define BILLACCEPTOR_UART	UART_2
#define BILLACCEPTOR_RES_TIMEOUT		300  	// 200ms
#define BILLACCEPTOR_VALIDATOR_MODE		0x30
#define BILLACCEPTOR_ADDRESS_BIT	0x100
#define BILLACCEPTOR_DATA_BIT		0x000
#define BILLACCEPTOR_ACK_BYTE		0x00
#define BILLACCEPTOR_RET_BYTE		0xAA
#define BILLACCEPTOR_NACK_BYTE 		0xFF

typedef enum {
	BILLACCEPTOR_RESET = 0x30,
	BILLACCEPTOR_SETUP = 0x31,
	BILLACCEPTOR_SECURITY = 0x32,
	BILLACCEPTOR_POLL = 0x33,
	BILLACCEPTOR_BILLTYPE = 0x34,
	BILLACCEPTOR_ESCROW = 0x35,
	BILLACCEPTOR_STACKER = 0x36,
	BILLACCEPTOR_EXPANSION_CMD = 0x37
}BILLACCEPTOR_Cmd_Code_t;

typedef enum {
	BILLACCEPTOR_FEATURE_ENABLE = 0x01,
	BILLACCEPTOR_IDENTIFICATION_WITH_OPT_BIT = 0x02,
	BILLACCEPTOR_RECYCLER_SETUP = 0x03,
	BILLACCEPTOR_RECYCLER_ENABLE = 0x04,
	BILLACCEPTOR_BILL_DISPENSE_STATUS = 0x05,
	BILLACCEPTOR_DISPENSE_BILL = 0x06,
	BILLACCEPTOR_DISPENSE_VALUE = 0x07,
	BILLACCEPTOR_PAYOUT_STATUS = 0x08,
	BILLACCEPTOR_PAYOUT_VALUE = 0x09,
	BILLACCEPTOR_PAYOUT_CANCEL = 0x0A,
}BILLACCEPTOR_SubCmd_Code_t;

static uint32_t billacceptor_timecnt = 0;
static bool billacceptor_timeout_occur = false;
static uint16_t tx_buf[64];


static void BILLACCEPTOR_on_1ms_interrupt();
static bool BILLACCEPTOR_send_cmd(uint16_t *data , size_t data_len);
static bool BILLACCEPTOR_clear_data();
static uint8_t BILLACCEPTOR_calculate_chk(uint16_t * data, size_t data_len);
static bool BILLACCEPTOR_receive_response(uint16_t *data, size_t data_len);
static bool BILLACCEPTOR_receive_response_timeout(uint16_t *data, size_t expected_len, size_t * real_len, uint16_t timeout);
static bool BILLACCEPTOR_is_res_ack(uint16_t code);
static void BILLACCEPTOR_send_ack();

bool BILLACCEPTOR_init(){
	TIMER_attach_intr_1ms(BILLACCEPTOR_on_1ms_interrupt);
}

bool BILLACCEPTOR_reset(){
	BILLACCEPTOR_clear_data();
	uint16_t cmd[1] = {BILLACCEPTOR_RESET};
	BILLACCEPTOR_send_cmd(cmd, 1);
	uint16_t code;
	if(!BILLACCEPTOR_receive_response(&code, 1)){
		return false;
	}
	if(!BILLACCEPTOR_is_res_ack(code)){
		return false;
	}
	return true;
}

bool BILLACCEPTOR_setup(BILLACCEPTOR_Setup_t *setup){
	BILLACCEPTOR_clear_data();
	// Send command
	uint16_t cmd[1] = { BILLACCEPTOR_SETUP };
	BILLACCEPTOR_send_cmd(cmd, 1);
	// Wait for get response
	uint16_t res[28];
	size_t res_size = sizeof(res)/sizeof(uint16_t);
	if(!BILLACCEPTOR_receive_response(res, res_size)){
		return false;
	}
	// Validate checksum
	if(BILLACCEPTOR_calculate_chk(res, res_size-1) != (uint8_t)res[res_size-1]){
		return false;
	}
	BILLACCEPTOR_send_ack();
	setup->feature_level = res[0];
	setup->currency_code[0] = res[1];
	setup->currency_code[1] = res[2];
	setup->scaling_factor[0] = res[3];
	setup->scaling_factor[1] = res[4];
	setup->decimal_place = res[5];
	setup->stacker_capacity[0] = res[6];
	setup->stacker_capacity[1] = res[7];
	setup->security_level[0] = res[8];
	setup->security_level[1] = res[9];
	setup->escrow = res[10];
	memcpy(setup->type_credit, &res[11], 16);
	return true;
}

bool BILLACCEPTOR_security(BILLACCEPTOR_Security_t * security){
	BILLACCEPTOR_clear_data();
	// Send command
	uint16_t cmd[3];
	cmd[0] = BILLACCEPTOR_SECURITY;
	cmd[1] = security->bill_type >> 8;
	cmd[2] = security->bill_type & 0xFF;
	BILLACCEPTOR_send_cmd(cmd, 3);
	// Wait for get response
	uint16_t code;
	if(!BILLACCEPTOR_receive_response(&code, 1)){
		return false;
	}
	if(!BILLACCEPTOR_is_res_ack(code)){
		return false;
	}
	return true;
}

bool BILLACCEPTOR_poll(BILLACCEPTOR_Poll_t * poll){
	BILLACCEPTOR_clear_data();
	// Send command
	uint16_t cmd[1] = {BILLACCEPTOR_POLL};
	BILLACCEPTOR_send_cmd(cmd, 1);
	// Wait for get response
	uint16_t res[17];
	size_t expected_res_size = sizeof(res)/sizeof(uint16_t);
	size_t real_res_size = 0;
	if(!BILLACCEPTOR_receive_response_timeout(res, expected_res_size, &real_res_size, 50)){
		return false;
	}
	if(real_res_size == 0){
		return false;
	}
	// Validate checksum
	if(BILLACCEPTOR_calculate_chk(res, real_res_size-1) != (uint8_t)res[real_res_size-1]){
		return false;
	}
	BILLACCEPTOR_send_ack();
	poll->type = 0xFF;
	if((res[0] >> 7) & 0x01){
		// BillAccptec Type
		poll->BillAccepted.bill_routing = (res[0] >> 4) & 0x07;
		poll->BillAccepted.bill_type = res[0] & 0x0F;
		poll->type = IS_BILLACCEPTED;
	}else{
		// Status Type
		poll->Status.status = res[0];
		poll->type = IS_STATUS;
	}
	return true;
}

bool BILLACCEPTOR_billtype(BILLACCEPTOR_BillType_t * billtype){
	BILLACCEPTOR_clear_data();
	// Send command
	uint16_t cmd[5];
	cmd[0] = BILLACCEPTOR_BILLTYPE;
	cmd[1] = billtype->bill_enable >> 8;
	cmd[2] = billtype->bill_enable & 0xFF;
	cmd[3] = billtype->bill_escrow_enable >> 8;
	cmd[4] = billtype->bill_escrow_enable & 0xFF;
	BILLACCEPTOR_send_cmd(cmd, 5);
	// Wait for get response
	uint16_t code;
	if(!BILLACCEPTOR_receive_response(&code, 1)){
		return false;
	}
	if(!BILLACCEPTOR_is_res_ack(code)){
		return false;
	}
	return true;
}

bool BILLACCEPTOR_escrow(BILLACCEPTOR_Escrow_t * escrow){
	BILLACCEPTOR_clear_data();
	// Send command
	uint16_t cmd[2];
	cmd[0] = BILLACCEPTOR_ESCROW;
	cmd[1] = escrow->escrow_status;
	BILLACCEPTOR_send_cmd(cmd, 2);
	// Wait for get response
	uint16_t code;
	if(!BILLACCEPTOR_receive_response(&code, 1)){
		return false;
	}
	if(!BILLACCEPTOR_is_res_ack(code)){
		return false;
	}
	return true;
}

bool BILLACCEPTOR_stacker(BILLACCEPTOR_Stacker_t * stacker){
	BILLACCEPTOR_clear_data();
	// Send command
	uint16_t cmd[1] = {BILLACCEPTOR_STACKER};
	BILLACCEPTOR_send_cmd(cmd, 1);
	// Wait for get response
	uint16_t res[3];
	size_t res_size = sizeof(res)/sizeof(uint16_t);
	if(!BILLACCEPTOR_receive_response(res, res_size)){
		return false;
	}
	// Validate checksum
	if(BILLACCEPTOR_calculate_chk(res, res_size-1) != (uint8_t)res[res_size-1]){
		return false;
	}
	BILLACCEPTOR_send_ack();
	if((res[0] >> 7)){
		// Stacker is full
		stacker->is_full = 1;
		stacker->number_of_bills = (uint16_t)(res[0] & 0x7F) << 8 | (res[1] & 0xFF);
	}else{
		// Status Type
		stacker->is_full = 0;
	}
	return true;
}

bool BILLACCEPTOR_expcmd_identification(BILLACCEPTOR_Identification_t *identification){
	BILLACCEPTOR_clear_data();
	// Send command
	uint16_t cmd[2];
	cmd[0] = BILLACCEPTOR_EXPANSION_CMD;
	cmd[1] = BILLACCEPTOR_IDENTIFICATION_WITH_OPT_BIT;
	BILLACCEPTOR_send_cmd(cmd, 2);
	// Wait for get response
	uint16_t res[34];
	size_t res_size = sizeof(res)/sizeof(uint16_t);
	if(!BILLACCEPTOR_receive_response(res, res_size)){
		return false;
	}
	// Validate checksum
	if(BILLACCEPTOR_calculate_chk(res, res_size-1) != (uint8_t)res[res_size-1]){
		return false;
	}
	BILLACCEPTOR_send_ack();
	uint8_t res_index = 0;
	for (int var = 0; var < sizeof(identification->manufacter_code); ++var) {
		identification->manufacter_code[var] = res[res_index++];
	}
	for (int var = 0; var < sizeof(identification->serial_number); ++var) {
		identification->serial_number[var] = res[res_index++];
	}
	for (int var = 0; var < sizeof(identification->model); ++var) {
		identification->model[var] = res[res_index++];
	}
	for (int var = 0; var < sizeof(identification->sw_version); ++var) {
		identification->sw_version[var] = res[res_index++];
	}
	for (int var = 0; var < sizeof(identification->option); ++var) {
		identification->option[var] = res[res_index++];
	}
	return true;
}

bool BILLACCEPTOR_expcmd_feature_enable(BILLACCEPTOR_FeatureEnable_t *feature_enable){
	BILLACCEPTOR_clear_data();
	// Send command
	uint16_t cmd[6];
	cmd[0] = BILLACCEPTOR_EXPANSION_CMD;
	cmd[1] = BILLACCEPTOR_FEATURE_ENABLE;
	cmd[2] = feature_enable->option[0];
	cmd[3] = feature_enable->option[1];
	cmd[4] = feature_enable->option[2];
	cmd[5] = feature_enable->option[3];

	BILLACCEPTOR_send_cmd(cmd, 6);
	// Wait for get response
	uint16_t code;
	if(!BILLACCEPTOR_receive_response(&code, 1)){
		return false;
	}
	if(!BILLACCEPTOR_is_res_ack(code)){
		return false;
	}
	return true;
}

bool BILLACCEPTOR_expcmd_recycler_setup(BILLACCEPTOR_RecyclerSetup_t *recycler_setup){
	BILLACCEPTOR_clear_data();
	// Send command
	uint16_t cmd[2];
	cmd[0] = BILLACCEPTOR_EXPANSION_CMD;
	cmd[1] = BILLACCEPTOR_RECYCLER_SETUP;
	BILLACCEPTOR_send_cmd(cmd, 2);
	// Wait for get response
	uint16_t res[3];
	size_t res_size = sizeof(res)/sizeof(uint16_t);
	if(!BILLACCEPTOR_receive_response(res, res_size)){
		return false;
	}
	// Validate checksum
	if(BILLACCEPTOR_calculate_chk(res, res_size-1) != (uint8_t)res[res_size-1]){
		return false;
	}
	BILLACCEPTOR_send_ack();
	uint8_t res_index = 0;
	for (int var = 0; var < sizeof(recycler_setup->bill_type); ++var) {
		recycler_setup->bill_type[var] = res[res_index++];
	}
	return true;
}

bool BILLACCEPTOR_expcmd_recycler_enable(BILLACCEPTOR_RecyclerEnable_t *recycler_enable){
	BILLACCEPTOR_clear_data();
	// Send command
	uint8_t cmd_len = 0;
	uint16_t cmd[6];
	cmd[cmd_len++] = BILLACCEPTOR_EXPANSION_CMD;
	cmd[cmd_len++] = BILLACCEPTOR_FEATURE_ENABLE;
	for (int var = 0; var < sizeof(recycler_enable->man_dispense_ena); ++var) {
		cmd[cmd_len++] = recycler_enable->man_dispense_ena[var];
	}
	for (int var = 0; var < sizeof(recycler_enable->bill_recycler_ena); ++var) {
		cmd[cmd_len++] = recycler_enable->bill_recycler_ena[var];
	}
	BILLACCEPTOR_send_cmd(cmd, cmd_len);
	// Wait for get response
	uint16_t code;
	if(!BILLACCEPTOR_receive_response(&code, 1)){
		return false;
	}
	if(!BILLACCEPTOR_is_res_ack(code)){
		return false;
	}
	return true;
}

bool BILLACCEPTOR_expcmd_bill_dispense_status(BILLACCEPTOR_BillDispenseStatus_t *bill_dispense_status){
	BILLACCEPTOR_clear_data();
	// Send command
	uint16_t cmd[2];
	cmd[0] = BILLACCEPTOR_EXPANSION_CMD;
	cmd[1] = BILLACCEPTOR_BILL_DISPENSE_STATUS;
	BILLACCEPTOR_send_cmd(cmd, 2);
	// Wait for get response
	uint16_t res[35];
	size_t res_size = sizeof(res)/sizeof(uint16_t);
	if(!BILLACCEPTOR_receive_response(res, res_size)){
		return false;
	}
	// Validate checksum
	if(BILLACCEPTOR_calculate_chk(res, res_size-1) != (uint8_t)res[res_size-1]){
		return false;
	}
	BILLACCEPTOR_send_ack();
	uint8_t res_index = 0;
	for (int var = 0; var < sizeof(bill_dispense_status->full_status); ++var) {
		bill_dispense_status->full_status[var] = res[res_index++];
	}
	for (int var = 0; var < sizeof(bill_dispense_status->bill_cnt); ++var) {
		bill_dispense_status->bill_cnt[var] = res[res_index++];
	}
	return true;
}

bool BILLACCEPTOR_expcmd_dispense_bill(BILLACCEPTOR_DispenseBill_t *dispense_bill){
	BILLACCEPTOR_clear_data();
	// Send command
	uint8_t cmd_len = 0;
	uint16_t cmd[5];
	cmd[cmd_len++] = BILLACCEPTOR_EXPANSION_CMD;
	cmd[cmd_len++] = BILLACCEPTOR_DISPENSE_BILL;
	for (int var = 0; var < sizeof(dispense_bill->bill_type); ++var) {
		cmd[cmd_len++] = dispense_bill->bill_type;
	}
	for (int var = 0; var < sizeof(dispense_bill->nb_bill); ++var) {
		cmd[cmd_len++] = dispense_bill->nb_bill[var];
	}
	BILLACCEPTOR_send_cmd(cmd, cmd_len);
	// Wait for get response
	uint16_t code;
	if(!BILLACCEPTOR_receive_response(&code, 1)){
		return false;
	}
	if(!BILLACCEPTOR_is_res_ack(code)){
		return false;
	}
	return true;
}

bool BILLACCEPTOR_expcmd_dispense_value(BILLACCEPTOR_DispenseValue_t * dispense_value){
	BILLACCEPTOR_clear_data();
	// Send command
	uint8_t cmd_len = 0;
	uint16_t cmd[4];
	cmd[cmd_len++] = BILLACCEPTOR_EXPANSION_CMD;
	cmd[cmd_len++] = BILLACCEPTOR_DISPENSE_VALUE;
	for (int var = 0; var < sizeof(dispense_value->value_bill); ++var) {
		cmd[cmd_len++] = dispense_value->value_bill[var];
	}
	BILLACCEPTOR_send_cmd(cmd, cmd_len);
	// Wait for get response
	uint16_t code;
	if(!BILLACCEPTOR_receive_response(&code, 1)){
		return false;
	}
	if(!BILLACCEPTOR_is_res_ack(code)){
		return false;
	}
	return true;
}

bool BILLACCEPTOR_expcmd_payout_status(BILLACCEPTOR_PayoutStatus_t * payout_status){
	BILLACCEPTOR_clear_data();
	// Send command
	uint16_t cmd[2];
	cmd[0] = BILLACCEPTOR_EXPANSION_CMD;
	cmd[1] = BILLACCEPTOR_PAYOUT_STATUS;
	BILLACCEPTOR_send_cmd(cmd, 2);
	// Wait for get response
	uint16_t res[33];
	size_t res_size = sizeof(res)/sizeof(uint16_t);
	if(!BILLACCEPTOR_receive_response(res, res_size)){
		return false;
	}
	// Validate checksum
	if(BILLACCEPTOR_calculate_chk(res, res_size-1) != (uint8_t)res[res_size-1]){
		return false;
	}
	BILLACCEPTOR_send_ack();
	uint8_t res_index = 0;
	for (int var = 0; var < sizeof(payout_status->nb_of_each_bill); ++var) {
		payout_status->nb_of_each_bill[var] = res[res_index++];
	}
	return true;
}

bool BILLACCEPTOR_expcmd_payout_value_poll(BILLACCEPTOR_PayoutValue_t *payout_value){
	BILLACCEPTOR_clear_data();
	// Send command
	uint16_t cmd[2];
	cmd[0] = BILLACCEPTOR_EXPANSION_CMD;
	cmd[1] = BILLACCEPTOR_PAYOUT_STATUS;
	BILLACCEPTOR_send_cmd(cmd, 2);
	// Wait for get response
	uint16_t res[3];
	size_t res_size = sizeof(res)/sizeof(uint16_t);
	if(!BILLACCEPTOR_receive_response(res, res_size)){
		return false;
	}
	// Validate checksum
	if(BILLACCEPTOR_calculate_chk(res, res_size-1) != (uint8_t)res[res_size-1]){
		return false;
	}
	BILLACCEPTOR_send_ack();
	uint8_t res_index = 0;
	for (int var = 0; var < sizeof(payout_value->payout_act); ++var) {
		payout_value->payout_act[var] = res[res_index++];
	}
	return true;
}

bool BILLACCEPTOR_expcmd_payout_cancel(BILLACCEPTOR_PayoutCancel_t *payout_cancel){
	BILLACCEPTOR_clear_data();
	// Send command
	uint8_t cmd_len = 0;
	uint16_t cmd[2];
	cmd[cmd_len++] = BILLACCEPTOR_EXPANSION_CMD;
	cmd[cmd_len++] = BILLACCEPTOR_PAYOUT_CANCEL;
	BILLACCEPTOR_send_cmd(cmd, cmd_len);
	// Wait for get response
	uint16_t code;
	if(!BILLACCEPTOR_receive_response(&code, 1)){
		return false;
	}
	if(!BILLACCEPTOR_is_res_ack(code)){
		return false;
	}
	return true;
}



bool BILLACCEPTOR_test(){
//	BILLACCEPTOR_send_data("Hello", 5);
	BILLACCEPTOR_reset();
	BILLACCEPTOR_BillType_t bill_type = {
				.bill_enable = 0xFFFF,
				.bill_escrow_enable = 0xFFFF
	};
	BILLACCEPTOR_billtype(&bill_type);
	BILLACCEPTOR_Escrow_t escrow = {
			.escrow_status = 0x01
	};
	BILLACCEPTOR_escrow(&escrow);
//	BILLACCEPTOR_Setup_t setup;
//	BILLACCEPTOR_setup(&setup);
//	BILLACCEPTOR_Poll_t poll;
//	BILLACCEPTOR_poll(&poll);
//	BILLACCEPTOR_Security_t security;
//	BILLACCEPTOR_security(&security);
//	BILLACCEPTOR_Stacker_t stacker;
//	BILLACCEPTOR_stacker(&stacker);
}

bool BILLACCEPTOR_test_2(){
	BILLACCEPTOR_Poll_t poll;
	BILLACCEPTOR_poll(&poll);
}

static void BILLACCEPTOR_on_1ms_interrupt(){
	if(billacceptor_timecnt > 0){
		billacceptor_timecnt--;
		if(billacceptor_timecnt == 0){
			billacceptor_timeout_occur = true;
		}
	}
}


static bool BILLACCEPTOR_receive_response_timeout(uint16_t *data, size_t expected_len, size_t * real_len, uint16_t timeout){
	size_t res_len = 0;
	billacceptor_timecnt = timeout;
	billacceptor_timeout_occur = false;
	while(1){
		if(billacceptor_timeout_occur){
			break;
		}
		if(UART_receive_available(BILLACCEPTOR_UART)){
			data[res_len++] = UART_receive_data(BILLACCEPTOR_UART);
			if(res_len == expected_len){
				break;
			}
		}
	}
	*real_len = res_len;
	UART_clear_buffer(BILLACCEPTOR_UART);
	return true;
}

static bool BILLACCEPTOR_receive_response(uint16_t *data, size_t data_len){
	bool success = false;
	size_t res_len = 0;
	billacceptor_timecnt = BILLACCEPTOR_RES_TIMEOUT;
	billacceptor_timeout_occur = false;
	while(1){
		if(billacceptor_timeout_occur){
			break;
		}
		if(UART_receive_available(BILLACCEPTOR_UART)){
			data[res_len++] = UART_receive_data(BILLACCEPTOR_UART);
			if(res_len == data_len){
				success = true;
				break;
			}
		}
	}
	UART_clear_buffer(BILLACCEPTOR_UART);
	return success;
}

static bool BILLACCEPTOR_is_res_ack(uint16_t code){
	if(BILLACCEPTOR_ACK_BYTE == (uint8_t)code){
		return true;
	}
	return false;
}

static bool BILLACCEPTOR_send_cmd(uint16_t *data , size_t data_len){
	size_t tx_len = 0;
	// Command
	tx_buf[tx_len++] = data[0] | BILLACCEPTOR_ADDRESS_BIT;
	// Data
	for (int var = 1; var < data_len; ++var) {
		tx_buf[tx_len++] = data[var] | BILLACCEPTOR_DATA_BIT;
	}
	// Chk
	tx_buf[tx_len] = BILLACCEPTOR_calculate_chk(tx_buf, tx_len);
	tx_len++;
	UART_send(BILLACCEPTOR_UART, tx_buf, tx_len);
}

static bool BILLACCEPTOR_clear_data(){
	UART_clear_buffer(BILLACCEPTOR_UART);
}

static uint8_t BILLACCEPTOR_calculate_chk(uint16_t * data, size_t data_len){
	uint16_t chk = 0;
	for (int var = 0; var < data_len; ++var) {
		chk = chk + data[var];
	}
	return (uint8_t)chk;
}
static void BILLACCEPTOR_send_ack(){
	uint16_t ack = 0x0000;
	UART_send(BILLACCEPTOR_UART, &ack, 1);
}
