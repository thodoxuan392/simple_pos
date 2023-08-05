/*
 * tcd_manager.c
 *
 *  Created on: Jun 2, 2023
 *      Author: xuanthodo
 */


#include "main.h"
#include "DeviceManager/tcdmanager.h"
#include "Device/tcd.h"
#include "Lib/scheduler/scheduler.h"
#include "Lib/utils/utils_logger.h"

#define INIT_DURATION					3000	// 3s
#define PAYOUT_DURATION					500	//500ms
#define CALLBACK_DURATION				300	//300ms
#define CARD_TO_PLACE_CARD_TIMEOUT		10000	// 3 seconds
#define TAKING_CARD_TIMEOUT				60000	// 3 minutes
#define ERROR_CHECK_INTERVAL			3000	// 3s
#define UPDATING_STATUS_TIME_WHEN_LOWER			5000	//7000s
#define UPDATING_STATUS_TIME_WHEN_NORMAL		100	//100ms



enum {
	TCD_IDLE,
	TCD_RESETING,
	TCD_WAIT_FOR_RESETING,
	TCD_PAYOUTING,
	TCD_WAIT_FOR_PAYOUTING,
	TCD_WAIT_FOR_CARD_IN_PLACE,
	TCD_WAIT_FOR_TAKING_CARD,
	TCD_CALLBACKING,
	TCD_WAIT_FOR_CALLBACKING,
	TCD_WAIT_FOR_UPDATING_STATUS,
	TCD_ERROR
};

typedef struct{
	TCD_id_t id;
	// Status
	uint8_t prev_state;
	uint8_t state;
	// Status
	TCD_status_t status;
	// Action
	bool payout_enable;
	bool callback_enable;
	bool reset_enable;
	// Timeout
	bool timeout_task_id;
	bool timeout;

}TCD_HandleType_t;

static const char * tcd_state_name[] = {
		[TCD_IDLE] = "TCD_IDLE\r\n",
		[TCD_RESETING] = "TCD_RESETING\r\n",
		[TCD_WAIT_FOR_RESETING] = "TCD_WAIT_FOR_RESETING\r\n",
		[TCD_PAYOUTING] = "TCD_PAYOUTING\r\n",
		[TCD_WAIT_FOR_PAYOUTING] = "TCD_WAIT_FOR_PAYOUTING\r\n",
		[TCD_WAIT_FOR_CARD_IN_PLACE] = "TCD_WAIT_FOR_CARD_IN_PLACE\r\n",
		[TCD_WAIT_FOR_TAKING_CARD] = "TCD_WAIT_FOR_TAKING_CARD\r\n",
		[TCD_CALLBACKING] = "TCD_CALLBACKING\r\n",
		[TCD_WAIT_FOR_CALLBACKING] = "TCD_WAIT_FOR_CALLBACKING\r\n",
		[TCD_WAIT_FOR_UPDATING_STATUS] = "TCD_WAIT_FOR_UPDATING_STATUS\r\n",
		[TCD_ERROR] = "TCD_ERROR\r\n",
};

static TCD_id_t tcd_using = TCD_1;

// TCD Device
static TCD_HandleType_t htcd_1 = {
	.id = TCD_1,
	.prev_state = TCD_IDLE,
	.state = TCD_IDLE,
	.status = {
		.is_error = true,
		.is_lower = true,
		.is_empty = true
	},
	.payout_enable = false,
	.callback_enable = false,
	.reset_enable = false,
	.timeout_task_id = 0,
	.timeout = false
};

static TCD_HandleType_t htcd_2 = {
	.id = TCD_2,
	.prev_state = TCD_IDLE,
	.state = TCD_IDLE,
	.status = {
		.is_error = true,
		.is_lower = true,
		.is_empty = true
	},
	.payout_enable = false,
	.callback_enable = false,
	.reset_enable = false,
	.timeout_task_id = 0,
	.timeout = false
};

// Callback
static TCDMNG_take_card_cb take_card_cb;
static TCDMNG_callback_card_cb callback_card_cb;

