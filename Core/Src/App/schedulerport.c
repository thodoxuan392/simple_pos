/*
 * schedulerport.c
 *
 *  Created on: Jun 3, 2023
 *      Author: xuanthodo
 */


#include "App/schedulerport.h"
#include "Lib/scheduler/scheduler.h"
#include "Hal/timer.h"

void SCHEDULERPORT_init(){
	TIMER_attach_intr_1ms(SCH_Update);
}
