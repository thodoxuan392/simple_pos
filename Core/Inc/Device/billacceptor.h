/*
 * money_scanner.h
 *
 *  Created on: May 10, 2023
 *      Author: xuanthodo
 */

#ifndef INC_DEVICE_BILLACCEPTOR_H_
#define INC_DEVICE_BILLACCEPTOR_H_

#include "stdio.h"
#include "stdbool.h"

typedef struct {
	uint8_t feature_level;
	uint8_t currency_code[2];
	uint8_t scaling_factor[2];
	uint8_t decimal_place;
	uint8_t stacker_capacity[2];
	uint8_t security_level[2];
	uint8_t escrow;
	uint8_t type_credit[16];
}BILLACCEPTOR_Setup_t;

typedef struct{
	uint16_t bill_type;
}BILLACCEPTOR_Security_t;


typedef enum {
	IS_BILLACCEPTED,
	IS_STATUS
}BILLACCEPTOR_PollType_t;

enum {
	BILL_STACKED = 0x0,
	BILL_ESCROW_POSITION = 0x1,
	BILL_RETURNED = 0x2,
	BILL_TO_RECYCLER = 0x3,
	DISABLE_BILL_REJECTED = 0x04,
	BILL_TO_RECYCLER_MAN = 0x05,
	MANUAL_DISPENSE = 0x06,
	TRANSFERRED_FROM_RECYCLER_TO_CASHBOX = 0x07,
};

typedef struct {
	struct{
		uint8_t bill_routing;
		uint8_t bill_type;
	}BillAccepted;
	struct{
		uint8_t status;
	}Status;
	BILLACCEPTOR_PollType_t type;
}BILLACCEPTOR_Poll_t;

typedef struct {
	uint16_t bill_enable;
	uint16_t bill_escrow_enable;
}BILLACCEPTOR_BillType_t;

typedef struct {
	uint8_t escrow_status;
	uint8_t poll_status;
}BILLACCEPTOR_Escrow_t;

typedef struct {
	uint8_t is_full;
	uint16_t number_of_bills;
}BILLACCEPTOR_Stacker_t;

// Expansion command
typedef struct {
	uint8_t manufacter_code[3];
	uint8_t serial_number[12];
	uint8_t model[12];
	uint8_t sw_version[2];
	uint8_t option[4];
}BILLACCEPTOR_Identification_t;

typedef struct {
	uint8_t option[4];
}BILLACCEPTOR_FeatureEnable_t;

typedef struct {
	uint8_t bill_type[2];
}BILLACCEPTOR_RecyclerSetup_t;

typedef struct {
	uint8_t man_dispense_ena[2];
	uint8_t bill_recycler_ena[16];
}BILLACCEPTOR_RecyclerEnable_t;

typedef struct {
	uint8_t full_status[2];
	uint8_t bill_cnt[32];
}BILLACCEPTOR_BillDispenseStatus_t;

typedef struct {
	uint8_t bill_type;
	uint8_t nb_bill[2];
}BILLACCEPTOR_DispenseBill_t;

typedef struct {
	uint8_t value_bill[2];
}BILLACCEPTOR_DispenseValue_t;

typedef struct {
	uint8_t nb_of_each_bill[32];
}BILLACCEPTOR_PayoutStatus_t;

typedef struct {
	uint8_t payout_act[2];
}BILLACCEPTOR_PayoutValue_t;

typedef struct {
	// Nothing to return
}BILLACCEPTOR_PayoutCancel_t;

bool BILLACCEPTOR_init();
bool BILLACCEPTOR_reset();
bool BILLACCEPTOR_setup(BILLACCEPTOR_Setup_t *setup);
bool BILLACCEPTOR_security(BILLACCEPTOR_Security_t * security);
bool BILLACCEPTOR_poll(BILLACCEPTOR_Poll_t * poll);
bool BILLACCEPTOR_billtype(BILLACCEPTOR_BillType_t * billtype);
bool BILLACCEPTOR_escrow(BILLACCEPTOR_Escrow_t * escrow);
bool BILLACCEPTOR_stacker(BILLACCEPTOR_Stacker_t * stacker);
// Expansion command
bool BILLACCEPTOR_expcmd_identification(BILLACCEPTOR_Identification_t *identification);
bool BILLACCEPTOR_expcmd_feature_enable(BILLACCEPTOR_FeatureEnable_t *feature_enable);
bool BILLACCEPTOR_expcmd_recycler_setup(BILLACCEPTOR_RecyclerSetup_t *recycler_setup);
bool BILLACCEPTOR_expcmd_recycler_enable(BILLACCEPTOR_RecyclerEnable_t *recycler_enable);
bool BILLACCEPTOR_expcmd_bill_dispense_status(BILLACCEPTOR_BillDispenseStatus_t *bill_dispense_status);
bool BILLACCEPTOR_expcmd_dispense_bill(BILLACCEPTOR_DispenseBill_t *dispense_bill);
bool BILLACCEPTOR_expcmd_dispense_value(BILLACCEPTOR_DispenseValue_t * dispense_value);
bool BILLACCEPTOR_expcmd_payout_status(BILLACCEPTOR_PayoutStatus_t * payout_status);
bool BILLACCEPTOR_expcmd_payout_value_poll(BILLACCEPTOR_PayoutValue_t *payout_value);
bool BILLACCEPTOR_expcmd_payout_cancel(BILLACCEPTOR_PayoutCancel_t *payout_cancel);

// For test IO
bool BILLACCEPTOR_test();

#endif /* INC_DEVICE_BILLACCEPTOR_H_ */