// Private function
static void TCD_run(TCD_HandleType_t *htcd);
static void TCD_idle(TCD_HandleType_t *htcd);
static void TCD_reseting(TCD_HandleType_t *htcd);
static void TCD_wait_for_reseting(TCD_HandleType_t *htcd);
static void TCD_payouting(TCD_HandleType_t *htcd);
static void TCD_wait_for_payouting(TCD_HandleType_t *htcd);
static void TCD_wait_for_card_in_place(TCD_HandleType_t *htcd);
static void TCD_wait_for_taking_card(TCD_HandleType_t *htcd);
static void TCD_callbacking(TCD_HandleType_t *htcd);
static void TCD_wait_for_callbacking(TCD_HandleType_t *htcd);
static void TCD_wait_for_updating_status(TCD_HandleType_t *htcd);
static void TCD_error(TCD_HandleType_t *htcd);
static void TCD_update_status(TCD_HandleType_t *htcd);
static bool TCD_is_available(TCD_HandleType_t *htcd);
static void TCD_timeout_tcd_1();
static void TCD_timeout_tcd_2();
static void TCD_printf(TCD_HandleType_t *htcd);

void TCDMNG_init(){
}

void TCDMNG_set_take_card_cb(TCDMNG_take_card_cb callback){
	take_card_cb = callback;
}
void TCDMNG_set_callback_card_cb(TCDMNG_callback_card_cb callback){
	callback_card_cb = callback;
}

void TCDMNG_run(){
	TCD_run(&htcd_1);
	TCD_run(&htcd_2);
}

TCDMNG_Status_t TCDMNG_get_status(){
	TCDMNG_Status_t status;
	status.TCD_1 = htcd_1.status;
	status.TCD_2 = htcd_2.status;
	return status;
}

bool TCDMNG_is_in_idle(){
	return (htcd_1.state == TCD_IDLE || htcd_2.state == TCD_IDLE);
}

bool TCDMNG_is_in_processing(){
	return (htcd_1.state != TCD_IDLE && htcd_2.state != TCD_ERROR);
}

bool TCDMNG_is_in_error(){
	return (htcd_1.state == TCD_ERROR || htcd_1.state == TCD_ERROR);
}

void TCDMNG_reset(){
	// Reset both of tcd
	htcd_1.reset_enable = true;
	htcd_2.reset_enable = true;
}

void TCDMNG_payout(){
	if(tcd_using == TCD_1){
		if((TCD_is_available(&htcd_1) && !TCD_is_lower(TCD_2))
			|| (TCD_is_available(&htcd_2) && TCD_is_lower(TCD_2) && TCD_is_lower(TCD_1))){
			htcd_2.payout_enable = true;
			tcd_using = TCD_2;
		}
	}
	else if(tcd_using == TCD_2){
		if((TCD_is_available(&htcd_2) && !TCD_is_lower(TCD_1))
			|| (TCD_is_available(&htcd_1) && TCD_is_lower(TCD_1) && TCD_is_lower(TCD_2))){
			htcd_1.payout_enable = true;
			tcd_using = TCD_1;
		}
	}else {
		utils_log_error("Cannot callback because don't any tcd is used before\r\n");
	}
}

void TCDMNG_callback(){
	if(tcd_using == TCD_1){
		if((TCD_is_available(&htcd_1) && !TCD_is_lower(TCD_2))
			|| (TCD_is_available(&htcd_2) && TCD_is_lower(TCD_2) && TCD_is_lower(TCD_1))){
			htcd_2.callback_enable = true;
			tcd_using = TCD_2;
		}
	}
	else if(tcd_using == TCD_2){
		if((TCD_is_available(&htcd_2) && !TCD_is_lower(TCD_1))
			|| (TCD_is_available(&htcd_1) && TCD_is_lower(TCD_1) && TCD_is_lower(TCD_2))){
			htcd_1.callback_enable = true;
			tcd_using = TCD_1;
		}
	}
	else {
		utils_log_error("Cannot callback because don't any tcd is used before\r\n");
	}
}


bool TCDMNG_is_error(){
	TCD_update_status(&htcd_1);
	TCD_update_status(&htcd_2);
	return (htcd_1.status.is_error) &&
			(htcd_2.status.is_error);
}

bool TCDMNG_is_lower(){
	TCD_update_status(&htcd_1);
	TCD_update_status(&htcd_2);
	return (htcd_1.status.is_lower && htcd_2.status.is_lower);
}

bool TCDMNG_is_empty(){
	TCD_update_status(&htcd_1);
	TCD_update_status(&htcd_2);
	return (htcd_1.status.is_empty && htcd_2.status.is_empty);
}

bool TCDMNG_is_available_for_use(){
	return (TCD_is_available(&htcd_1) || TCD_is_available(&htcd_2));
}


static bool TCD_is_available(TCD_HandleType_t *htcd){
	TCD_update_status(htcd);
	return (!htcd->status.is_empty &&
			!htcd->status.is_error);
}

