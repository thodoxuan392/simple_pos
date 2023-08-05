/*
 * keypadhandler.h
 *
 *  Created on: Jun 9, 2023
 *      Author: xuanthodo
 */

#ifndef INC_APP_KEYPADHANDLER_H_
#define INC_APP_KEYPADHANDLER_H_

#include "stdio.h"
#include "stdbool.h"

#define KEYPADHANDLER_BUFFER_LEN	64


bool KEYPADHANDLER_init();
bool KEYPADHANDLER_run();
bool KEYPADHANDLER_is_not_in_setting();



#endif /* INC_APP_KEYPADHANDLER_H_ */