static void TCD_run(TCD_HandleType_t *htcd){
	TCD_update_status(htcd);
	switch (htcd->state){
		case TCD_IDLE:
			TCD_idle(htcd);
			break;
		case TCD_RESETING:
			TCD_reseting(htcd);
			break;
		case TCD_WAIT_FOR_RESETING:
			TCD_wait_for_reseting(htcd);
			break;
		case TCD_PAYOUTING:
			TCD_payouting(htcd);
			break;
		case TCD_WAIT_FOR_PAYOUTING:
			TCD_wait_for_payouting(htcd);
			break;
		case TCD_WAIT_FOR_CARD_IN_PLACE:
			TCD_wait_for_card_in_place(htcd);
			break;
		case TCD_WAIT_FOR_TAKING_CARD:
			TCD_wait_for_taking_card(htcd);
			break;
		case TCD_CALLBACKING:
			TCD_callbacking(htcd);
			break;
		case TCD_WAIT_FOR_CALLBACKING:
			TCD_wait_for_callbacking(htcd);
			break;
		case TCD_WAIT_FOR_UPDATING_STATUS:
			TCD_wait_for_updating_status(htcd);
			break;
		case TCD_ERROR:
			TCD_error(htcd);
			break;
		default:
			break;
	}
	TCD_printf(htcd);
	htcd->prev_state = htcd->state;
}


static void TCD_idle(TCD_HandleType_t *htcd){
	if (htcd->status.is_error){
		SCH_Delete_Task(htcd->timeout_task_id);
		htcd->timeout = false;
		void * timeout_func = htcd->id == TCD_1? TCD_timeout_tcd_1 : TCD_timeout_tcd_2;
		htcd->timeout_task_id = SCH_Add_Task(timeout_func, ERROR_CHECK_INTERVAL, 0);
		htcd->state = TCD_ERROR;
	}
	else if(htcd->reset_enable){
		htcd->reset_enable = false;
		htcd->state = TCD_RESETING;
	}
	else if(htcd->payout_enable){
		htcd->payout_enable = false;
		htcd->state = TCD_PAYOUTING;
	}
	else if (htcd->callback_enable){
		htcd->callback_enable = false;
		htcd->state = TCD_CALLBACKING;
	}
}

static void TCD_reseting(TCD_HandleType_t *htcd){
	TCD_reset(htcd->id, true);
	// How long to enable payout signal
	SCH_Delete_Task(htcd->timeout_task_id);
	htcd->timeout = false;
	void * timeout_func = htcd->id == TCD_1? TCD_timeout_tcd_1 : TCD_timeout_tcd_2;
	htcd->timeout_task_id = SCH_Add_Task(timeout_func, PAYOUT_DURATION, 0);
	htcd->state = TCD_WAIT_FOR_RESETING;
}

static void TCD_wait_for_reseting(TCD_HandleType_t *htcd){
	if(htcd->timeout){
		TCD_reset(htcd->id, false);
	}
}

static void TCD_payouting(TCD_HandleType_t *htcd){
	TCD_payout_card(htcd->id, true);
	SCH_Delete_Task(htcd->timeout_task_id);
	htcd->timeout = false;
	void * timeout_func = htcd->id == TCD_1? TCD_timeout_tcd_1 : TCD_timeout_tcd_2;
	htcd->timeout_task_id = SCH_Add_Task(timeout_func, PAYOUT_DURATION, 0);
	htcd->state = TCD_WAIT_FOR_PAYOUTING;
}

static void TCD_wait_for_payouting(TCD_HandleType_t *htcd){
	if(htcd->timeout){
		TCD_payout_card(htcd->id, false);
		SCH_Delete_Task(htcd->timeout_task_id);
		htcd->timeout = false;
		void * timeout_func = htcd->id == TCD_1? TCD_timeout_tcd_1 : TCD_timeout_tcd_2;
		htcd->timeout_task_id = SCH_Add_Task(timeout_func, CARD_TO_PLACE_CARD_TIMEOUT, 0);
		htcd->state = TCD_WAIT_FOR_CARD_IN_PLACE;
	}
}

static void TCD_wait_for_card_in_place(TCD_HandleType_t *htcd){
	if(htcd->timeout){
		utils_log_error("Timeout to payout card, check card in tcd\r\n");
		SCH_Delete_Task(htcd->timeout_task_id);
		htcd->timeout = false;
		void * timeout_func = htcd->id == TCD_1? TCD_timeout_tcd_1 : TCD_timeout_tcd_2;
		htcd->timeout_task_id = SCH_Add_Task(timeout_func, ERROR_CHECK_INTERVAL, 0);
		htcd->state = TCD_ERROR;
	}
	if(TCD_is_out_ok(htcd->id)){
		SCH_Delete_Task(htcd->timeout_task_id);
		htcd->timeout = false;
		void * timeout_func = htcd->id == TCD_1? TCD_timeout_tcd_1 : TCD_timeout_tcd_2;
		htcd->timeout_task_id = SCH_Add_Task(timeout_func, TAKING_CARD_TIMEOUT, 0);
		htcd->state = TCD_WAIT_FOR_TAKING_CARD;
	}
}

static void TCD_wait_for_taking_card(TCD_HandleType_t *htcd){
	uint32_t updating_status_time;
	if(htcd->timeout){
		utils_log_error("Timeout to taking card\r\n");
		SCH_Delete_Task(htcd->timeout_task_id);
		htcd->timeout = false;
		void * timeout_func = htcd->id == TCD_1? TCD_timeout_tcd_1 : TCD_timeout_tcd_2;
		htcd->timeout_task_id = SCH_Add_Task(timeout_func, ERROR_CHECK_INTERVAL, 0);
		htcd->state = TCD_ERROR;
	}
	// Card is in place but cannot be take
	if(!TCD_is_out_ok(htcd->id)){
		utils_log_error("Take card completed\r\n");
		// Should callback
		if(take_card_cb) take_card_cb(htcd->id);
		SCH_Delete_Task(htcd->timeout_task_id);
		htcd->timeout = false;
		if(TCD_is_lower(htcd->id)){
			updating_status_time = UPDATING_STATUS_TIME_WHEN_LOWER;
		}else{
			updating_status_time = UPDATING_STATUS_TIME_WHEN_NORMAL;
		}
		void * timeout_func = htcd->id == TCD_1? TCD_timeout_tcd_1 : TCD_timeout_tcd_2;
		htcd->timeout_task_id = SCH_Add_Task(timeout_func, updating_status_time , 0);
		htcd->state = TCD_WAIT_FOR_UPDATING_STATUS;
	}
}

static void TCD_callbacking(TCD_HandleType_t *htcd){
	TCD_callback(htcd->id, true);
	// Should callback
	if(callback_card_cb) callback_card_cb(htcd->id);
	// How long to enable payout signal
	SCH_Delete_Task(htcd->timeout_task_id);
	htcd->timeout = false;
	void * timeout_func = htcd->id == TCD_1? TCD_timeout_tcd_1 : TCD_timeout_tcd_2;
	htcd->timeout_task_id = SCH_Add_Task(timeout_func, PAYOUT_DURATION, 0);
	htcd->state = TCD_WAIT_FOR_CALLBACKING;
}

static void TCD_wait_for_callbacking(TCD_HandleType_t *htcd){
	if(htcd->timeout){
		TCD_callback(htcd->id, false);
		htcd->state = TCD_IDLE;
	}
}

static void TCD_wait_for_updating_status(TCD_HandleType_t *htcd){
	if(htcd->timeout){
		htcd->state = TCD_IDLE;
	}
}


static void TCD_error(TCD_HandleType_t *htcd){
	if(htcd->timeout){
		if(!TCDMNG_is_error()){
			htcd->state = TCD_IDLE;
		}else{
			SCH_Delete_Task(htcd->timeout_task_id);
			htcd->timeout = false;
			void * timeout_func = htcd->id == TCD_1? TCD_timeout_tcd_1 : TCD_timeout_tcd_2;
			htcd->timeout_task_id = SCH_Add_Task(timeout_func, ERROR_CHECK_INTERVAL, 0);
		}
	}
}

static void TCD_update_status(TCD_HandleType_t *htcd){
	// Get status of 2 TCD
	htcd->status.is_empty = TCD_is_empty(htcd->id);
	htcd->status.is_error = TCD_is_error(htcd->id);
	htcd->status.is_lower = TCD_is_lower(htcd->id);
}

static void TCD_timeout_tcd_1(){
	htcd_1.timeout = true;
}
static void TCD_timeout_tcd_2(){
	htcd_2.timeout = true;
}


static void TCD_printf(TCD_HandleType_t *htcd){
	if(htcd->prev_state != htcd->state){
		utils_log_info("TCD_%d: %s",htcd->id, tcd_state_name[htcd->state]);
	}
}
